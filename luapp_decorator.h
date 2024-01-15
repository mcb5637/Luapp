#pragma once
#include <format>
#include <string_view>
#include <sstream>
#include <set>

#include "constexprTypename.h"
#include "luapp_common.h"
#include "luapp_userdata.h"

namespace lua::decorator {
	/// <summary>
	/// <para>represents a lua state.
	/// contains only a pointer, so pass-by-value is prefered.
	/// you need to close this state manually.</para>
	/// <para>the notation [-x,+y,e] is used to indicate changes in the stack:</para>
	/// <para>x is the amount poped from the stack,</para>
	/// <para>y is the amount pushed to the stack</para>
	/// <para>(? is used for an amount that does not depend on the parameters)</para>
	/// <para>(a|b indicates an amount of a or b),</para>
	/// <para>e is an indication of possible exceptions</para>
	/// <para>(- no exception, m memory only, e other exceptions, v throws on purpose, t only with type checks enabled).</para>
	/// </summary>
	template<class B>
	class State : public B {
		constexpr static std::string_view MethodsName = "Methods";
		constexpr static std::string_view BaseTypeNameName = "BaseTypeName";

		template<class State, class T> requires userdata::IndexCpp<State, T>
		friend int userdata::IndexOperator(State L);
	public:
		/// <summary>
		/// <para>normal function to interface with lua.</para>
		/// <para>recieves its arguments on the lua stack in direct order (first argument at 1) (with nothing else on the stack).</para>
		/// <para>to return, push arguments onto the stack in direct order, and return the number of returns from the func.</para>
		/// </summary>
		/// <param name="L">lua state</param>
		/// <returns>number of return values on the stack</returns>
		using CppFunction = int(*)(State L);

		/// <summary>
		/// adapts a CppFunction to a CFunction (aka lua_CFunction), doing all the type conversion and exception handling.
		/// </summary>
		/// <param name="l">lua state</param>
		/// <returns>number of return values on the stack</returns>
		template<CppFunction F>
		static int CppToCFunction(lua_State* l) {
			if constexpr (CatchExceptions) {
				bool err = false;
				State L{ l }; // trivial, no destructor to call, so its save
				int ret = 0;
				{ // make sure all c++ objects gets their destructor called
					try {
						ret = F(L);
					}
					catch (const std::exception& e) {
						L.PushFString("%s: %s in %s", typeid(e).name(), e.what(), __FUNCSIG__);
						err = true;
					}
					catch (...) {
						auto ExceptionConverter = B::GetExConv();
						if (ExceptionConverter != nullptr) {
							try {
								auto s = ExceptionConverter(std::current_exception(), __FUNCSIG__);
								L.Push(s);
								err = true;
							}
							catch (...) {}
						}
						if (!err) {
							L.PushFString("unknown exception caught in %s", __FUNCSIG__);
							err = true;
						}
					}
				}
				if (err)
					L.Error();
				return ret;
			}
			else {
				State L{ l };
				return F(L);
			}
		}
		/// <summary>
		/// adapts a member function to a CppFunction by storing the object in upvalue 1.
		/// </summary>
		/// <typeparam name="O">object type</typeparam>
		/// <typeparam name="F">member function pointer to push</typeparam>
		/// <param name="L">state</param>
		/// <returns>number of return values on the stack</returns>
		template<class O, int(O::* F)(State L)>
		static int MemberClosure(State L) {
			auto* o = static_cast<O*>(L.ToUserdata(B::Upvalueindex(1)));
			return std::invoke(F, o, L);
		}
		/// <summary>
		/// adapts a member function to a CppFunction by storing the object in upvalue 1.
		/// </summary>
		/// <typeparam name="O">object type</typeparam>
		/// <typeparam name="F">member function pointer to push</typeparam>
		/// <param name="L">state</param>
		/// <returns>number of return values on the stack</returns>
		template<class O, int(O::* F)(State L) const>
		static int MemberClosure(State L) {
			auto* o = static_cast<O*>(L.ToUserdata(B::Upvalueindex(1)));
			return std::invoke(F, o, L);
		}

		/// <summary>
		/// info to register a function to lua.
		/// </summary>
		struct FuncReference {
			std::string_view Name;
			CFunction Func;
			void* Upvalue;

			constexpr FuncReference(std::string_view name, CFunction f, void* upvalue = nullptr) : Name(name), Func(f), Upvalue(upvalue) {
			}

			/// <summary>
			/// A reference to a CppFunction.
			/// </summary>
			/// <typeparam name="F"></typeparam>
			/// <param name="name"></param>
			/// <returns></returns>
			template<CppFunction F>
			constexpr static FuncReference GetRef(std::string_view name) {
				return { name, &CppToCFunction<F> };
			}
			/// <summary>
			/// A Reference to a CFunction.
			/// </summary>
			/// <typeparam name="F"></typeparam>
			/// <param name="name"></param>
			/// <returns></returns>
			template<CFunction F>
			constexpr static FuncReference GetRef(std::string_view name) {
				return { name, F };
			}
			/// <summary>
			/// A reference to a Member function bound as CppFunction.
			/// <para>does NOT take ownership of the object, you have to keep it alive for as long as the pushed function might be called and dispose of it afterwards.</para>
			/// <para>if you need GC on a object, a userdata type with call operator might be more appropriate.</para>
			/// </summary>
			/// <typeparam name="O"></typeparam>
			/// <typeparam name="F"></typeparam>
			/// <param name="obj"></param>
			/// <param name="name"></param>
			/// <returns></returns>
			template<class O, int(O::* F)(State L)>
			constexpr static FuncReference GetRef(O& obj, std::string_view name) {
				return { name, &CppToCFunction<MemberClosure<O, F>>, &obj };
			}
			/// <summary>
			/// A reference to a Member function bound as CppFunction.
			/// <para>does NOT take ownership of the object, you have to keep it alive for as long as the pushed function might be called and dispose of it afterwards.</para>
			/// <para>needs a non const object, because lua does not have const userdata.</para>
			/// <para>if you need GC on a object, a userdata type with call operator might be more appropriate.</para>
			/// </summary>
			/// <typeparam name="O"></typeparam>
			/// <typeparam name="F"></typeparam>
			/// <param name="obj"></param>
			/// <param name="name"></param>
			/// <returns></returns>
			template<class O, int(O::* F)(State L) const>
			constexpr static FuncReference GetRef(O& obj, std::string_view name) {
				return { name, &CppToCFunction<MemberClosure<O, F>>, &obj };
			}

			auto operator<=>(const FuncReference&) const = default;
		};

		/// <summary>
		/// lua reference. just an int, so pass by value preffered.
		/// </summary>
		/// <see cref='lua50::State::Ref'/>
		class Reference {
			friend class State;
			int r;

			constexpr Reference(int r)
			{
				this->r = r;
			}
		public:
			// initialized with noref
			constexpr Reference()
			{
				r = State<B>::NoRef.r;
			}

			auto operator<=>(const Reference&) const = default;
		};

		/// <summary>
		/// lua hook function (when registered, gets called durng lua code execution).
		/// </summary>
		/// <param name="L">lua state</param>
		/// <param name="ar">activation record</param>
		using CppHook = void(*) (State L, B::ActivationRecord ar);

		/// <summary>
		/// adapts a CppHook to a CHook, doing all the type conversion and exception handling.
		/// </summary>
		/// <param name="l">lua state</param>
		/// <param name="ar">activation record</param>
		template<CppHook F>
		static void CppToCHook(lua_State* l, lua_Debug* ar) {
			if constexpr (CatchExceptions) {
				bool err = false;
				State L{ l }; // trivial, no destructor to call, so its save
				{ // make sure all c++ objects gets their destructor called
					try {
						F(L, typename B::ActivationRecord{ ar });
					}
					catch (const std::exception& e) {
						L.PushFString("%s: %s in %s", typeid(e).name(), e.what(), __FUNCSIG__);
						err = true;
					}
					catch (...) {
						auto ExceptionConverter = B::GetExConv();
						if (ExceptionConverter) {
							try {
								auto s = ExceptionConverter(std::current_exception(), __FUNCSIG__);
								L.Push(s);
								err = true;
							}
							catch (...) {}
						}
						if (!err) {
							L.PushFString("unknown exception caught in %s", __FUNCSIG__);
							err = true;
						}
					}
				}
				if (err)
					L.Error();
			}
			else {
				State L{ l };
				F(L, typename B::ActivationRecord{ ar });
			}
		}



		/// <summary>
		/// creates a State from a lua_State* (usually from external APIs).
		/// </summary>
		/// <param name="L">state pointer</param>
		State(lua_State* L) : B(L) {}
		/// <summary>
		/// opens a new lua state.
		/// </summary>
		/// <param name="io">open io and os libs</param>
		/// <param name="debug">open debug lib</param>
		State(bool io = true, bool debug = false) : B(io, debug) {}
		/// <summary>
		/// opens a new lua state (for similarity with luas c api).
		/// </summary>
		/// <param name="io">open io and os libs</param>
		/// <param name="debug">open debug lib</param>
		/// <returns>new state</returns>
		static State Create(bool io = true, bool debug = false) { return State(io, debug); }

		/// <summary>
		/// pushes a CFunction or CClosure (function with upvalues) onto the stack.
		/// to create a CClosure, push the initial values for its upvalues onto the stack, and then call this function with the number of upvalues as nups.
		/// <para>[-nups,+1,m]</para>
		/// </summary>
		/// <param name="F">function</param>
		/// <param name="nups">number of upvalues</param>
		template<CFunction F>
		void Push(int nups = 0)
		{
			Push(F, nups);
		}
		/// <summary>
		/// pushes a CppFunction or CppClosure (function with upvalues) onto the stack.
		/// to create a CClosure, push the initial values for its upvalues onto the stack, and then call this function with the number of upvalues as nups.
		/// <para>[-nups,+1,m]</para>
		/// </summary>
		/// <param name="F">function</param>
		/// <param name="nups">number of upvalues</param>
		template<CppFunction F>
		void Push(int nups = 0)
		{
			B::Push(&CppToCFunction<F>, nups);
		}
		/// <summary>
		/// pushes a member function pointer bound to a object onto the stack. the bound object takes up upvalue 1.
		/// to create a CClosure, push the initial values for its upvalues onto the stack, and then call this function with the number of upvalues as nups.
		/// The object gets inserted as upvalue 1, and all others shifted up by 1.
		/// <para>does NOT take ownership of the object, you have to keep it alive for as long as the pushed function might be called and dispose of it afterwards.</para>
		/// <para>if you need GC on a object, a userdata type with call operator might be more appropriate.</para>
		/// <para>[-nups,+1,m]</para>
		/// </summary>
		/// <typeparam name="O">object type</typeparam>
		/// <typeparam name="F">function</typeparam>
		/// <param name="obj">object</param>
		/// <param name="nups">number of upvalues</param>
		template<class O, int(O::*F)(State L)>
		void Push(O& obj, int nups = 0) {
			B::PushLightUserdata(&obj);
			B::Insert(-nups - 1);
			Push<MemberClosure<O, F>>(nups + 1);
		}
		/// <summary>
		/// pushes a member function pointer bound to a object onto the stack. the bound object takes up upvalue 1.
		/// to create a CClosure, push the initial values for its upvalues onto the stack, and then call this function with the number of upvalues as nups.
		/// The object gets inserted as upvalue 1, and all others shifted up by 1.
		/// <para>does NOT take ownership of the object, you have to keep it alive for as long as the pushed function might be called and dispose of it afterwards.</para>
		/// <para>needs a non const object, because lua does not have const userdata.</para>
		/// <para>if you need GC on a object, a userdata type with call operator might be more appropriate.</para>
		/// <para>[-nups,+1,m]</para>
		/// </summary>
		/// <typeparam name="O">object type</typeparam>
		/// <typeparam name="F">function</typeparam>
		/// <param name="obj">object</param>
		/// <param name="nups">number of upvalues</param>
		template<class O, int(O::* F)(State L) const>
		void Push(O& obj, int nups = 0) {
			B::PushLightUserdata(&obj);
			B::Insert(-nups - 1);
			Push<MemberClosure<O, F>>(nups + 1);
		}
		/// <summary>
		/// pushes a std::string_view.
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="s">string to push</param>
		void Push(std::string_view s)
		{
			B::Push(s.data(), s.size());
		}
		/// <summary>
		/// pushes an integer onto the stack.
		/// <para>[-0,+1,-]</para>
		/// </summary>
		/// <param name="i">int</param>
		void Push(int i) {
			if constexpr (B::Capabilities::NativeIntegers)
				B::Push(static_cast<lua::Integer>(i));
			else
				B::Push(static_cast<lua::Number>(i));
		}
		/// <summary>
		/// pushes the string representation of a MetaEvent onto the stack.
		/// </summary>
		/// <param name="ev">event</param>
		void Push(B::MetaEvent ev) {
			Push(B::GetMetaEventName(ev));
		}
		using B::Push;

		/// <summary>
		/// dumps a lua function at the top of the stack to binary, which can be loaded again via Load.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <returns>binary data of the function</returns>
		std::string Dump() {
			std::stringstream str{};
			B::Dump([](lua_State* L, const void* data, size_t s, void* ud) {
				auto* st = static_cast<std::stringstream*>(ud);
				st->write(static_cast<const char*>(data), s);
				return 0;
				}, &str);
			return str.str();
		}
		using B::Dump;

		/// <summary>
		/// converts to a std::string_view.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="idx">valid index to convert.</param>
		/// <returns>string</returns>
		/// <exception cref="lua::LuaException">if not a string</exception>
		std::string_view ToStringView(int idx)
		{
			size_t l = 0;
			const char* s = B::ToString(idx, &l);
			if (!s)
				throw LuaException("no string");
			return { s, l };
		}
		/// <summary>
		/// converts to a std::string.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="idx">valid index to convert.</param>
		/// <returns>string</returns>
		/// <exception cref="lua::LuaException">if not a string</exception>
		std::string ToStdString(int idx)
		{
			return std::string{ ToStringView(idx) };
		}
		/// <summary>
		/// pops a key from the stack, and pushes the associated value in the table at index onto the stack.
		/// may call metamethods.
		/// <para>[-1,+1,e]</para>
		/// </summary>
		/// <param name="index">valid index for table access</param>
		/// <exception cref="lua::LuaException">on lua error</exception>
		void GetTable(int index) {
			B::PushValue(index);
			B::Insert(-2);
			B::Push(B::GetTable_Unprotected);
			B::Insert(-3);
			TCall(2, 1);
		}
		/// <summary>
		/// assigns the value at the top of the stack to the key just below the top in the table at index. pops both key and value from the stack.
		/// may call metamethods.
		/// <para>[-2,+0,e]</para>
		/// </summary>
		/// <param name="index">valid index for table acccess</param>
		/// <exception cref="lua::LuaException">on lua error</exception>
		void SetTable(int index) {
			B::PushValue(index);
			B::Insert(-3);
			B::Push(B::SetTable_Unprotected);
			B::Insert(-4);
			TCall(3, 0);
		}
		/// <summary>
		/// pushes the with s associated value in the table at index onto the stack.
		/// may not call metamethods.
		/// <para>[-0,+1,t]</para>
		/// </summary>
		/// <param name="index">valid index for table access</param>
		/// <param name="n">key</param>
		void GetTableRaw(int index, std::string_view s) {
			index = B::ToAbsoluteIndex(index);
			Push(s);
			B::GetTableRaw(index);
		}
		using B::GetTableRaw;
		/// <summary>
		/// assigns the value at the top of the stack to the key s in the table at index. pops the value from the stack.
		/// may not call metamethods.
		/// <para>[-1,+0,mt]</para>
		/// </summary>
		/// <param name="index">valid index for table acccess</param>
		/// <param name="n">key</param>
		void SetTableRaw(int index, std::string_view s) {
			index = B::ToAbsoluteIndex(index);
			Push(s);
			B::Insert(-2);
			B::SetTableRaw(index);
		}
		using B::SetTableRaw;
		/// <summary>
		/// assigns the value at the top of the stack to the key just below the top in the global table. pops both key and value from the stack.
		/// may not call metamethods.
		/// <para>[-2,+0,m]</para>
		/// </summary>
		void SetGlobal() {
			if constexpr (B::Capabilities::GlobalsIndex) {
				B::SetTableRaw(B::GLOBALSINDEX);
			}
			else {
				B::PushGlobalTable();
				B::Insert(-3);
				B::SetTableRaw(-3);
				B::Pop(1);
			}
		}
		/// <summary>
		/// assigns the value at the top of the stack to the key k in the global table. pops the value from the stack.
		/// may not call metamethods.
		/// <para>[-1,+0,m]</para>
		/// </summary>
		/// <param name="k">key</param>
		void SetGlobal(std::string_view k) {
			Push(k);
			B::Insert(-2);
			SetGlobal();
		}
		/// <summary>
		/// pops a key from the stack, and pushes the associated value in the global table onto the stack.
		/// may not call metamethods.
		/// <para>[-1,+1,-]</para>
		/// </summary>
		void GetGlobal() {
			if constexpr (B::Capabilities::GlobalsIndex) {
				B::GetTableRaw(B::GLOBALSINDEX);
			}
			else {
				B::PushGlobalTable();
				B::Insert(-2);
				B::GetTableRaw(-2);
				B::Remove(-2);
			}
		}
		/// <summary>
		/// pushes the with k associated value in the global table onto the stack.
		/// may not call metamethods.
		/// <para>[-0,+1,-]</para>
		/// </summary>
		/// <param name="k"></param>
		void GetGlobal(std::string_view k) {
			Push(k);
			GetGlobal();
		}
		/// <summary>
		/// compares 2 lua values. returns true, if the value at i1 satisfies op when compared with the value at i2.
		/// also returns false, if any of the indices are invalid.
		/// may call metamethods.
		/// <para>[-0,+0,e]</para>
		/// </summary>
		/// <param name="i1">acceptable index 1</param>
		/// <param name="i2">acceptable index 1</param>
		/// <param name="op">operator</param>
		/// <returns>satisfies</returns>
		/// <exception cref="lua::LuaException">on lua error</exception>
		bool Compare(int i1, int i2, B::ComparisonOperator op) {
			bool ret = false;
			if (!B::IsValidIndex(i1) || !B::IsValidIndex(i2))
				return false;
			i1 = B::ToAbsoluteIndex(i1);
			i2 = B::ToAbsoluteIndex(i2);
			B::Push(&B::Compare_Unprotected);
			B::PushValue(i1);
			B::PushValue(i2);
			B::PushLightUserdata(&ret);
			Push(static_cast<int>(op));
			TCall(4, 0);
			return ret;
		}
		/// <summary>
		/// checks equality of 2 values. may call metamethods.
		/// also returns false, if any of the indices are invalid.
		/// <para>[-0,+0,e]</para>
		/// </summary>
		/// <param name="i1">acceptable index 1</param>
		/// <param name="i2">acceptable index 2</param>
		/// <returns>values equals</returns>
		/// <exception cref="lua::LuaException">on lua error</exception>
		bool Equal(int i1, int i2) {
			return Compare(i1, i2, B::ComparisonOperator::Equals);
		}
		/// <summary>
		/// checks if i1 is smaller than i2. may call metamethods.
		/// also returns false, if any of the indices are invalid.
		/// <para>[-0,+0,e]</para>
		/// </summary>
		/// <param name="i1">acceptable index 1</param>
		/// <param name="i2">acceptable index 2</param>
		/// <returns>smaller than</returns>
		/// <exception cref="lua::LuaException">on lua error</exception>
		bool LessThan(int i1, int i2) {
			return Compare(i1, i2, B::ComparisonOperator::LessThan);
		}
		/// <summary>
		/// returns the length of an object. this is the same as applying the # operator (may call metamethods).
		/// (in lua 5.1, does not call metamethods for tables.)
		/// pushes the result onto the stack.
		/// <para>[-0,+1,e]</para>
		/// </summary>
		/// <param name="index">index to query</param>
		/// <exception cref="lua::LuaException">on lua error</exception>
		void ObjLength(int index) requires (B::Capabilities::MetatableLengthModulo) {
			index = B::ToAbsoluteIndex(index);
			B::Push(&B::ObjLen_Unprotected);
			B::PushValue(index);
			TCall(1, 1);
		}
		/// <summary>
		/// concatenates the num values at the top of the stack, using the usual semantics. if num is 0, pushes an empty string, if num is 1, does nothing.
		/// <para>[-num,+1,e]</para>
		/// </summary>
		/// <param name="num">number of values to concatenate</param>
		/// <exception cref="lua::LuaException">on lua error</exception>
		void Concat(int num) {
			B::Push(&B::Concat_Unprotected);
			B::Insert(-num - 1);
			Push(num);
			TCall(num + 1, 1);
		}
		/// <summary>
		/// performs an arithmetic operation over the 2 values at the top of the stack (or one in case of unary negation and bitwise not), pops the values and pushes the result.
		/// the top values is the second operand.
		/// may call metamethods.
		/// <para>[-2|1,+1,e]</para>
		/// </summary>
		/// <param name="op">operator</param>
		/// <exception cref="lua::LuaException">on lua error</exception>
		void Arithmetic(B::ArihmeticOperator op) {
			bool hasoneparam;
			if constexpr (B::Capabilities::NativeIntegers) 
				hasoneparam = op == B::ArihmeticOperator::UnaryNegation || op == B::ArihmeticOperator::BitwiseNot;
			else
				hasoneparam = op == B::ArihmeticOperator::UnaryNegation;
			B::Push(&B::Arithmetic_Unprotected);
			B::Insert(hasoneparam ? -2 : -3);
			Push(static_cast<int>(op));
			TCall(hasoneparam ? 2 : 3, 1);
		}
		/// <summary>
		/// traverses the table index by poping the previous key from the stack and pushing the next key and value to the stack.
		/// if there are no more elements in the table, returns false and pushes nothing. otherwise returns true.
		/// <para>do not call tostring onto a key, unless you know that it is acually a string</para>
		/// <para>[-1,+2|0,e]</para>
		/// </summary>
		/// <param name="index">valid index to traverse</param>
		/// <returns>had next</returns>
		bool Next(int index) {
			bool r = false;
			B::PushValue(index);
			B::Insert(-2);
			B::PushLightUserdata(&r);
			B::Insert(-3);
			B::Push(&B::Next_Unproteced);
			B::Insert(-4);
			TCall(3, B::MULTIRET);
			return r;
		}
		class PairsHolder;
		/// <summary>
		/// <para>allows iteration over a lua table.</para>
		/// <para>while iterating, the key is at -2, and the value is at -1.</para>
		/// <para>do not pop value or key.</para>
		/// <para>do not apply ToString directly onto the key, unless you know it is actually a string.</para>
		/// <para>iterator returns the type of the key.</para>
		/// <para>++PairsIter may also throw, if something goes wrong.</para>
		/// <para>when the iteration ends by reaching its end, no key/value pair is left on the stack.</para>
		/// <para>if you break the iteration (or leave it otherwise), you have to clean up the key/value pair from the stack yourself.</para>
		/// <para>[-0,+2|0,e]</para>
		/// </summary>
		/// <param name="index">valid index to iterate over</param>
		/// <returns>PairsHolder</returns>
		/// <exception cref="lua::LuaException">on lua error</exception>
		PairsHolder Pairs(int index) {
			return PairsHolder(*this, index);
		}
		class IPairsHolder;
		/// <summary>
		/// <para>allows iteration over an array style lua table.</para>
		/// <para>while iterating the key is in the iterator, and the value is at -1.</para>
		/// <para>the iteration begins at key 1 and ends directly before the first key that is assigned nil.</para>
		/// <para>when the iteration ends by reaching its end, no value is left on the stack.</para>
		/// <para>if you break the iteration (or leave it otherwise), you have to clean up the value from the stack yourself.</para>
		/// <para>[-0,+1|0,-]</para>
		/// </summary>
		/// <param name="index">valid index to iterate over</param>
		/// <returns>IPairsHolder</returns>
		IPairsHolder IPairs(int index) {
			return IPairsHolder(*this, index);
		}

		/// <summary>
		/// sets the hook. the hook function gets called every time one of the conditions mask is met.
		/// removes any previous hooks.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="F">hook function</param>
		/// <param name="mask">conditions when to call</param>
		/// <param name="count">count parameter for count condition</param>
		template<CppHook F>
		void Debug_SetHook(B::HookEvent mask, int count) {
			B::Debug_SetHook(&CppToCHook<F>, mask, count);
		}
		/// <summary>
		/// returns the depth of the call stack. valid calls to Debug_GetStack are in the range [0,Debug_GetStackDepth()-1].
		/// returns 0, if nothing is on the call stack.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <returns></returns>
		int Debug_GetStackDepth() {
			if (!B::Debug_IsStackLevelValid(0))
				return 0;
			int li = 1, le = 1;
			while (B::Debug_IsStackLevelValid(le)) {
				li = le;
				le *= 2;
			}
			while (li < le) {
				int m = (li + le) / 2;
				if (B::Debug_IsStackLevelValid(m))
					li = m + 1;
				else
					le = m;
			}
			return le;
		}
		class LocalsHolder;
		/// <summary>
		/// allows to iterate over the locals of a DebugInfo. (only if from the current call stack).
		/// each iteration pushes the current value, it needs to be popped by the user.
		/// the iterator contains the locals name and its number (usefull for SetLocal).
		/// </summary>
		/// <param name="i"></param>
		/// <returns></returns>
		LocalsHolder Debug_Locals(typename const B::DebugInfo& i) {
			return LocalsHolder{ *this, i };
		}
		/// <summary>
		/// allows to iterate over the locals of level of the current call stack.
		/// each iteration pushes the current value, it needs to be popped by the user.
		/// the iterator contains the locals name and its number (usefull for SetLocal).
		/// </summary>
		/// <param name="lvl"></param>
		/// <returns></returns>
		LocalsHolder Debug_Locals(int lvl) {
			typename B::DebugInfo i{};
			if (!B::Debug_GetStack(lvl, i, B::DebugInfoOptions::None, false))
				throw lua::LuaException{ "invalid stack level" };
			return Debug_Locals(i);
		}
		class UpvaluesHolder;
		/// <summary>
		/// allows to iterate over the upvalues of a function.
		/// each iteration pushes the current value, it needs to be popped by the user.
		/// the iterator contains the upvalues name and its number (usefull for SetUpvalue).
		/// </summary>
		/// <param name="func"></param>
		/// <returns></returns>
		UpvaluesHolder Debug_Upvalues(int func) {
			return UpvaluesHolder{ *this, func };
		}

		/// <summary>
		/// default formatting for debug string.
		/// </summary>
		struct ToDebugString_Format {
			/// <summary>
			/// formats the source part of a c func.
			/// </summary>
			/// <param name="L"></param>
			/// <param name="index"></param>
			/// <param name="d"></param>
			/// <returns></returns>
			static std::string CFuncSourceFormat(State L, int index, const typename B::DebugInfo& d) {
				return std::format("C:{}", static_cast<void*>(L.ToCFunction(index)));;
			}
			/// <summary>
			/// formats the source part of a lua func.
			/// </summary>
			/// <param name="L"></param>
			/// <param name="index"></param>
			/// <param name="d"></param>
			/// <returns></returns>
			static std::string LuaFuncSourceFormat(State L, int index, const typename B::DebugInfo& d) {
				return std::format("{}:{}", d.ShortSrc, d.LineDefined);;
			}
			/// <summary>
			/// formats a function.
			/// </summary>
			/// <param name="L"></param>
			/// <param name="index"></param>
			/// <param name="d"></param>
			/// <param name="name"></param>
			/// <param name="src"></param>
			/// <returns></returns>
			static std::string FuncFormat(State L, int index, const typename B::DebugInfo& d, std::string_view name, std::string_view src, std::string_view pre, std::string_view post) {
				return std::format("{}{} {} {} (defined in: {}){}", pre, d.What, d.NameWhat, name, src, post);
			}
		};
	private:
		template<class Fmt>
		std::string ToDebugString_Recursive(int index, int tableExpandLevels, size_t indent, std::set<const void*>& tablesDone)
		{
			LType t = B::Type(index);
			switch (t)
			{
			case LType::Nil:
				return "nil";
			case LType::Boolean:
				return B::ToBoolean(index) ? "true" : "false";
			case LType::LightUserdata:
				return std::format("<LightUserdata {}>", B::ToUserdata(index));
			case LType::Number:
				if constexpr (B::Capabilities::NativeIntegers) {
					if (B::IsInteger(index)) {
						return std::format("{}",  *B::ToInteger(index));
					}
				}
				return std::format("{}", *B::ToNumber(index));
			case LType::String:
				return "\"" + ToStdString(index) + "\"";
			case LType::Table:
			{
				auto tp = B::ToPointer(index);
				if (tablesDone.find(tp) != tablesDone.end()) {
					return std::format("<table, recursion {}>", tp);
				}
				if (tableExpandLevels > 0 && B::CheckStack(3)) {
					tablesDone.insert(tp);
					std::stringstream str{};
					str << "{\n";
					for (auto t : Pairs(index)) {
						str << std::string(indent + 1, '\t') << '[' << ToDebugString_Recursive<Fmt>(-2, tableExpandLevels - 1, indent + 1, tablesDone)
							<< "] = " << ToDebugString_Recursive<Fmt>(-1, tableExpandLevels - 1, indent + 1, tablesDone) << ",\n";
					}
					str << std::string(indent, '\t') << "}";
					return str.str();
				}
				return std::format("<table {}>", tp);
			}
			case LType::Function:
			{
				B::PushValue(index);
				typename B::DebugInfo d = B::Debug_GetInfoForFunc(B::DebugInfoOptions::Name | B::DebugInfoOptions::Source | B::DebugInfoOptions::Line);
				auto name = GetNameForFunc(d);
				auto src = B::IsCFunction(index) ? Fmt::CFuncSourceFormat(*this, index, d) : Fmt::LuaFuncSourceFormat(*this, index, d);
				return Fmt::FuncFormat(*this, index, d, name, src, "<function ", ">");
			}
			case LType::Userdata:
			{
				std::string_view ud = "unknown type";
				if (GetMetaField(index, B::MetaEvent::Name)) {
					if (B::IsString(-1))
						ud = ToStringView(-1);
					B::Pop(1);
				}
				return std::format("<Userdata {} {}>", ud, static_cast<void*>(B::ToUserdata(index)));
			}
			case LType::Thread:
				return std::format("<thread {}>", static_cast<void*>(B::ToThread(index).GetState()));
			case LType::None:
				return "<none>";
			default:
				return "<unknown>";
			}
		}
	public:
		/// <summary>
		/// turns the value at index to a debug string.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to check</param>
		/// <returns>debug string</returns>
		template<class Fmt = ToDebugString_Format>
		std::string ToDebugString(int index, int maxTableExpandLevels = 0, size_t indent = 0) {
			std::set<const void*> tablesDone{};
			return ToDebugString_Recursive<Fmt>(index, maxTableExpandLevels, indent, tablesDone);
		}
		/// <summary>
		/// generates a stack trace from levelStart to levelEnd (or the end of the stack).
		/// level 0 is the current running function, and n+1 is the function that called n.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="levelStart">stack level to start</param>
		/// <param name="levelEnd">stack level to end (may end before that, if the end of the stack is reached)</param>
		/// <param name="upvalues">include a ToDebugString of upvalues</param>
		/// <param name="locals">include a ToDebugString of locals</param>
		/// <returns>stack trace</returns>
		template<class Fmt = ToDebugString_Format>
		std::string GenerateStackTrace(int levelStart = 0, int levelEnd = -1, bool upvalues = false, bool locals = false)
		{
			int lvl = levelStart;
			typename B::DebugInfo ar{};
			std::ostringstream trace{};
			while (levelEnd != lvl && B::Debug_GetStack(lvl, ar, B::DebugInfoOptions::Name | B::DebugInfoOptions::Source | B::DebugInfoOptions::Line, true)) {
				auto name = Debug_GetNameForStackFunc(ar);
				auto src = B::IsCFunction(-1) ? Fmt::CFuncSourceFormat(*this, -1, ar) : Fmt::LuaFuncSourceFormat(*this, -1, ar);
				trace << "\t";
				trace << Fmt::FuncFormat(*this, -1, ar, name, src, "", "");
				if (locals) {
					const char* localname;
					int lnum = 1;
					while (localname = B::Debug_GetLocal(lvl, lnum)) {
						trace << "\r\n\t\tlocal " << localname << " = " << ToDebugString(-1);
						B::Pop(1);
						lnum++;
					}
				}
				if (upvalues) {
					const char* upname;
					int unum = 1;
					while (upname = B::Debug_GetUpvalue(-1, unum)) {
						trace << "\r\n\t\tupvalue " << upname << " = " << ToDebugString(-1);
						B::Pop(1);
						unum++;
					}
				}
				B::Pop(1);
				trace << "\r\n";
				lvl++;
			}
			return trace.str();
		}
		/// <summary>
		/// intended to be used with Pcall. attaches a stack trace to its first parameter.
		/// </summary>
		/// <param name="L">lua state</param>
		/// <returns>1</returns>
		static int DefaultErrorDecorator(State L)
		{
			std::ostringstream trace{};
			trace << L.ToString(-1);
			L.Pop(1);
			trace << "\r\nStacktrace:\r\n";
			trace << L.GenerateStackTrace(1, -1, true, true);
			L.Push(trace.str());
			return 1;
		}
		/// <summary>
		/// calls a function. does catch lua exceptions, and throws an LuaException.
		/// first push the function, then the arguments in order, then call.
		/// pops the function and its arguments, then pushes its results.
		/// use MULTIRET to return all values.
		/// if an error gets cought, attaches a stack trace and then throws a LuaException.
		/// <para>[-nargs+1,+nresults|0,t]</para>
		/// </summary>
		/// <param name="nargs">number of parameters</param>
		/// <param name="nresults">number of return values</param>
		/// <returns>number of return values</returns>
		/// <exception cref="lua::LuaException">on lua error</exception>
		int TCall(int nargs, int nresults)
		{
			int t = B::GetTop() - nargs - 1;
			Push<DefaultErrorDecorator>();
			int ehsi = B::ToAbsoluteIndex(-nargs - 2); // just under the func to be called
			B::Insert(ehsi);
			auto c = B::PCall(nargs, nresults, ehsi);
			if (c != B::ErrorCode::Success) {
				std::string msg = B::ErrorCodeFormat(c);
				msg += ToStringView(-1);
				B::Pop(1); // error msg
				B::Remove(ehsi); // DefaultErrorDecorator
				throw LuaException{ msg };
			}
			B::Remove(ehsi); // DefaultErrorDecorator
			return B::GetTop() - t;
		}


		/// <summary>
		/// registers the function f via the key name in index.
		/// use index = -3 to register in the ToS.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="name">key to register</param>
		/// <param name="f">function to register</param>
		/// <param name="index">valid index where to register</param>
		/// <param name="upval">if != nullptr, gets added as only upvalue for the function</param>
		void RegisterFunc(std::string_view name, CFunction f, int index, void* upval = nullptr)
		{
			Push(name);
			if (upval)
				B::PushLightUserdata(upval);
			B::Push(f, upval == nullptr ? 0 : 1);
			B::SetTableRaw(index);
		}
		/// <summary>
		/// registers the function f via the key name in the global environment.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="name">key to register</param>
		/// <param name="f">function to register</param>
		/// <param name="upval">if != nullptr, gets added as only upvalue for the function</param>
		void RegisterFunc(std::string_view name, CFunction f, void* upval = nullptr)
		{
			if (upval)
				B::PushLightUserdata(upval);
			B::Push(f, upval == nullptr ? 0 : 1);
			SetGlobal(name);
		}
		/// <summary>
		/// registers the function f via the key name in index.
		/// use index = -3 to register in the ToS.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="name">key to register</param>
		/// <param name="F">function to register</param>
		/// <param name="index">valid index where to register</param>
		/// <param name="upval">if != nullptr, gets added as only upvalue for the function</param>
		template<CFunction F>
		void RegisterFunc(std::string_view name, int index, void* upval = nullptr) {
			RegisterFunc(name, F, index, upval);
		}
		/// <summary>
		/// registers the function f via the key name in the global environment.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="name">key to register</param>
		/// <param name="F">function to register</param>
		/// <param name="upval">if != nullptr, gets added as only upvalue for the function</param>
		template<CFunction F>
		void RegisterFunc(std::string_view name, void* upval = nullptr) {
			RegisterFunc(name, F, upval);
		}
		/// <summary>
		/// registers the function f via the key name in index.
		/// use index = -3 to register in the ToS.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="name">key to register</param>
		/// <param name="F">function to register</param>
		/// <param name="index">valid index where to register</param>
		/// <param name="upval">if != nullptr, gets added as only upvalue for the function</param>
		template<CppFunction F>
		void RegisterFunc(std::string_view name, int index, void* upval = nullptr)
		{
			RegisterFunc(name, &CppToCFunction<F>, index, upval);
		}
		/// <summary>
		/// registers the function f via the key name in the global environment.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="name">key to register</param>
		/// <param name="F">function to register</param>
		/// <param name="upval">if != nullptr, gets added as only upvalue for the function</param>
		template<CppFunction F>
		void RegisterFunc(std::string_view name, void* upval = nullptr)
		{
			RegisterFunc(name, &CppToCFunction<F>, upval);
		}
		/// <summary>
		/// registers the member function F via the key name in the global environment. The object will take up upvalue 1.
		/// <para>does NOT take ownership of the object, you have to keep it alive for as long as the pushed function might be called and dispose of it afterwards.</para>
		/// <para>if you need GC on a object, a userdata type with call operator might be more appropriate.</para>
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <typeparam name="O">object type</typeparam>
		/// <typeparam name="F">member function to register</typeparam>
		/// <param name="obj">object</param>
		/// <param name="name">key to register</param>
		template<class O, int(O::* F)(State L)>
		void RegisterFunc(O& obj, std::string_view name)
		{
			RegisterFunc(name, &CppToCFunction<MemberClosure<O, F>>, &obj);
		}
		/// <summary>
		/// registers the member function F via the key name in the global environment. The object will take up upvalue 1.
		/// <para>does NOT take ownership of the object, you have to keep it alive for as long as the pushed function might be called and dispose of it afterwards.</para>
		/// <para>needs a non const object, because lua does not have const userdata.</para>
		/// <para>if you need GC on a object, a userdata type with call operator might be more appropriate.</para>
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <typeparam name="O">object type</typeparam>
		/// <typeparam name="F">member function to register</typeparam>
		/// <param name="obj">object</param>
		/// <param name="name">key to register</param>
		template<class O, int(O::* F)(State L) const>
		void RegisterFunc(O& obj, std::string_view name)
		{
			RegisterFunc(name, &CppToCFunction<MemberClosure<O, F>>, &obj);
		}
		/// <summary>
		/// registers all functions in funcs into index.
		/// use index = -3 to register in the ToS.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="funcs">iterable containing FuncReference to register</param>
		/// <param name="index">valid index where to register</param>
		template<class T>
		void RegisterFuncs(const T& funcs, int index) {
			for (const FuncReference& f : funcs) {
				RegisterFunc(f.Name, f.Func, index, f.Upvalue);
			}
		}
		/// <summary>
		/// registers all functions in funcs into the global environment.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="funcs">iterable containing FuncReference to register</param>
		template<class T>
		void RegisterFuncs(const T& funcs) {
			for (const FuncReference& f : funcs) {
				RegisterFunc(f.Name, f.Func, f.Upvalue);
			}
		}
		/// <summary>
		/// registers all functions in funcs into a global table name.
		/// reuses existing table, if present, creates a new one otherwise.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="funcs">iterable containing FuncReference to register</param>
		/// <param name="name">name of the global table</param>
		template<class T>
		void RegisterGlobalLib(const T& funcs, std::string_view name) {
			Push(name);
			Push(name);
			GetGlobal();
			if (!B::IsTable(-1)) {
				B::Pop(1);
				B::NewTable();
			}
			RegisterFuncs(funcs, -3);
			SetGlobal();
		}

		/// <summary>
		/// depending on CatchExceptions, throws a lua::LuaException or calls Error.
		/// in both cases, msg is used as error message.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="msg">error message</param>
		/// <exception cref="lua::LuaException">always</exception>
		[[noreturn]] void ErrorOrThrow(std::string_view msg) {
			if constexpr (CatchExceptions) {
				throw lua::LuaException{ std::string{msg} };
			}
			else {
				Push(msg);
				B::Error();
			}
		}

	private:
		std::string GetNameForFunc_FindField(int i, int level, std::set<const void*>& searched) {
			if (level <= 0 || !B::IsTable(-1))
				return "";
			if (!B::CheckStack(2))
				return "";
			const void* p = B::ToPointer(-1);
			if (searched.find(p) != searched.end())
				return "";
			searched.insert(p);
			for (auto kt : Pairs(-1)) {
				if (kt != lua::LType::String)
					continue;
				if (B::RawEqual(-1, i)) {
					auto r = ToStdString(-2);
					B::Pop(2);
					return r;
				}
				std::string l = GetNameForFunc_FindField(i, level - 1, searched);
				if (!l.empty()) {
					auto r = ToStdString(-2);
					B::Pop(2);
					return r + "." + l;
				}
			}
			return "";
		}

	public:
		/// <summary>
		/// tries to find a suitable name for a given function at the top of the stack.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <returns></returns>
		std::string GetNameForFunc() {
			int i = B::GetTop();
			if constexpr (B::Capabilities::LoadedTable)
				GetTableRaw(B::REGISTRYINDEX, B::REGISTRY_LOADED_TABLE);
			else
				B::PushGlobalTable();
			std::set<const void*> searched{};
			std::string r = GetNameForFunc_FindField(i, 3, searched);
			if (r.starts_with("_G.")) {
				r = r.substr(3);
			}
			B::SetTop(i);
			return r;
		}
		/// <summary>
		/// tries to find a suitable name for a given function at the top of the stack, with a given DebugInfo.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="info"></param>
		/// <returns></returns>
		std::string GetNameForFunc(const typename B::DebugInfo& info) {
			if (info.Name != nullptr && *info.Name != '\0') {
				return info.Name;
			}
			return GetNameForFunc();
		}

		/// <summary>
		/// tries to find a suitable name for the function in the call stack with a given DebugInfo.
		/// requires the DebugInfo to be created with DebugInfoOptions::Name information.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="info"></param>
		/// <returns></returns>
		std::string Debug_GetNameForStackFunc(const typename B::DebugInfo& info) {
			if (info.Name != nullptr && *info.Name != '\0') {
				return info.Name;
			}
			if (!B::Debug_PushDebugInfoFunc(info))
				return "";
			auto r = GetNameForFunc();
			B::Pop(1);
			return r;
		}
		/// <summary>
		/// tries to find a suitable name for the function in the call stack at a given level.
		/// </summary>
		/// <param name="lvl"></param>
		/// <returns></returns>
		std::string Debug_GetNameForStackFunc(int lvl) {
			typename B::DebugInfo i{};
			if (!B::Debug_GetStack(lvl, i, B::DebugInfoOptions::Name, true))
				return "";
			return Debug_GetNameForStackFunc(i);
		}

		/// <summary>
		/// generates an error message of the form
		/// 'bad argument #&lt;arg&gt; to &lt;func&gt; (&lt;extramsg&gt;)'
		/// throws c++ or lua error depending on CatchExceptions.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="arg">argument number</param>
		/// <param name="msg">extra message</param>
		/// <exception cref="lua::LuaException">always</exception>
		[[noreturn]] void ArgError(int arg, std::string_view msg) {
			typename B::DebugInfo i{};
			if (!B::Debug_GetStack(0, i, B::DebugInfoOptions::Name, false))
				ErrorOrThrow(std::format("bad argument #{} ({})", arg, msg));
			if (i.NameWhat != nullptr && i.NameWhat == std::string_view{ "method" }) {
				--arg;
				if (arg == 0)
					ErrorOrThrow(std::format("calling `{}' on bad self ({})", i.Name, msg));
			}
			auto n = Debug_GetNameForStackFunc(0);
			ErrorOrThrow(std::format("bad argument #{} to `{}' ({})", arg, n, msg));
		}
		/// <summary>
		/// calls ArgError if b is not satisfied.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="b">condition</param>
		/// <param name="arg">argument number</param>
		/// <param name="msg">extra message</param>
		/// <exception cref="lua::LuaException">if !b</exception>
		void ArgCheck(bool b, int arg, std::string_view msg) {
			if (!b)
				ArgError(arg, msg);
		}
		/// <summary>
		/// throws an error with the message: "location: bad argument narg to 'func' (tname expected, got rt)"
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">index</param>
		/// <param name="t">expected type</param>
		/// <exception cref="lua::LuaException">always</exception>
		[[noreturn]] void TypeError(int idx, std::string_view t) {
			ArgError(idx, std::format("{} expected, got {}", t, B::TypeName(B::Type(idx))));
		}
		/// <summary>
		/// throws an error with the message: "location: bad argument narg to 'func' (tname expected, got rt)"
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">index</param>
		/// <param name="t">expected type</param>
		/// <exception cref="lua::LuaException">always</exception>
		[[noreturn]] void TypeError(int idx, LType t) {
			TypeError(idx, B::TypeName(t));
		}
		/// <summary>
		/// throws an error if a is not met.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="a">condition</param>
		/// <param name="msg">message</param>
		/// <exception cref="lua::LuaException">if !a</exception>
		void Assert(bool a, std::string_view msg) {
			if (!a)
				ErrorOrThrow(msg);
		}
		/// <summary>
		/// used to build a prefix for error messages, pushes a string of the form 'chunkname:currentline: '
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="lvl">stack level</param>
		void Where(int lvl) {
			typename B::DebugInfo i{};
			if (B::Debug_GetStack(0, i, B::DebugInfoOptions::Source | B::DebugInfoOptions::Line, false)) {
				if (i.LineDefined != 0) {
					B::PushFString("%s:%d: ", i.ShortSrc, i.LineDefined);
					return;
				}
			}
			B::Push("");
			return;
		}


		/// <summary>
		/// pushes the metafield of obj onto the stack.
		/// returns if it found one, pushes nothing if not.
		/// <para>[-0,+1|0,m]</para>
		/// </summary>
		/// <param name="obj">object to check</param>
		/// <param name="ev">event name</param>
		/// <returns>found</returns>
		bool GetMetaField(int obj, std::string_view ev) {
			if (!B::GetMetatable(obj)) {
				return false;
			}
			Push(ev);
			B::GetTableRaw(-2);
			if (B::IsNil(-1)) {
				B::Pop(2);
				return false;
			}
			B::Remove(-2);
			return true;
		}
		/// <summary>
		/// pushes the metafield of obj onto the stack.
		/// returns if it found one, pushes nothing if not.
		/// <para>[-0,+1|0,m]</para>
		/// </summary>
		/// <param name="obj">object to check</param>
		/// <param name="ev">event code</param>
		/// <returns>found</returns>
		bool GetMetaField(int obj, B::MetaEvent ev) {
			return GetMetaField(obj, B::GetMetaEventName(ev));
		}
		/// <summary>
		/// if obj has a metatable and a field ev in it, calls it with obj as its only argument and pushes its return value.
		/// returns if it found a method to call.
		/// <para>[-0,+0|1,e]</para>
		/// </summary>
		/// <param name="obj">valid index to call methamethod of</param>
		/// <param name="ev">event string</param>
		/// <returns>found method</returns>
		bool CallMeta(int obj, std::string_view ev) {
			obj = B::ToAbsoluteIndex(obj);
			if (!GetMetaField(obj, ev)) {
				return false;
			}
			B::PushValue(obj);
			TCall(1, 1);
			return true;
		}
		/// <summary>
		/// if obj has a metatable and a field ev in it, calls it with obj as its only argument and pushes its return value.
		/// returns if it found a method to call.
		/// <para>[-0,+0|1,e]</para>
		/// </summary>
		/// <param name="obj">valid index to call methamethod of</param>
		/// <param name="ev">event</param>
		/// <returns>found method</returns>
		/// <exception cref="lua::LuaException">on lua error</exception>
		bool CallMeta(int obj, B::MetaEvent ev) {
			return CallMeta(obj, B::GetMetaEventName(ev));
		}
		/// <summary>
		/// pushes the metatable associated with name.
		/// <para>[-0,+1,-]</para>
		/// </summary>
		/// <param name="name">metatable name</param>
		void GetMetaTableFromRegistry(std::string_view tname) {
			Push(tname);
			GetTable(B::REGISTRYINDEX);
		}
		/// <summary>
		/// if the registry already has a value associated with name, returns 0. otherwise creates a new table, adds it and returns 1.
		/// in both cases, pushes the final value onto the stack.
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="name">metatable name</param>
		/// <returns>created</returns>
		bool NewMetaTable(std::string_view tname) {
			GetMetaTableFromRegistry(tname);
			if (!B::IsNil(-1)) {
				return false;
			}
			B::Pop(1);
			B::NewTable();
			Push(B::MetaEvent::Name);
			Push(tname);
			B::SetTableRaw(-3);
			Push(tname);
			B::PushValue(-2);
			B::SetTableRaw(B::REGISTRYINDEX);
			return true;
		}
		/// <summary>
		/// checks for an userdata type. (via its metatable).
		/// returns nullptr on fail.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <param name="name">metatable name</param>
		/// <returns>userdata</returns>
		/// <see cref="lua::State::GetUserData"/>
		void* TestUserdata(int idx, std::string_view tname) {
			void* ud = B::ToUserdata(idx);
			if (ud != nullptr) {
				if (B::GetMetatable(idx)) {
					GetMetaTableFromRegistry(tname);
					if (!B::RawEqual(-1, -2)) {
						ud = nullptr;
					}
					B::Pop(2);
					return ud;
				}
			}
			return nullptr;
		}


		/// <summary>
		/// checks if the stack can grow to top + extra elements, and does so if possible.
		/// throws if it cannot.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="extra">number of elements</param>
		/// <param name="msg">extra error message</param>
		/// <exception cref="lua::LuaException">if it cannot grow the stack</exception>
		void CheckStack(int extra, std::string_view msg) {
			if (!B::CheckStack(extra))
				ErrorOrThrow(std::format("stack overflow ({})", msg));
		}
		/// <summary>
		/// checks if the stack contains at least n enements.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="n">n</param>
		/// <exception cref="lua::LuaException">on stack too small</exception>
		void CheckStackHasElements(int n) {
			int t = B::GetTop();
			if (t < n)
				ErrorOrThrow("stack contains not enough elements");
		}
		/// <summary>
		/// checks the type if idx.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <param name="t">type</param>
		/// <exception cref="lua::LuaException">if type does not match</exception>
		void CheckType(int idx, LType t) {
			if (B::Type(idx) != t)
				TypeError(idx, t);
		}
		/// <summary>
		/// checks if there is any argument (including nil) at idx
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <exception cref="lua::LuaException">if none</exception>
		void CheckAny(int idx) {
			if (B::Type(idx) == LType::None)
				ArgError(idx, "value expected");
		}
		/// <summary>
		/// checks if there is a number and returns it.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <returns>number</returns>
		/// <exception cref="lua::LuaException">if not number</exception>
		Number CheckNumber(int idx) {
			auto n = B::ToNumber(idx);
			if (n.has_value())
				return *n;
			TypeError(idx, LType::Number);
		}
		/// <summary>
		/// checks if there is a number and returns it cast to a float.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <returns>float</returns>
		/// <exception cref="lua::LuaException">if not number</exception>
		float CheckFloat(int idx) {
			return static_cast<float>(CheckNumber(idx));
		}
		/// <summary>
		/// checks if there is a integer and returns it.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <returns>int</returns>
		/// <exception cref="lua::LuaException">if not number</exception>
		Integer CheckInteger(int idx) {
			if constexpr (B::Capabilities::NativeIntegers) {
				auto i = B::ToInteger(idx);
				if (i.has_value())
					return *i;
				if (B::IsNumber(idx))
					ArgError(idx, "number has no integer representation");
				else
					TypeError(idx, LType::Number);
			}
			else {
				return static_cast<Integer>(CheckNumber(idx));
			}
		}
		/// <summary>
		/// checks if there is a integer and returns it.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <returns>int</returns>
		/// <exception cref="lua::LuaException">if not number</exception>
		int CheckInt(int idx) {
			if constexpr (B::Capabilities::NativeIntegers) {
				auto i = B::ToInteger(idx);
				if (i.has_value())
					return static_cast<int>(*i);
				if (B::IsNumber(idx))
					ArgError(idx, "number has no integer representation");
				else
					TypeError(idx, LType::Number);
			}
			else {
				return static_cast<int>(CheckNumber(idx));
			}
		}
		/// <summary>
		/// checks if there is a string and returns it.
		/// <para>warning: converts the value on the stack to a string, which might confuse pairs/next</para>
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <param name="len">optional, writes length of the string here, if not nullptr</param>
		/// <returns>c string</returns>
		/// <exception cref="lua::LuaException">if not string</exception>
		const char* CheckString(int idx, size_t* len = nullptr) {
			const char* s = B::ToString(idx, len);
			if (s == nullptr)
				TypeError(idx, LType::String);
			return s;
		}
		/// <summary>
		/// checks if there is a bool and returns it.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <returns>bool</returns>
		/// <exception cref="lua::LuaException">if not bool</exception>
		bool CheckBool(int idx) {
			CheckType(idx, LType::Boolean);
			return B::ToBoolean(idx);
		}
		/// <summary>
		/// checks for an userdata type. (via its metatable).
		/// throws on fail.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <param name="name">metatable name</param>
		/// <returns>userdata</returns>
		/// <see cref="lua::State::GetUserData"/>
		void* CheckUserdata(int idx, std::string_view name) {
			void* ud = TestUserdata(idx, name);
			if (ud == nullptr)
				ArgError(idx, name);
			return ud;
		}

		/// <summary>
		/// loads a string as lua code and executes it. throws on errors.
		/// <para>[-0,+?,m]</para>
		/// </summary>
		/// <param name="code">code</param>
		/// <param name="len">code lenght</param>
		/// <param name="name">code name</param>
		/// <returns>number of return values</returns>
		/// <exception cref="lua::LuaException">on lua exceptions</exception>
		int DoStringT(std::string_view code, const char* name = nullptr) {
			if (!name)
				name = code.data();
			auto e = B::LoadBuffer(code.data(), code.length(), name);
			if (e != B::ErrorCode::Success) {
				std::string msg = B::ErrorCodeFormat(e);
				msg += B::ToString(-1);
				B::Pop(1); // error msg
				throw LuaException{ msg };
			}
			return TCall(0, B::MULTIRET);
		}

		/// <summary>
		/// checks if the table at index has a table under the key name. if it does not, creates one.
		/// in both cases, pushes the table onto the stack.
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="name">key to query</param>
		/// <param name="index">index of the table</param>
		/// <returns>had table before the call</returns>
		bool GetSubTable(std::string_view name, int index) {
			index = B::ToAbsoluteIndex(index);
			Push(name);
			B::GetTableRaw(index);
			if (!B::IsTable(-1)) {
				B::Pop(1);
				B::NewTable();
				Push(name);
				B::PushValue(-2);
				B::SetTableRaw(index);
				return false;
			}
			return true;
		}
		/// <summary>
		/// checks if the global table has a table under the key name. if it does not, creates one.
		/// in both cases, pushes the table onto the stack.
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="name">key to query</param>
		/// <param name="index">index of the table</param>
		/// <returns>had table before the call</returns>
		bool GetSubTable(std::string_view name) {
			Push(name);
			GetGlobal();
			if (!B::IsTable(-1)) {
				B::Pop(1);
				B::NewTable();
				Push(name);
				B::PushValue(-2);
				SetGlobal();
				return false;
			}
			return true;
		}

		/// <summary>
		/// in idx is a number returns it. if idx is none or nil, returns def.
		/// otherwise throws.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">aceptable index to check</param>
		/// <param name="def">default value</param>
		/// <returns>number</returns>
		/// <exception cref="lua::LuaException">if invalid</exception>
		Number OptNumber(int idx, Number def) {
			if (B::IsNoneOrNil(idx))
				return def;
			else
				return CheckNumber(idx);
		}
		/// <summary>
		/// in idx is a number returns it cast to an float. if idx is none or nil, returns def.
		/// otherwise throws.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">aceptable index to check</param>
		/// <param name="def">default value</param>
		/// <returns>float</returns>
		/// <exception cref="lua::LuaException">if invalid</exception>
		float OptFloat(int idx, float def) {
			return static_cast<float>(OptNumber(idx, def));
		}
		/// <summary>
		/// in idx is a number returns it cast to an int. if idx is none or nil, returns def.
		/// otherwise throws.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">aceptable index to check</param>
		/// <param name="def">default value</param>
		/// <returns>int</returns>
		/// <exception cref="lua::LuaException">if invalid</exception>
		int OptInt(int idx, int def) {
			if (B::IsNoneOrNil(idx))
				return def;
			else
				return CheckInt(idx);
		}
		/// <summary>
		/// in idx is a number returns it cast to an Integer. if idx is none or nil, returns def.
		/// otherwise throws.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">aceptable index to check</param>
		/// <param name="def">default value</param>
		/// <returns>int</returns>
		/// <exception cref="lua::LuaException">if invalid</exception>
		Integer OptInteger(int idx, Integer def) {
			if (B::IsNoneOrNil(idx))
				return def;
			else
				return CheckInteger(idx);
		}
		/// <summary>
		/// in idx is a string returns. if idx is none or nil, returns def.
		/// otherwise throws.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">aceptable index to check</param>
		/// <param name="def">default value</param>
		/// <param name="l">optional length out</param>
		/// <returns>c string</returns>
		/// <exception cref="lua::LuaException">if invalid</exception>
		const char* OptString(int idx, const char* def, size_t* l = nullptr) {
			if (B::IsNoneOrNil(idx)) {
				if (l)
					*l = (def ? strlen(def) : 0);
				return def;
			}
			else
				return CheckString(idx, l);
		}
		/// <summary>
		/// in idx is a bool returns it. if idx is none or nil, returns def.
		/// otherwise throws.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">aceptable index to check</param>
		/// <param name="def">default value</param>
		/// <returns>bool</returns>
		/// <exception cref="lua::LuaException">if invalid</exception>
		bool OptBool(int idx, bool def) {
			if (B::IsNoneOrNil(idx))
				return def;
			else
				return CheckBool(idx);
		}

		/// <summary>
		/// creates a unique reference to a value.
		/// pops the value.
		/// <para>[-1,+0,m]</para>
		/// </summary>
		/// <param name="t">table to reference in</param>
		/// <returns>reference</returns>
		Reference Ref(int t = B::REGISTRYINDEX) {
			return { B::RefI(t) };
		}
		/// <summary>
		/// frees the reference r.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="r">reference</param>
		/// <param name="t">table to reference in</param>
		void UnRef(Reference r, int t = B::REGISTRYINDEX) {
			B::UnRefI(r.r, t);
		}
		/// <summary>
		/// pushes the value associated with the reference r.
		/// <para>[-0,+1,-]</para>
		/// </summary>
		/// <param name="r">reference</param>
		/// <param name="t">table to reference in</param>
		void Push(Reference r, int t = B::REGISTRYINDEX) {
			B::GetTableRaw(t, r.r);
		}

		/// <summary>
		/// no valid reference, guranteed to be different from all valid references.
		/// if pushed, pushes nil.
		/// </summary>
		constexpr static Reference NoRef{ B::NOREFI };
		/// <summary>
		/// reference to nil.
		/// </summary>
		constexpr static Reference RefNil{ B::REFNILI };



		/// <summary>
		/// checks if idx is a string and returns it.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <returns>string</returns>
		/// <exception cref="lua::LuaException">if not a string</exception>
		std::string_view CheckStringView(int idx)
		{
			size_t l;
			const char* s = CheckString(idx, &l);
			return { s, l };
		}
		/// <summary>
		/// if idx is a string, returns it. if it is nil or none, returns a copy of def. otherwise throws.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <param name="def">default value</param>
		/// <returns>string</returns>
		/// <exception cref="lua::LuaException">if invalid</exception>
		std::string_view OptStringView(int idx, std::string_view def)
		{
			size_t l;
			const char* s = OptString(idx, def.data(), &l);
			return { s, l };
		}
		/// <summary>
		/// checks if idx is a string and returns it.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <returns>string</returns>
		/// <exception cref="lua::LuaException">if not a string</exception>
		std::string CheckStdString(int idx)
		{
			return std::string{ CheckStringView(idx) };
		}
		/// <summary>
		/// if idx is a string, returns it. if it is nil or none, returns a copy of def. otherwise throws.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <param name="def">default value</param>
		/// <returns>string</returns>
		/// <exception cref="lua::LuaException">if invalid</exception>
		std::string OptStdString(int idx, const std::string& def)
		{
			return std::string{ OptStringView(idx, def) };
		}

		/// <summary>
		/// converts idx to a string, pushes it and returns it.
		/// calls ToString metamethod, if possible.
		/// <para>[-0,+1,e]</para>
		/// </summary>
		/// <param name="idx">valid index to convert</param>
		/// <returns></returns>
		std::string_view ConvertToString(int idx) {
			idx = B::ToAbsoluteIndex(idx);
			if (CallMeta(idx, B::MetaEvent::ToString)) {  /* metafield? */
				if (!B::IsString(-1))
					throw LuaException{ "'__tostring' must return a string" };
			}
			else {
				switch (B::Type(idx)) {
				case LType::Number: {
					if constexpr (B::Capabilities::NativeIntegers) {
						if (B::IsInteger(idx)) {
							Push(std::format("{}", *B::ToInteger(idx)));
							break;
						}
					}
					Push(std::format("{}", *B::ToNumber(idx)));
					break;
				}
				case LType::String:
					B::PushValue(idx);
					break;
				case LType::Boolean:
					B::Push((B::ToBoolean(idx) ? "true" : "false"));
					break;
				case LType::Nil:
					B::Push("nil");
					break;
				default: {
					if (GetMetaField(idx, B::MetaEvent::Name))  /* try name */
					{
						if (B::IsString(-1)) {
							Push(std::format("{}: {}", B::ToString(-1), B::ToPointer(idx)));
							B::Remove(-2);
							break;
						}
						B::Remove(-2);
					}
					Push(std::format("{}: {}", B::TypeName(B::Type(idx)), B::ToPointer(idx)));
					break;
				}
				}
			}
			return ToStringView(-1);
		}


		/// <summary>
		/// iterator over a lua table.
		/// </summary>
		class PairsIter {
			State L;
			int Index;
			bool HasNext = false;

			PairsIter(State L, int i) : L(L), Index(i) {}
		public:
			friend class State;
			friend class PairsHolder;
			inline friend bool operator==(const PairsIter& i, std::default_sentinel_t s) {
				return !i.HasNext;
			}
			inline friend bool operator==(std::default_sentinel_t s, const PairsIter& i) {
				return !i.HasNext;
			}

			/// <summary>
			/// advances the iterator to the next key/value pair.
			/// </summary>
			/// <returns>this</returns>
			/// <exception cref="lua::LuaException">on lua error</exception>
			PairsIter& operator++() {
				L.Pop(1); // value
				HasNext = L.Next(Index);
				return *this;
			}
			/// <summary>
			/// advances the iterator to the next key/value pair.
			/// </summary>
			/// <returns>copy of this</returns>
			/// <exception cref="lua::LuaException">on lua error</exception>
			PairsIter operator++(int) {
				PairsIter r = *this;
				++(*this);
				return r;
			}
			/// <summary>
			/// gets the type of the key
			/// </summary>
			/// <returns>key type</returns>
			LType operator*() {
				return L.Type(-2);
			}
		};
		/// <summary>
		/// holds info to iterate over a table. provides begin and end.
		/// </summary>
		class PairsHolder {
			friend class State;
			State L;
			int Index;
			PairsHolder(State l, int i) : L(l), Index(L.ToAbsoluteIndex(i)) {}
		public:
			/// <summary>
			/// begins a new iteration. pushes the first key/value pair onto the stack.
			/// </summary>
			/// <returns>iterator</returns>
			/// <exception cref="lua::LuaException">on lua error</exception>
			PairsIter begin() {
				L.Push();
				PairsIter i{ L, Index };
				i.HasNext = L.Next(Index);
				return i;
			}
			/// <summary>
			/// marks the end of a table iteration.
			/// </summary>
			/// <returns>iterator sentinel</returns>
			std::default_sentinel_t end() {
				return std::default_sentinel;
			}
		};
		/// <summary>
		/// iterator to iterate over an array style table.
		/// </summary>
		class IPairsIter {
			State L;
			int Index;
			int Key = 1;
			bool HasNext = false;

			IPairsIter(State l, int i) : L(l), Index(i) {}
		public:
			friend class State;
			friend class IPairsHolder;
			inline friend bool operator==(const IPairsIter& i, std::default_sentinel_t) {
				return !i.HasNext;
			}
			inline friend bool operator==(std::default_sentinel_t, const IPairsIter& i) {
				return !i.HasNext;
			}

			/// <summary>
			/// advances the iterator to the next key.
			/// </summary>
			/// <returns>this</returns>
			IPairsIter& operator++() {
				L.Pop(1);
				++Key;
				L.GetTableRaw(Index, Key);
				if (L.Type(-1) == LType::Nil) {
					HasNext = false;
					L.Pop(1);
				}
				else {
					HasNext = true;
				}
				return *this;
			}
			/// <summary>
			/// advances the iterator to the next key.
			/// </summary>
			/// <returns>copy of this</returns>
			IPairsIter operator++(int) {
				IPairsIter r = *this;
				++(*this);
				return r;
			}
			/// <summary>
			/// acesses the current key.
			/// </summary>
			/// <returns>key</returns>
			int operator*() {
				return Key;
			}
		};
		/// <summary>
		/// holds information to iterate over an array style table. provides begin and end.
		/// </summary>
		class IPairsHolder {
			friend class State;
			State L;
			int Index;
			IPairsHolder(State l, int i) : L(l), Index(L.ToAbsoluteIndex(i)) {}
		public:
			/// <summary>
			/// begins a new iteration. pushes the first value onto the stack and provides the first key (1).
			/// </summary>
			/// <returns>iterator</returns>
			IPairsIter begin() {
				IPairsIter i{ L, Index };
				L.GetTableRaw(Index, i.Key);
				if (L.Type(-1) == LType::Nil) {
					i.HasNext = false;
					L.Pop(1);
				}
				else {
					i.HasNext = true;
				}
				return i;
			}
			/// <summary>
			/// marks the end of table.
			/// </summary>
			/// <returns>iterator sentinel</returns>
			std::default_sentinel_t end() {
				return std::default_sentinel;
			}
		};
		/// <summary>
		/// iterator over the locals of a DebugInfo.
		/// </summary>
		class LocalsIter {
			friend class State;
			State L;
			const typename B::DebugInfo& Inf;

		public:
			/// <summary>
			/// local name and number.
			/// </summary>
			struct Local {
				int LocalNum;
				const char* Name;
			};
		private:
			Local Loc;
			LocalsIter(State l, const typename B::DebugInfo& i, int n) : L(l), Inf(i), Loc(n, nullptr) {
			}
		public:
			inline friend bool operator==(const LocalsIter& i, std::default_sentinel_t s) {
				return i.Loc.Name == nullptr;
			}
			inline friend bool operator==(std::default_sentinel_t s, const LocalsIter& i) {
				return i.Loc.Name == nullptr;
			}

			LocalsIter& operator++() {
				Loc.Name = L.Debug_GetLocal(Inf, ++Loc.LocalNum);
				return *this;
			}
			LocalsIter operator++(int) {
				LocalsIter r = *this;
				++(*this);
				return r;
			}
			const Local& operator*() {
				return Loc;
			}
		};
		/// <summary>
		/// holds information to iterate over the locals of a DebugInfo. provides begin and end.
		/// </summary>
		class LocalsHolder {
			friend class State;
			State L;
			typename B::DebugInfo Inf;

			LocalsHolder(State l, typename B::DebugInfo i) : L(l), Inf(i) {}
		public:
			LocalsIter begin() {
				LocalsIter r{ L, Inf, 0 };
				return ++r;
			}
			std::default_sentinel_t end() {
				return std::default_sentinel;
			}
		};
		/// <summary>
		/// iterator over the upvalues of a function.
		/// </summary>
		class UpvaluesIter {
			friend class State;
			State L;
			int Func;

		public:
			/// <summary>
			/// local name and number.
			/// </summary>
			struct Upval {
				int UpvalNum;
				const char* Name;
			};
		private:
			Upval Upv;
			UpvaluesIter(State l, int f, int n) : L(l), Func(f), Upv(n, nullptr) {
			}
		public:
			inline friend bool operator==(const UpvaluesIter& i, std::default_sentinel_t s) {
				return i.Upv.Name == nullptr;
			}
			inline friend bool operator==(std::default_sentinel_t s, const UpvaluesIter& i) {
				return i.Upv.Name == nullptr;
			}

			UpvaluesIter& operator++() {
				Upv.Name = L.Debug_GetUpvalue(Func, ++Upv.UpvalNum);
				return *this;
			}
			UpvaluesIter operator++(int) {
				UpvaluesIter r = *this;
				++(*this);
				return r;
			}
			const Upval& operator*() {
				return Upv;
			}
		};
		/// <summary>
		/// holds information to iterate over the upvalues of a function. provides begin and end.
		/// </summary>
		class UpvaluesHolder {
			friend class State;
			State L;
			int Func;

			UpvaluesHolder(State l, int f) : L(l), Func(l.ToAbsoluteIndex(f)) {}
		public:
			UpvaluesIter begin() {
				UpvaluesIter r{ L, Func, 0 };
				return ++r;
			}
			std::default_sentinel_t end() {
				return std::default_sentinel;
			}
		};

		/// <summary>
		/// checks if i is an userdata of type T (or able to be cast to T) and returns it. returns nullptr if not.
		/// </summary>
		/// <typeparam name="T">class type</typeparam>
		/// <param name="i">acceptable index to check</param>
		/// <returns>obj</returns>
		template<class T>
		T* OptionalUserClass(int i) {
			if constexpr (userdata::BaseDefined<T>) {
				if (B::Type(i) != LType::Userdata)
					return nullptr;
				if (!B::GetMetatable(i))
					return nullptr;
				Push(BaseTypeNameName);
				B::GetTableRaw(-2);
				if (B::Type(-1) != LType::String) {
					B::Pop(2);
					return nullptr;
				}
				if (typename_details::type_name<typename T::BaseClass>() != ToStringView(-1)) {
					B::Pop(2);
					return nullptr;
				}
				B::Pop(2);
				// do not acces ActualObj here, this might be of a different type alltogether
				auto* u = static_cast<userdata::UserClassBase<typename T::BaseClass>*>(B::ToUserdata(i));
				return dynamic_cast<T*>(u->BaseObj);
			}
			else {
				return static_cast<T*>(TestUserdata(i, typename_details::type_name<T>()));
			}
		}
		/// <summary>
		/// checks if i is an userdata of type T (or able to be cast to T) abd returns it. throws if not.
		/// </summary>
		/// <typeparam name="T">class type</typeparam>
		/// <param name="i">acceptable index to check</param>
		/// <returns>obj</returns>
		/// <exception cref="lua::LuaException">if type does not match</exception>
		template<class T>
		T* CheckUserClass(int i) {
			T* t = OptionalUserClass<T>(i);
			if (t == nullptr)
				ErrorOrThrow(std::format("no {} at argument {}", typename_details::type_name<T>(), i));
			return t;
		}
		/// <summary>
		/// gets the metatable for type T.
		/// <para>lua class generation:</para>
		/// <para>if T::LuaMethods is iterable over LuaReference, registers them all as userdata methods (__index).</para>
		/// <para></para>
		/// <para>if T is not trivially destructable, generates a finalizer (__gc) that calls its destructor.</para>
		/// <para></para>
		/// <para>metatable operator definition:</para>
		/// <para>- by CFunction or CppFunction: you provide an implementation as a static class member (used, if both provided).</para>
		/// <para>- or automatically by C++ operator overloads (this requires a nothrow move constructor for operators).</para>
		/// <para></para>
		/// <para>== comparator (also ~= comparator) (__eq):</para>
		/// <para>- Equals static member.</para>
		/// <para>- == operator overload (or &lt;==&gt; overload) (checks type, then operator).</para>
		/// <para></para>
		/// <para>&lt; comparator (also &gt; comparator) (__lt):</para>
		/// <para>- LessThan static member.</para>
		/// <para>- &lt; operator overload (or &lt;==&gt; overload) (checks type, then operator).</para>
		/// <para></para>
		/// <para>&lt;= comparator (also &lt;= comparator) (__le):</para>
		/// <para>- LessOrEquals static member.</para>
		/// <para>- &lt;= operator overload (or &lt;==&gt; overload) (checks type, then operator).</para>
		/// <para></para>
		/// <para>+ operator (__add):</para>
		/// <para>- Add static member.</para>
		/// <para>- + operator overload (only works for both operands of type T).</para>
		/// <para></para>
		/// <para>- operator (__sub):</para>
		/// <para>- Substract static member.</para>
		/// <para>- - operator overload (only works for both operands of type T).</para>
		/// <para></para>
		/// <para>* operator (__mul):</para>
		/// <para>- Multiply static member.</para>
		/// <para>- * operator overload (only works for both operands of type T).</para>
		/// <para></para>
		/// <para>/ operator (__div):</para>
		/// <para>- Divide static member.</para>
		/// <para>- / operator overload (only works for both operands of type T).</para>
		/// <para></para>
		/// <para>// operator (__idiv):</para>
		/// <para>- IntegerDivide static member.</para>
		/// <para>(no operator in c++).</para>
		/// <para></para>
		/// <para>% operator (__mod):</para>
		/// <para>- Modulo static member.</para>
		/// <para>(no operator in c++).</para>
		/// <para></para>
		/// <para>^ operator (__pow):</para>
		/// <para>- Pow static member.</para>
		/// <para>(no operator in c++).</para>
		/// <para></para>
		/// <para>unary - operator (__unm):</para>
		/// <para>- UnaryMinus static member.</para>
		/// <para>- (unary) - operator overload.</para>
		/// <para></para>
		/// <para>&amp; operator (__band):</para>
		/// <para>- BitwiseAnd static member.</para>
		/// <para>- &amp; operator overload (only works for both operands of type T).</para>
		/// <para></para>
		/// <para>| operator (__bor):</para>
		/// <para>- BitwiseOr static member.</para>
		/// <para>- | operator overload (only works for both operands of type T).</para>
		/// <para></para>
		/// <para>~ operator (__bxor):</para>
		/// <para>- BitwiseXOr static member.</para>
		/// <para>- ^ operator overload (only works for both operands of type T).</para>
		/// <para></para>
		/// <para>unary ~ operator (__bnot):</para>
		/// <para>- BitwiseNot static member.</para>
		/// <para>- (unary) ~ operator overload.</para>
		/// <para></para>
		/// <para>&lt;&lt; operator (__shl):</para>
		/// <para>- ShiftLeft static member.</para>
		/// <para>- &lt;&lt; operator overload (only works for both operands of type T).</para>
		/// <para></para>
		/// <para>&gt;&gt; operator (__shr):</para>
		/// <para>- ShiftRight static member.</para>
		/// <para>- &gt;&gt; operator overload (only works for both operands of type T).</para>
		/// <para></para>
		/// <para># operator (__len):</para>
		/// <para>- Length static member.</para>
		/// <para>(no operator in c++).</para>
		/// <para></para>
		/// <para>.. operator (__concat):</para>
		/// <para>- Concat static member.</para>
		/// <para>(no operator in c++).</para>
		/// <para></para>
		/// <para>[x]=1 operator (__newindex):</para>
		/// <para>- NewIndex static member.</para>
		/// <para></para>
		/// <para>(...) operator (__call)</para>
		/// <para>- Call static member.</para>
		/// <para></para>
		/// <para>=[x] operator (__index):</para>
		/// <para>- Index static member.</para>
		/// <para></para>
		/// <para>tostring (lua) ConvertToString (c++):</para>
		/// <para>- ToString static member.</para>
		/// <para></para>
		/// <para>if T has both LuaMethods and Index defined, first LuaMethods is searched, and if nothing is found, Index is called.</para>
		/// <para></para>
		/// <para>if T::LuaMetaMethods is iterable over LuaReference, registers them all into the metatable as additional metamethods (possibly overriding the default generaded)</para>
		/// <para></para>
		/// <para>to handle inheritance, define T::BaseClass as T in the base class and do not change the typedef in the derived classes.</para>
		/// <para>a call to GetUserData&lt;T::BaseClass&gt; on an userdata of type T will then return a correctly cast pointer to T::BaseClass.</para>
		/// <para>all variables for class generation get used via normal overload resolution, meaning the most derived class wins.</para>
		/// <para>make sure you include all methods from base classes in LuaMethods, or they will get lost.</para>
		/// <para>as far as luapp is concerned a class may only have one base class (defined via T::BaseClass) but other inheritances that are not visible to luapp are allowed.</para>
		/// </summary>
		/// <typeparam name="T">type to generate metatable for</typeparam>
		template<class T>
		void GetUserClassMetatable() {
			if (NewMetaTable(typename_details::type_name<T>())) {
				if constexpr (userdata::IndexCpp<State<B>, T>) {
					RegisterFunc<userdata::IndexOperator<State<B>, T>>(B::GetMetaEventName(B::MetaEvent::Index), -3);
					if constexpr (userdata::HasLuaMethods<T>) {
						Push(MethodsName);
						B::NewTable();
						RegisterFuncs(T::LuaMethods, -3);
						B::SetTableRaw(-3);
					}
				}
				else if constexpr (userdata::HasLuaMethods<T>) {
					Push(B::GetMetaEventName(B::MetaEvent::Index));
					B::NewTable();
					RegisterFuncs(T::LuaMethods, -3);
					B::SetTableRaw(-3);
				}

				if constexpr (!std::is_trivially_destructible_v<T>)
					RegisterFunc<userdata::Finalizer<State<B>, T>>(B::GetMetaEventName(B::MetaEvent::Finalizer), -3);

				if constexpr (userdata::EquatableCpp<State<B>, T>)
					RegisterFunc<T::Equals>(B::GetMetaEventName(B::MetaEvent::Equals), -3);
				else if constexpr (userdata::EquatableOp<T>)
					RegisterFunc<userdata::EqualsOperator<State<B>, T>>(B::GetMetaEventName(B::MetaEvent::Equals), -3);

				if constexpr (userdata::LessThanCpp<State<B>, T>)
					RegisterFunc<T::LessThan>(B::GetMetaEventName(B::MetaEvent::LessThan), -3);
				else if constexpr (userdata::LessThanOp<T>)
					RegisterFunc<userdata::LessThanOperator<State<B>, T>>(B::GetMetaEventName(B::MetaEvent::LessThan), -3);

				if constexpr (userdata::LessThanEqualsCpp<State<B>, T>)
					RegisterFunc<T::LessOrEquals>(B::GetMetaEventName(B::MetaEvent::LessOrEquals), -3);
				else if constexpr (userdata::LessThanEqualsOp<T>)
					RegisterFunc<userdata::LessThanEqualsOperator<State<B>, T>>(B::GetMetaEventName(B::MetaEvent::LessOrEquals), -3);

				if constexpr (userdata::AddCpp<State<B>, T>)
					RegisterFunc<T::Add>(B::GetMetaEventName(B::MetaEvent::Add), -3);
				else if constexpr (userdata::AddOp<T>)
					RegisterFunc<userdata::AddOperator<State<B>, T>>(B::GetMetaEventName(B::MetaEvent::Add), -3);

				if constexpr (userdata::SubtractCpp<State<B>, T>)
					RegisterFunc<T::Substract>(B::GetMetaEventName(B::MetaEvent::Subtract), -3);
				else if constexpr (userdata::SubtractOp<T>)
					RegisterFunc<userdata::SubtractOperator<State<B>, T>>(B::GetMetaEventName(B::MetaEvent::Subtract), -3);

				if constexpr (userdata::MultiplyCpp<State<B>, T>)
					RegisterFunc<T::Multiply>(B::GetMetaEventName(B::MetaEvent::Multiply), -3);
				else if constexpr (userdata::MultiplyOp<T>)
					RegisterFunc<userdata::MultiplyOperator<State<B>, T>>(B::GetMetaEventName(B::MetaEvent::Multiply), -3);

				if constexpr (userdata::DivideCpp<State<B>, T>)
					RegisterFunc<T::Divide>(B::GetMetaEventName(B::MetaEvent::Divide), -3);
				else if constexpr (userdata::DivideOp<T>)
					RegisterFunc<userdata::DivideOperator<State<B>, T>>(B::GetMetaEventName(B::MetaEvent::Divide), -3);

				if constexpr (B::Capabilities::NativeIntegers) {
					if constexpr (userdata::IntegerDivideCpp<State<B>, T>)
						RegisterFunc<T::IntegerDivide>(B::GetMetaEventName(B::MetaEvent::IntegerDivide), -3);
				}

				if constexpr (B::Capabilities::MetatableLengthModulo) {
					if constexpr (userdata::ModuloCpp<State<B>, T>)
						RegisterFunc<T::Modulo>(B::GetMetaEventName(B::MetaEvent::Modulo), -3);
				}

				if constexpr (userdata::PowCpp<State<B>, T>)
					RegisterFunc<T::Pow>(B::GetMetaEventName(B::MetaEvent::Pow), -3);

				if constexpr (userdata::UnaryMinusCpp<State<B>, T>)
					RegisterFunc<T::UnaryMinus>(B::GetMetaEventName(B::MetaEvent::UnaryMinus), -3);
				else if constexpr (userdata::UnaryMinusOp<T>)
					RegisterFunc<userdata::UnaryMinusOperator<State<B>, T>>(B::GetMetaEventName(B::MetaEvent::UnaryMinus), -3);

				if constexpr (B::Capabilities::NativeIntegers) {
					if constexpr (userdata::BitwiseAndCpp<State<B>, T>)
						RegisterFunc<T::BitwiseAnd>(B::GetMetaEventName(B::MetaEvent::BitwiseAnd), -3);
					else if constexpr (userdata::BitwiseAndOp<T>)
						RegisterFunc<userdata::BitwiseAndOperator<State<B>, T>>(B::GetMetaEventName(B::MetaEvent::BitwiseAnd), -3);

					if constexpr (userdata::BitwiseOrCpp<State<B>, T>)
						RegisterFunc<T::BitwiseOr>(B::GetMetaEventName(B::MetaEvent::BitwiseOr), -3);
					else if constexpr (userdata::BitwiseOrOp<T>)
						RegisterFunc<userdata::BitwiseOrOperator<State<B>, T>>(B::GetMetaEventName(B::MetaEvent::BitwiseOr), -3);

					if constexpr (userdata::BitwiseXOrCpp<State<B>, T>)
						RegisterFunc<T::BitwiseXOr>(B::GetMetaEventName(B::MetaEvent::BitwiseXOr), -3);
					else if constexpr (userdata::BitwiseXOrOp<T>)
						RegisterFunc<userdata::BitwiseXOrOperator<State<B>, T>>(B::GetMetaEventName(B::MetaEvent::BitwiseXOr), -3);

					if constexpr (userdata::BitwiseNotCpp<State<B>, T>)
						RegisterFunc<T::BitwiseNot>(B::GetMetaEventName(B::MetaEvent::BitwiseNot), -3);
					else if constexpr (userdata::BitwiseNotOp<T>)
						RegisterFunc<userdata::BitwiseNotOperator<State<B>, T>>(B::GetMetaEventName(B::MetaEvent::BitwiseNot), -3);

					if constexpr (userdata::ShiftLeftCpp<State<B>, T>)
						RegisterFunc<T::ShiftLeft>(B::GetMetaEventName(B::MetaEvent::ShiftLeft), -3);
					else if constexpr (userdata::ShiftLeftOp<T>)
						RegisterFunc<userdata::ShiftLeftOperator<State<B>, T>>(B::GetMetaEventName(B::MetaEvent::ShiftLeft), -3);

					if constexpr (userdata::ShiftRightCpp<State<B>, T>)
						RegisterFunc<T::ShiftRight>(B::GetMetaEventName(B::MetaEvent::ShiftRight), -3);
					else if constexpr (userdata::ShiftRightOp<T>)
						RegisterFunc<userdata::ShiftRightOperator<State<B>, T>>(B::GetMetaEventName(B::MetaEvent::ShiftRight), -3);
				}

				if constexpr (B::Capabilities::MetatableLengthModulo) {
					if constexpr (userdata::LengthCpp<State<B>, T>)
						RegisterFunc<T::Length>(B::GetMetaEventName(B::MetaEvent::Length), -3);
				}

				if constexpr (userdata::ConcatCpp<State<B>, T>)
					RegisterFunc<T::Concat>(B::GetMetaEventName(B::MetaEvent::Concat), -3);

				if constexpr (userdata::NewIndexCpp<State<B>, T>)
					RegisterFunc<T::NewIndex>(B::GetMetaEventName(B::MetaEvent::NewIndex), -3);

				if constexpr (userdata::CallCpp<State<B>, T>)
					RegisterFunc<T::Call>(B::GetMetaEventName(B::MetaEvent::Call), -3);

				if constexpr (userdata::ToStringCpp<State<B>, T>)
					RegisterFunc<T::ToString>(B::GetMetaEventName(B::MetaEvent::ToString), -3);

				if constexpr (userdata::HasLuaMetaMethods<T>)
					RegisterFuncs(T::LuaMetaMethods, -3);

				Push(B::MetaEvent::Name);
				Push(typename_details::type_name<T>());
				B::SetTableRaw(-3);
				Push(BaseTypeNameName);
				if constexpr (userdata::BaseDefined<T>) {
					static_assert(std::derived_from<T, typename T::BaseClass>);
					Push(typename_details::type_name<typename T::BaseClass>());
				}
				else
					Push(typename_details::type_name<T>());
				B::SetTableRaw(-3);
			}
		}
		template<class T>
		void PrepareUserClassType() {
			GetUserClassMetatable<T>();
			B::Pop(1);
		}
	private:
		template<class T>
		void* UserClassAlloc(int uvs) {
			if constexpr (B::Capabilities::ArbitraryUservalues) {
				return B::NewUserdata(sizeof(T), uvs);
			}
			else {
				return B::NewUserdata(sizeof(T));
			}
		}
		template<class T, class ... Args>
		T* UserClassNew(int uvs, Args&& ... args) {
			return new (UserClassAlloc<T>(uvs)) T(std::forward<Args>(args)...);
		}
		template<class T, class ... Args>
		T* UserClassNewBase(int uvs, Args&& ... args) {
			if constexpr (userdata::BaseDefined<T>) {
				return &UserClassNew<userdata::UserClassHolder<T, typename T::BaseClass>>(uvs, std::forward<Args>(args)...)->ActualObj;
			}
			else {
				return UserClassNew<T>(uvs, std::forward<Args>(args)...);
			}
		}
		template<class T, class ... Args>
		T* NewUserClassUnchecked(int uvs, Args&& ... args) {
			T* r = UserClassNewBase<T>(uvs, std::forward<Args>(args)...);
			GetUserClassMetatable<T>();
			B::SetMetatable(-2);
			return r;
		}
	public:
		/// <summary>
		/// <para>converts a c++ class to a lua userdata. creates a new full userdata and calls the constructor of T, forwarding all arguments.</para>
		/// <para>a class (metatable) for a userdata type is only generated once, and then reused for all userdata of the same type.</para>
		/// <para></para>
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <typeparam name="T">type to create</typeparam>
		/// <typeparam name="...Args">parameters for constructor</typeparam>
		/// <param name="...args">parameters for constructor</param>
		/// <returns>obj</returns>
		template<class T, class ... Args>
		requires userdata::UserClassUserValuesValid<State<B>, T>
		T* NewUserClass(Args&& ... args) {
			return NewUserClassUnchecked<T>(userdata::UserClassUserValues<T>(), std::forward<Args>(args)...);
		}
		/// <summary>
		/// <para>converts a c++ class to a lua userdata. creates a new full userdata and calls the constructor of T, forwarding all arguments.</para>
		/// <para>a class (metatable) for a userdata type is only generated once, and then reused for all userdata of the same type.</para>
		/// <para></para>
		/// <para>lua class generation: see lua::State::NewUserData</para>
		/// </summary>
		/// <typeparam name="T">type to create</typeparam>
		/// <typeparam name="...Args">parameters for constructor</typeparam>
		/// <param name="nuvalues">number of user values</param>
		/// <param name="...args">parameters for constructor</param>
		/// <returns>obj</returns>
		template<class T, class ... Args>
		requires B::Capabilities::ArbitraryUservalues
		T* NewUserClassWithUserValues(int nuvalues, Args&& ... args) {
			if (nuvalues > userdata::StateMaxUservalues<State<B>>())
				throw lua::LuaException{ std::format("this lua state only supports {} uservalues, instead of the requested {}", userdata::StateMaxUservalues<State<B>>(), nuvalues) };
			return NewUserClassUnchecked<T>(nuvalues, std::forward<Args>(args)...);
		}
	};

	/// <summary>
	/// holds a lua state and automatically closes it, if it goes out of scope.
	/// </summary>
	/// <typeparam name="B"></typeparam>
	template<class B>
	class UniqueState : public State<B> {
	public:
		/// <summary>
		/// creates a State from a lua_State* (usually from external APIs).
		/// </summary>
		/// <param name="L">state pointer</param>
		UniqueState(lua_State* L) : State<B>(L) {}
		/// <summary>
		/// opens a new lua state.
		/// </summary>
		/// <param name="io">open io and os libs</param>
		/// <param name="debug">open debug lib</param>
		UniqueState(bool io = true, bool debug = false) : State<B>(io, debug) {}

		UniqueState(const UniqueState&) = delete;
		UniqueState(UniqueState&& s) noexcept : State<B>(s.GetState()) { s.L = nullptr; }
		UniqueState(const State<B>& s) noexcept : State<B>(s) {}
		UniqueState(State<B>&& s) noexcept : State<B>(s) {}

		UniqueState& operator=(const UniqueState&) = delete;
		UniqueState& operator=(UniqueState&& s) noexcept {
			if (this->L == s.L)
				return;
			State<B>::Close();
			this->L = s.L;
			s.L = nullptr;
		}
		UniqueState& operator=(const State<B>& s) noexcept {
			if (this->L == s.L)
				return;
			State<B>::Close();
			this->L = s.L;
		};
		UniqueState& operator=(State<B>&& s) noexcept {
			if (this->L == s.L)
				return;
			State<B>::Close();
			this->L = s.L;
		}

		~UniqueState() {
			State<B>::Close();
		}
	};
}

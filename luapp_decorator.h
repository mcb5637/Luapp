#pragma once
#include <format>
#include <string_view>
#include <sstream>

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
		constexpr static const char* MethodsName = "Methods";
		constexpr static const char* TypeNameName = "TypeName";
		constexpr static const char* BaseTypeNameName = "BaseTypeName";

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
		/// info to register a function to lua.
		/// </summary>
		struct FuncReference {
			const char* Name;
			CFunction Func;

			constexpr FuncReference(const char* name, CFunction f) {
				Name = name;
				Func = f;
			}

			template<CppFunction F>
			constexpr static FuncReference GetRef(const char* name) {
				return { name, &CppToCFunction<F> };
			}
			template<CFunction F>
			constexpr static FuncReference GetRef(const char* name) {
				return { name, F };
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
			B::Push(static_cast<lua::Integer>(i));
		}
		using B::Push;

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
		/// pushes the result onto the stack.
		/// <para>[-0,+1,e]</para>
		/// </summary>
		/// <param name="index">index to query</param>
		/// <exception cref="lua::LuaException">on lua error</exception>
		void ObjLength(int index) {
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
			bool hasoneparam = op == B::ArihmeticOperator::UnaryNegation || op == B::ArihmeticOperator::BitwiseNot;
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
		/// turns the value at index to a debug string.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to check</param>
		/// <returns>debug string</returns>
		std::string ToDebugString(int index)
		{
			LType t = B::Type(index);
			switch (t)
			{
			case LType::Nil:
				return "nil";
			case LType::Boolean:
				return B::ToBoolean(index) ? "true" : "false";
			case LType::LightUserdata:
				return std::format("<LightUserdata {}>", static_cast<void*>(B::ToUserdata(index)));
			case LType::Number:
				return std::to_string(*B::ToNumber(index));
			case LType::String:
				return "\"" + ToStdString(index) + "\"";
			case LType::Table:
				return std::format("<table {}>", B::ToPointer(index));
			case LType::Function:
			{
				B::PushValue(index);
				typename B::DebugInfo d = B::Debug_GetInfoForFunc(B::DebugInfoOptions::Name | B::DebugInfoOptions::Source | B::DebugInfoOptions::Line);
				std::ostringstream name{};
				name << "<function ";
				name << d.What << " ";
				name << d.NameWhat << " ";
				name << (d.Name ? d.Name : "null") << " (defined in:";
				name << d.ShortSrc << ":";
				name << d.CurrentLine << ")>";
				return name.str();
			}
			case LType::Userdata:
			{
				std::string_view ud = "";
				if (GetMetaField(-1, TypeNameName)) {
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
		std::string GenerateStackTrace(int levelStart = 0, int levelEnd = -1, bool upvalues = false, bool locals = false)
		{
			int lvl = levelStart;
			typename B::DebugInfo ar{};
			std::ostringstream trace{};
			while (levelEnd != lvl && B::Debug_GetStack(lvl, ar, B::DebugInfoOptions::Name | B::DebugInfoOptions::Source | B::DebugInfoOptions::Line, upvalues)) {
				trace << "\t";
				trace << ar.What << " ";
				trace << ar.NameWhat << " ";
				trace << (ar.Name ? ar.Name : "null") << " (defined in:";
				trace << ar.ShortSrc << ":";
				trace << ar.CurrentLine << ")";
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
					B::Pop(1);
				}
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
		/// use MULTIRET to return all values, use GetTop tofigure out how many got returned.
		/// if an error gets cought, attaches a stack trace and then throws a LuaException.
		/// <para>[-nargs+1,+nresults|0,t]</para>
		/// </summary>
		/// <param name="nargs">number of parameters</param>
		/// <param name="nresults">number of return values</param>
		/// <exception cref="lua::LuaException">on lua error</exception>
		void TCall(int nargs, int nresults)
		{
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
		}


		/// <summary>
		/// registers the function f via the key name in index.
		/// use index = -3 to register in the ToS.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="name">key to register</param>
		/// <param name="f">function to register</param>
		/// <param name="index">valid index where to register</param>
		void RegisterFunc(const char* name, CFunction f, int index)
		{
			B::Push(name);
			B::Push(f);
			B::SetTableRaw(index);
		}
		/// <summary>
		/// registers the function f via the key name in the global environment.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="name">key to register</param>
		/// <param name="f">function to register</param>
		void RegisterFunc(const char* name, CFunction f)
		{
			B::Push(f);
			B::SetGlobal(name);
		}
		/// <summary>
		/// registers the function f via the key name in index.
		/// use index = -3 to register in the ToS.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="name">key to register</param>
		/// <param name="F">function to register</param>
		/// <param name="index">valid index where to register</param>
		template<CFunction F>
		void RegisterFunc(const char* name, int index) {
			RegisterFunc(name, F, index);
		}
		/// <summary>
		/// registers the function f via the key name in the global environment.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="name">key to register</param>
		/// <param name="F">function to register</param>
		template<CFunction F>
		void RegisterFunc(const char* name) {
			RegisterFunc(name, F);
		}
		/// <summary>
		/// registers the function f via the key name in index.
		/// use index = -3 to register in the ToS.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="name">key to register</param>
		/// <param name="F">function to register</param>
		/// <param name="index">valid index where to register</param>
		template<CppFunction F>
		void RegisterFunc(const char* name, int index)
		{
			RegisterFunc(name, &CppToCFunction<F>, index);
		}
		/// <summary>
		/// registers the function f via the key name in the global environment.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="name">key to register</param>
		/// <param name="F">function to register</param>
		template<CppFunction F>
		void RegisterFunc(const char* name)
		{
			RegisterFunc(name, &CppToCFunction<F>);
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
				RegisterFunc(f.Name, f.Func, index);
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
				RegisterFunc(f.Name, f.Func);
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
		void RegisterGlobalLib(const T& funcs, const char* name) {
			B::Push(name);
			B::Push(name);
			B::GetGlobal();
			if (!B::IsTable(-1)) {
				B::Pop(1);
				B::NewTable();
			}
			RegisterFuncs(funcs, -3);
			B::SetGlobal();
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
			if (i.NameWhat == std::string_view{ "method" }) {
				--arg;
				if (arg == 0)
					ErrorOrThrow(std::format("calling `{}' on bad self ({})", i.Name, msg));
			}
			if (i.Name == nullptr) {
				i.Name = "?";
			}
			ErrorOrThrow(std::format("bad argument #{} to `{}' ({})", arg, i.Name, msg));
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
			B::Push(tname);
			B::GetTable(B::REGISTRYINDEX);
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
				return 0;
			}
			B::Pop(1);
			B::NewTable();
			B::Push(B::MetaEvent::Name);
			Push(tname);
			B::SetTableRaw(-3);
			Push(tname);
			B::PushValue(-1);
			B::SetTableRaw(B::REGISTRYINDEX);
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
		void CheckStack(int extra, const char* msg) {
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
		/// <exception cref="lua::LuaException">on lua exceptions</exception>
		void DoStringT(const char* code, size_t len = 0, const char* name = nullptr) {
			if (!name)
				name = code;
			if (len == 0)
				len = strlen(code);
			auto e = B::LoadBuffer(code, len, name);
			if (e != B::ErrorCode::Success) {
				std::string msg = B::ErrorCodeFormat(e);
				msg += B::ToString(-1);
				B::Pop(1); // error msg
				throw LuaException{ msg };
			}
			TCall(0, B::MULTIRET);
		}

		/// <summary>
		/// checks if the table at index has a table under the key name. if it does not, creates one.
		/// in both cases, pushes the table onto the stack.
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="name">key to query</param>
		/// <param name="index">index of the table</param>
		/// <returns>had table before the call</returns>
		bool GetSubTable(const char* name, int index) {
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
		bool GetSubTable(const char* name) {
			Push(name);
			B::GetGlobal();
			if (!B::IsTable(-1)) {
				B::Pop(1);
				B::NewTable();
				Push(name);
				B::PushValue(-2);
				B::SetGlobal();
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
					Push(std::format("{}", *B::Tonumber(idx)));
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
							PushFString("%s: %p", B::ToString(-1), B::ToPointer(idx));
							B::Remove(-2);
							break;
						}
						B::Remove(-2);
					}
					PushFString("%s: %p", B::TypeName(B::Type(idx)), B::ToPointer(idx));
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

			friend class State;
			friend class PairsHolder;
			inline friend bool operator==(const PairsIter& i, std::default_sentinel_t s) {
				return !i.HasNext;
			}
			inline friend bool operator==(std::default_sentinel_t s, const PairsIter& i) {
				return !i.HasNext;
			}

			PairsIter(State L, int i) : L(L), Index(i) {}
		public:
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

			friend class State;
			friend class IPairsHolder;
			inline friend bool operator==(const IPairsIter& i, std::default_sentinel_t) {
				return !i.HasNext;
			}
			inline friend bool operator==(std::default_sentinel_t, const IPairsIter& i) {
				return !i.HasNext;
			}

			IPairsIter(State l, int i) : L(l), Index(i) {}
		public:
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
		///  holds information to iterate over an array style table. provides begin and end.
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
	};

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
		UniqueState(UniqueState&& s) : State<B>(s.GetState()) { B = nullptr; }

		~UniqueState() {
			State<B>::Close();
		}
	};
}

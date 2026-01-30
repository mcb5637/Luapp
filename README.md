# Luapp
C++ Lua bridge.
- Does not leak functions/structs into the global namespace (except lua_State and lua_Debug).
- Simple functions that should get inlined by any C++ compiler, giving you the same performance as if you were using the C API directly.
- Translating between Lua and C++ exceptions.
- Powerfull templates to create C++ classes into full userdata.
- Intended to be used as git submodule.

Usage:
- only use one version at a time
- make sure the following headers are includable: constexprTypename.h, luapp_common.h, luapp_decorator.h, luapp_userdata.h
- include the header of your version (example: luapp54.h for lua 5.4)
- add the cpp file of your version to compile (example: luapp54.cpp for lua 5.4)
- if you selected lua jit, also add luapp51.cpp and make sure, luapp51.h is includable
	(luapp51.cpp will include luajit instead of lua51 if lua51 is not found. it will default to lua51, if both are found, but they are ABI compatible, so using both jit or 51 is fine at that point)
- link against your lua lib/dll as usual

AutoTranslateAPI:
- allows automatic translation of parameters and return values of a function
- used when a function `F` is used in the following ways: `L.Push<F>()`, `L.RegisterFunc<F>(name)` or `lua::FuncReference::Get(UC)Ref<F>(name)`
- to be allowed, the following conditions need to be met:
  - all parameters need to be Checkable (== `L.Check<T>()` results in the correct type)
  - the return value is Pushable (== `L.Push(r)` is valid) or be a tuple of Pushable values
  - both conditions do respect state Extensions
  - `lua::UserClassChecked<T>` checks for a UserClass of type `T` (with the usual cast rules), treat is as a pointer
  - `lua::PushNewUserClass(std::in_place_type<T>, ...)` pushes a new UserClass of type `T` with the ctor parameters `...`
  - special for UserClass `T` methods (either autoregistered metamethods or via `lua::FuncReference::GetUCRef<F>(name)` manually registered (meta)methods)
    allows `T*` and `T&` (and their const counterparts) in the parameter list

UserClass:
- c++ class as full userdata
- metatable will be automatically build from class T
  - `T::LuaMethods` iterable over `lua::FuncReference`, registered as metamethods in `__index`
  - `T::~T()` (destructor), registered as finalizer `__gc` (skipped if trivial)
  - operators: by c++ operator or member function (member function preferred)
    (member function in this context is a static `CppFunction` or a (mabe static) member function that follows the requirements of AutoTranslateAPI)
    - `==` (`__eq`): `Equals` member function or `==`/`<=>` operator
    - `<`and `>` (`__lt`): `LessThan` member function or `<`/`<=>` operator
    - `<=` and `>=` (`__le`): `LessOrEquals` member function or `<=`/`<=>` operator
    - `+` (`__add`): `Add` member function or `+` operator
    - `-` (`__sub`): `Subtract` member function or `-` operator
    - `*` (`__mul`): `Multiply` member function or `*` operator
    - `/` (`__div`): `Divide` member function or `/` operator
    - `//` (`__idiv`): `IntegerDivide` member function
    - `%` (`__mod`): `Modulo` member function
    - `^` (`__pow`): `Pow` member function
    - unary `-` (`__unm`): `UnaryMinus` member function or unary `-` operator
    - `&` (`__band`): `BitwiseAnd` member function or `&` operator
    - `|` (`__bor`): `BitwiseOr` member function or `|` operator
    - `~` (`__bxor`): `BitwiseXOr` member function or `^` operator
    - unary `~` (`__bnot`): `BitwiseNot` member function or unary `~` operator
    - `<<` (`__shl`): `ShiftLeft` member function or `<<` operator
    - `>>` (`__shr`): `ShiftRight` member function or `>>` operator
    - `#` (`__len`): `Length` member function
    - `..` (`__concat`): `Concat` member function
    - `[]=` (`__newindex`): `NewIndex` member function or
    - `()` (`__call`): `Call` member function
    - `[]` (`__index`): `Index` member function
    - `tostring()` (`__tostring`): `ToString` member function
  - member functions need to be enabled with `static constexpr bool UserClassMetaMethods = true`
  - operators need to be enabled with `static constexpr bool UserClassOperatorTranslate = true`
  - if `T` has both `LuaMethods` and `Index` defined, first `LuaMethods` is searched, and if nothing is found, `Index` is called
  - if `T::LuaMetaMethods` is iterable over LuaReference, registers them all into the metatable as additional metamethods (possibly overriding the default generated)
  - inheritance:
  - a subclass must mark its parents via `using InheritsFrom = std::tuple<ParentA, ...>`
  - (parents that are not marked will be invisible to luapp)
  - `CheckUserClass<ParentA>` and `OptionalUserClass<ParentA>` will recognize objects of a subclass and correctly cast them, even in case of multi-inheritance
  - `LuaMethods` of parent classes are registered before own, so overriding methods is possible
  - (note that subclasses with no own methods should define an empty `LuaMethods`, otherwise the parents methods will be registered multiple times)
  - all other metatable entries are only generated for the child class (but their underlying operators/functions may be inherited)
  - create with `L.NewUserClass<T>(...)` which calls the constructor with the supplied parameters
  - access with `L.CheckUserClass<T>(index)` (note that `L.ToUserdata(index)` may return a different pointer, to allow for polymorphism)

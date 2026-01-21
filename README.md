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


UserClass:
- c++ class as full userdata
- metatable will be automatically build from class T
  - `T::LuaMethods` iterable over `LuaReference`, registered as metamethods in `__index`
  - `T::~T()` (destructor), registered as finalizer `__gc` (skipped if trivial)
  - operators: by c++ operator or static function (static function preferred)
    - `==` (`__eq`): `Equals` static member or `==`/`<=>` operator
    - `<`and `>` (`__lt`): `LessThan` static member or `<`/`<=>` operator
    - `<=` and `>=` (`__le`): `LessOrEquals` static member or `<=`/`<=>` operator
    - `+` (`__add`): `Add` static member or `+` operator
    - `-` (`__sub`): `Subtract` static member or `-` operator
    - `*` (`__mul`): `Multiply` static member or `*` operator
    - `/` (`__div`): `Divide` static member or `/` operator
    - `//` (`__idiv`): `IntegerDivide` static member
    - `%` (`__mod`): `Modulo` static member
    - `^` (`__pow`): `Pow` static member
    - unary `-` (`__unm`): `UnaryMinus` static member or unary `-` operator
    - `&` (`__band`): `BitwiseAnd` static member or `&` operator
    - `|` (`__bor`): `BitwiseOr` static member or `|` operator
    - `~` (`__bxor`): `BitwiseXOr` static member or `^` operator
    - unary `~` (`__bnot`): `BitwiseNot` static member or unary `~` operator
    - `<<` (`__shl`): `ShiftLeft` static member or `<<` operator
    - `>>` (`__shr`): `ShiftRight` static member or `>>` operator
    - `#` (`__len`): `Length` static member
    - `..` (`__concat`): `Concat` static member
    - `[]=` (`__newindex`): `NewIndex` static member or
    - `()` (`__call`): `Call` static member
    - `[]` (`__index`): `Index` static member
    - `tostring()` (`__tostring`): `ToString` static member
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

#pragma once
#include <span>
#include <sstream>

namespace lua::serialization
{
    template<class T>
    struct StreamIO
    {
        T& Stream;

        void Write(std::span<const char> data)
        {
            Stream.write(data.data(), static_cast<std::streamsize>(data.size()));
        }

        void Read(std::span<char> data)
        {
            Stream.read(data.data(), static_cast<std::streamsize>(data.size()));
            if (Stream.gcount() != static_cast<std::streamsize>(data.size()))
                throw std::format_error{ "read error, eof" };
        }
    };

    template<class T>
    concept IsIO = requires(T& stream, std::span<const char> w, std::span<char> r)
    {
        stream.Write(w);
        stream.Read(r);
    };

    static_assert(IsIO<StreamIO<std::stringstream>>);

    /**
     * lua serialization and deserialization.
     * generally, serialize writes the type (so it can write a reference instead) and deserialize checks type then
     * dispatches. To serialize Userdata:
     * - put a serializer function into metatable[UserdataSerializerMetaEvent], func(ud)->typenameString, anything.
     * - and registry a deserializer for that name in GetUserdataDeserializer(typenameString) ->
     * lua::CppToCFunction<...>, func(anything)->ud
     * - anything can be any serializable lua value, including a table, but may not reference the to-be-serialized
     * userdata (other serializable userdata is allowed).
     * @tparam IO read/write funcs
     * @tparam State lua state
     * @tparam DataOnly if true, throws on serializing/deserializing functions or userdata
     */
    template<IsIO IO, class State, bool DataOnly = false>
    class LuaSerializer
    {
        struct Reference {
            LType Type;
            const void* Id;

            auto operator<=>(const Reference&) const = default;
        };
        struct UpvalueReference {
            int FuncReference;
            int UpvalueNum;
        };

        IO Stream;
        State L;
        std::vector<char> Data;
        std::map<Reference, int> RefToNumber;
        std::map<const void*, UpvalueReference> UpRefs;
        int NextReferenceNumber = 1;
        int IndexOfReferenceHolder = 0;

        // a reference to something already serialized/deserialized
        static constexpr LType ReferenceType = static_cast<LType>(-2);
        // a reference to a already serilaized/deserialized upvalue
        static constexpr LType UpvalueReferenceType = static_cast<LType>(-3);
        // lua::Integer
        static constexpr LType IntegerType = static_cast<LType>(-5);

		static constexpr int FileVersion = 3;

        bool CanSerialize(int idx)
        {
            switch (L.Type(idx)) {
            case LType::Nil:
            case LType::Boolean:
            case LType::Number:
            case LType::String:
            case LType::Table:
                return true;
            case LType::Function:
            {
                if (L.IsCFunction(idx))
                    return false;
                if constexpr (!State::Capabilities::UpvalueId) {
                    L.PushValue(idx);
                    DebugInfo i = L.Debug_GetInfoForFunc(DebugInfoOptions::Upvalues);
                    if (i.NumUpvalues > 0) {
                        //shok::LogString("AdvLuaStateSerializer: cannot serialize function with upvalues");
                        return false;
                    }
                }
                return true;
            }
            case LType::Userdata:
            {
                if (!L.GetMetatable(idx))
                    return false;
                L.Push(State::MetaEvent::Serialize);
                L.GetTableRaw(-2);
                bool r = L.IsFunction(-1);
                L.Pop(2);
                return r;
            }
            default:
                return false;
            }
        }

        template<class T>
        void WritePrimitive(T d)
        {
            Stream.Write(std::span(reinterpret_cast<char*>(&d), sizeof(T)));
        }
        template<class T>
        T ReadPrimitive()
        {
            T d;
            Stream.Read(std::span(reinterpret_cast<char*>(&d), sizeof(T)));
            return d;
        }

        void WriteLenPrefixed(std::span<const char> data)
        {
            WritePrimitive(data.size());
            Stream.Write(data);
        }
        std::span<char> ReadLenPrefixed()
        {
            size_t len = ReadPrimitive<size_t>();
            Data.resize(len);
            Stream.Read(Data);
            return Data;
        }

        LType DeserializeType()
        {
            auto r = ReadPrimitive<LType>();
            if ((static_cast<int>(r) < static_cast<int>(LType::Nil) || static_cast<int>(r) > static_cast<int>(LType::Thread))
                && r != ReferenceType && r != UpvalueReferenceType && r != IntegerType)
                throw std::format_error{ "error reading lua type, not a valid type" };
            return r;
        }

        void SerializeBool(int idx)
        {
            WritePrimitive(LType::Boolean);
            WritePrimitive(L.ToBoolean(idx));
        }
        void DeserializeBool()
        {
            L.Push(ReadPrimitive<bool>());
        }

        void SerializeNumber(int idx)
        {
            if constexpr (State::Capabilities::NativeIntegers)
            {
                if (L.IsInteger(idx))
                {
                    WritePrimitive(IntegerType);
                    WritePrimitive(*L.ToInteger(idx));
                    return;
                }
            }
            WritePrimitive(LType::Number);
            WritePrimitive(*L.ToNumber(idx));
        }
        void DeserializeNumber()
        {
            L.Push(ReadPrimitive<Number>());
        }
        void DeserializeInteger()
        {
            if constexpr (!State::Capabilities::NativeIntegers)
                throw std::format_error{ "cannot read integer, not supported by state" };
            L.Push(ReadPrimitive<Integer>());
        }

        void SerializeString(int idx)
        {
            WritePrimitive(LType::String);
            std::string_view s = L.ToStringView(idx);
            WriteLenPrefixed(std::span(s.data(), s.size()));
        }
        void DeserializeString()
        {
            auto s = ReadLenPrefixed();
            L.Push(std::string_view(s.data(), s.size()));
        }

        void SerializeReference(int ref)
        {
            WritePrimitive(ref);
        }
        int DeserializeReferenceRaw()
        {
            return ReadPrimitive<int>();;
        }
        void DeserializeReference()
        {
            auto ref = DeserializeReferenceRaw();
            L.GetTableRaw(IndexOfReferenceHolder, ref);
            if (L.IsNil(-1))
                throw std::format_error{ "error reading reference, invalid" };
        }

        void SerializeTable(int idx, bool isglobal = false);
        void DeserializeTable(bool create);

        void SerializeFunction(int idx);
        void DeserializeFunction();

        void SerializeUserdata(int idx);
        void DeserializeUserdata();

        void SerializeAnything(int idx);
        void DeserializeAnything(LType t);
        void DeserializeAnything()
        {
            DeserializeAnything(DeserializeType());
        }

        void PrepareSerialize()
        {
            WritePrimitive(FileVersion);
            WritePrimitive(L.Version());
            IndexOfReferenceHolder = L.GetTop();
        }
        void CleanupSerialize()
        {
            L.SetTop(IndexOfReferenceHolder);
        }

        void PrepareDeserialize()
        {
            int v = ReadPrimitive<int>();
            if (v == FileVersion) {
                auto luaver = ReadPrimitive<double>();
                if (L.Version() != luaver)
                    throw std::format_error{ "file version missmatch" };
            }
            else {
                throw std::format_error{ "invalid fileversion" };
            }
            L.NewTable();
            IndexOfReferenceHolder = L.GetTop();
        }
        void CleanupDeserialize(bool ret)
        {
            if (ret) {
                L.Remove(IndexOfReferenceHolder);
            }
            else {
                L.SetTop(IndexOfReferenceHolder - 1);
            }
        }

    public:
        LuaSerializer(IO s, State l) : Stream(s), L(l) {}

        /**
         * serializes a single variable
         * @param i index to serialize
         */
        void SerializeVariable(int i)
        {
            PrepareSerialize();

            SerializeAnything(i);

            CleanupSerialize();
        }
        /**
         * deserializes a single variable and pushes it
         */
        void DeserializeVariable()
        {
            PrepareDeserialize();

            DeserializeAnything();

            CleanupDeserialize(true);
        }

        /**
         * serializes multiple variables
         * @param n number of variables to serialite, L.GetTop(), if < 0
         */
        void SerializeStack(int n = -1)
        {
            if (n < 0)
                n = L.GetTop();

            PrepareSerialize();

            WritePrimitive(n);

            for (int i = 1; i <= n; ++i)
                SerializeAnything(i);

            CleanupSerialize();
        }
        /**
         * deserializes multiple variables and pushes them
         */
        void DeserializeStack()
        {
            PrepareDeserialize();

            int n = ReadPrimitive<int>();

            for (int i = 1; i <= n; ++i) {
                DeserializeAnything();
            }

            CleanupDeserialize(true);
        }

        /**
         * serializes all globals and the serialized registry.
         * use IsGlobalSkipped to skip single globals.
         */
        void SerializeState()
        {
            PrepareSerialize();

            L.PushGlobalTable();
            SerializeTable(-1, true);

            L.PushSerializedRegistry();
            SerializeTable(-1, false);

            CleanupSerialize();
        }
        /**
         * deserializes all globals and the serialized registry.
         */
        void DeserializeState()
        {
            PrepareDeserialize();

            L.PushGlobalTable();
            if (DeserializeType() != lua::LType::Table)
                throw std::format_error{ "_G is not a table" };
            DeserializeTable(false);

            L.Push(State::RegistrySerializeKey);
            DeserializeAnything();
            L.SetTableRaw(L.REGISTRYINDEX);

            CleanupDeserialize(false);
        }

        CFunction (*GetUserdataDeserializer)(std::string_view) = [](std::string_view) -> CFunction
        {
            throw std::format_error{ "no GetUserdataDeserializer set" };
        };
        bool (*IsGlobalSkipped)(std::string_view) = [](std::string_view)
        {
            return false;
        };
    };

    template <IsIO IO, class State, bool DataOnly>
    void LuaSerializer<IO, State, DataOnly>::SerializeTable(int idx, bool isglobal){
        Reference r{ LType::Table, L.ToPointer(idx) };
        auto refn = RefToNumber.find(r);
        if (refn == RefToNumber.end()) {
            L.CheckStack(5);
            int refnum = NextReferenceNumber++;
            RefToNumber.insert(std::make_pair(r, refnum));
            WritePrimitive(LType::Table);
            SerializeReference(refnum);
            idx = L.ToAbsoluteIndex(idx);
            for ([[maybe_unused]] auto _ : L.Pairs(idx)) {
                if (CanSerialize(-1) && CanSerialize(-2)) {
                    if (isglobal && L.IsString(-2) && IsGlobalSkipped(L.ToString(-2)))
                        continue;
                    SerializeAnything(-2);
                    SerializeAnything(-1);
                }
            }
            WritePrimitive(LType::Nil); // i use serialized nil as end of table
            if (L.GetMetatable(-1)) {
                SerializeAnything(-1);
                L.Pop(1);
            }
            else
                WritePrimitive(LType::Nil);
        }
        else {
            WritePrimitive(ReferenceType);
            SerializeReference(refn->second);
        }
    }
    template <IsIO IO, class State, bool DataOnly>
    void LuaSerializer<IO, State, DataOnly>::DeserializeTable(bool create) {
        int ref = DeserializeReferenceRaw();
        if (create) {
            L.NewTable();
        }

        L.PushValue(-1);
        L.SetTableRaw(IndexOfReferenceHolder, ref);

        while (true) {
            DeserializeAnything();
            if (L.IsNil(-1)) { // i use serialized nil as end of table
                L.Pop(1);
                break;
            }
            DeserializeAnything();
            L.SetTableRaw(-3);
        }

        DeserializeAnything();
        if (L.IsTable(-1)) // table or nil
            L.SetMetatable(-2);
        else
            L.Pop(1);
    }

    template <IsIO IO, class State, bool DataOnly>
    void LuaSerializer<IO, State, DataOnly>::SerializeFunction(int idx) {
        if constexpr (DataOnly)
        {
            throw std::format_error{ "functions not allowed" };
        }
        L.PushValue(idx);
        DebugInfo i = L.Debug_GetInfoForFunc(DebugInfoOptions::Upvalues);
        if constexpr (!State::Capabilities::UpvalueId) {
            if (i.NumUpvalues > 0)
                throw std::format_error{ "cannot serialize a lua function with upvalues" };
        }

        Reference r{ LType::Function, L.ToPointer(idx) };
        auto refn = RefToNumber.find(r);
        if (refn == RefToNumber.end()) {
            int refnum = NextReferenceNumber++;
            RefToNumber.insert(std::make_pair(r, refnum));
            WritePrimitive(LType::Function);
            SerializeReference(refnum);
            L.PushValue(idx);
            L.Dump([](lua_State*, const void* d, size_t len, void* ud) {
                if (len > 0)
                {
                    auto* th = static_cast<LuaSerializer*>(ud);
                    th->WriteLenPrefixed(std::span(static_cast<const char*>(d), len));
                }
                return 0;
            }, this);
            WriteLenPrefixed(std::array<char, 0>{});
            L.Pop(1);
            WritePrimitive(i.NumUpvalues);
            if constexpr (State::Capabilities::UpvalueId) {
                for (int n = 1; n <= i.NumUpvalues; ++n) {
                    const void* id = L.Debug_UpvalueID(idx, n);
                    auto uref = UpRefs.find(id);
                    if (uref == UpRefs.end()) {
                        UpRefs.insert(std::make_pair(id, UpvalueReference{ refnum, n }));
                        L.Debug_GetUpvalue(idx, n);
                        SerializeAnything(-1);
                        L.Pop(1);
                    }
                    else {
                        WritePrimitive(UpvalueReferenceType);
                        WritePrimitive(uref->second.FuncReference);
                        WritePrimitive(uref->second.UpvalueNum);
                    }
                }
            }
        }
        else {
            WritePrimitive(ReferenceType);
            SerializeReference(refn->second);
        }
    }
    template<IsIO IO, class State, bool DataOnly>
    void LuaSerializer<IO, State, DataOnly>::DeserializeFunction()
    {
        if constexpr (DataOnly)
            throw std::format_error{ "functions not allowed" };
        int ref = DeserializeReferenceRaw();
        L.Load([](lua_State*, void* ud, size_t* len) -> const char* {
            auto* th = static_cast<LuaSerializer*>(ud);
            auto r = th->ReadLenPrefixed();
            if (r.empty())
            {
                *len = 0;
                return nullptr;
            }
            *len = r.size();
            return r.data();
            }, this, nullptr);
        if (!Data.empty()) // last read
        {
            if (!ReadLenPrefixed().empty())
                throw std::format_error{ "parsing function failed" };
        }
        L.PushValue(-1);
        L.SetTableRaw(IndexOfReferenceHolder, ref);
        int n_upvalues = ReadPrimitive<int>();
        if constexpr (State::Capabilities::UpvalueId) {
            for (int n = 1; n <= n_upvalues; ++n) {
                LType t = DeserializeType();
                if (t == UpvalueReferenceType) {
                    int funcref = ReadPrimitive<int>();
                    int upnum = ReadPrimitive<int>();
                    L.GetTableRaw(IndexOfReferenceHolder, funcref);
                    if (L.IsNil(-1))
                        throw std::format_error{ "error reading upvalue reference, invalid" };
                    L.Debug_UpvalueJoin(-2, n, -1, upnum);
                    L.Pop(1);
                }
                else {
                    DeserializeAnything(t);
                    L.Debug_SetUpvalue(-2, n);
                }
            }
        }
        else {
            if (n_upvalues > 0)
                throw std::format_error{ "attempting to deserialize func with upvalues when upvalue serialization is not available" };
        }
    }

    template<IsIO IO, class State, bool DataOnly>
    void LuaSerializer<IO, State, DataOnly>::SerializeUserdata(int idx)
    {
        if constexpr (DataOnly)
            throw std::format_error{ "functions not allowed" };
        Reference r{ LType::Userdata, L.ToPointer(idx) };
        auto refn = RefToNumber.find(r);
        if (refn == RefToNumber.end()) {
            idx = L.ToAbsoluteIndex(idx);
            int refnum = NextReferenceNumber++;
            RefToNumber.insert(std::make_pair(r, refnum));
            WritePrimitive(LType::Userdata);
            SerializeReference(refnum);
            if (!L.GetMetatable(idx))
                throw std::format_error{ "cannot serialize a userdata without serializer" };
            L.Push(State::MetaEvent::Serialize);
            L.GetTableRaw(-2);
            if (!L.IsFunction(-1))
                throw std::format_error{ "cannot serialize a userdata without serializer function" };
            L.PushValue(idx);
            L.TCall(1, 2);
            SerializeString(-2); // name
            SerializeAnything(-1); // probably a table or function
            L.Pop(3);
        }
        else {
            WritePrimitive(ReferenceType);
            SerializeReference(refn->second);
        }
    }
    template<IsIO IO, class State, bool DataOnly>
    void LuaSerializer<IO, State, DataOnly>::DeserializeUserdata()
    {
        if (DataOnly)
            throw std::format_error{ "functions not allowed" };
        int ref = DeserializeReferenceRaw();

        if (DeserializeType() != LType::String)
            throw std::format_error{ "deserialize udata name error" };
        auto tn = ReadLenPrefixed();
        auto f = GetUserdataDeserializer(std::string_view(tn.data(), tn.size()));
        if (f == nullptr)
            throw std::format_error{ "deserialize udata name not found" };
        L.Push(f);
        DeserializeAnything();
        L.TCall(1, 1);

        L.PushValue(-1);
        L.SetTableRaw(IndexOfReferenceHolder, ref);
    }

    template<IsIO IO, class State, bool DataOnly>
    void LuaSerializer<IO, State, DataOnly>::SerializeAnything(int idx)
    {
        switch (L.Type(idx)) {
        case LType::Nil:
            WritePrimitive(LType::Nil);
            break;
        case LType::Boolean:
            SerializeBool(idx);
            break;
        case LType::Number:
            SerializeNumber(idx);
            break;
        case LType::String:
            SerializeString(idx);
            break;
        case LType::Table:
            SerializeTable(idx);
            break;
        case LType::Function:
            SerializeFunction(idx);
            break;
        case LType::Userdata:
            SerializeUserdata(idx);
            break;
        default:
            throw std::format_error{ "invalid type" };
        }
    }
    template<IsIO IO, class State, bool DataOnly>
    void LuaSerializer<IO, State, DataOnly>::DeserializeAnything(LType t)
    {
        switch (t) {
        case LType::Nil:
            L.Push();
            break;
        case LType::Boolean:
            DeserializeBool();
            break;
        case LType::Number:
            DeserializeNumber();
            break;
        case IntegerType:
            DeserializeInteger();
            break;
        case LType::String:
            DeserializeString();
            break;
        case LType::Table:
            DeserializeTable(true);
            break;
        case LType::Function:
            DeserializeFunction();
            break;
        case LType::Userdata:
            DeserializeUserdata();
            break;
        case ReferenceType:
            DeserializeReference();
            break;
        default:
            throw std::format_error{ "invalid type" };
        }
    }
} // namespace lua::serialization

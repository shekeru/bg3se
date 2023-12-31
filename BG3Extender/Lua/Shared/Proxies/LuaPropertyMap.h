#pragma once

#include <Lua/LuaHelpers.h>
#include <Lua/Shared/LuaLifetime.h>
#include <unordered_map>

BEGIN_NS(lua)

template <class T>
void Serialize(lua_State* L, T* obj);

template <class T>
void Unserialize(lua_State* L, int index, T* obj);

class GenericPropertyMap : Noncopyable<GenericPropertyMap>
{
public:
	using TFallbackGetter = PropertyOperationResult (lua_State* L, LifetimeHandle const& lifetime, void* object, FixedString const& prop);
	using TFallbackSetter = PropertyOperationResult (lua_State* L, void* object, FixedString const& prop, int index);
	using TConstructor = void (void*);
	using TDestructor = void (void*);
	using TSerializer = void (lua_State* L, void*);
	using TUnserializer = void (lua_State* L, int index, void*);

	struct RawPropertyAccessors
	{
		using Getter = PropertyOperationResult (lua_State* L, LifetimeHandle const& lifetime, void* object, std::size_t offset, uint64_t flag);
		using Setter = PropertyOperationResult (lua_State* L, void* object, int index, std::size_t offset, uint64_t flag);
		using Serializer = PropertyOperationResult (lua_State* L, void* object, std::size_t offset, uint64_t flag);
		using Unserializer = PropertyOperationResult (lua_State* L, void* object, int index, std::size_t offset, uint64_t flag);

		FixedString Name;
		std::size_t Offset;
		uint64_t Flag;
		Getter* Get;
		Setter* Set;
		Serializer* Serialize;
		Unserializer* Unserialize;
	};

	struct RawPropertyValidators
	{
		using Validator = bool (void* object, std::size_t offset, uint64_t flag);

		FixedString Name;
		Validator* Validate;
		std::size_t Offset;
		uint64_t Flag;
	};

	enum class ValidationState
	{
		Unknown,
		Valid,
		Invalid
	};

	void Init(int registryIndex);
	void Finish();
	bool HasProperty(FixedString const& prop) const;
	PropertyOperationResult GetRawProperty(lua_State* L, LifetimeHandle const& lifetime, void* object, FixedString const& prop) const;
	PropertyOperationResult SetRawProperty(lua_State* L, void* object, FixedString const& prop, int index) const;
	void AddRawProperty(char const* prop, typename RawPropertyAccessors::Getter* getter, typename RawPropertyAccessors::Setter* setter,
		typename RawPropertyAccessors::Serializer* serialize, typename RawPropertyAccessors::Unserializer* unserialize, 
		std::size_t offset, uint64_t flag = 0);
	void AddRawValidator(char const* prop, typename RawPropertyValidators::Validator* validate, std::size_t offset, uint64_t flag = 0);
	void AddRawProperty(char const* prop, typename RawPropertyAccessors::Getter* getter,
		typename RawPropertyAccessors::Setter* setter, typename RawPropertyValidators::Validator* validate, 
		typename RawPropertyAccessors::Serializer* serialize, typename RawPropertyAccessors::Unserializer* unserialize,
		std::size_t offset, uint64_t flag = 0);
	bool IsA(int typeRegistryIndex) const;
	bool ValidatePropertyMap(void* object);
	bool ValidateObject(void* object);

	FixedString Name;
	std::unordered_map<FixedString, RawPropertyAccessors> Properties;
	std::vector<RawPropertyValidators> Validators;
	std::vector<FixedString> Parents;
	std::vector<int> ParentRegistryIndices;
	TFallbackGetter* FallbackGetter{ nullptr };
	TFallbackSetter* FallbackSetter{ nullptr };
	TConstructor* Construct{ nullptr };
	TDestructor* Destroy{ nullptr };
	TSerializer* Serialize{ nullptr };
	TUnserializer* Unserialize{ nullptr };
	std::size_t Size{ 0 };
	bool IsInitializing{ false };
	bool Initialized{ false };
	ValidationState Validated{ ValidationState::Unknown };
	int RegistryIndex{ -1 };
};

inline PropertyOperationResult GenericSetNonWriteableProperty(lua_State* L,  void* obj, int index, std::size_t offset, uint64_t)
{
	return PropertyOperationResult::UnsupportedType;
}

inline PropertyOperationResult GenericSetReadOnlyProperty(lua_State* L, void* obj, int index, std::size_t offset, uint64_t)
{
	return PropertyOperationResult::ReadOnly;
}

inline PropertyOperationResult GenericNullSerializeProperty(lua_State* L, void* object, std::size_t offset, uint64_t flag)
{
	return PropertyOperationResult::UnsupportedType;
}

inline PropertyOperationResult GenericNullUnserializeProperty(lua_State* L, void* object, int index, std::size_t offset, uint64_t flag)
{
	return PropertyOperationResult::UnsupportedType;
}

inline PropertyOperationResult GenericUnserializeReadOnlyProperty(lua_State* L, void* object, int index, std::size_t offset, uint64_t flag)
{
	return PropertyOperationResult::ReadOnly;
}

inline bool GenericValidateNoopProperty(void* obj, std::size_t offset, uint64_t)
{
	return true;
}

template <class T>
void DefaultConstruct(void* ptr)
{
	new (reinterpret_cast<T*>(ptr)) T();
}

template <class T>
void DefaultDestroy(void* ptr)
{
	(reinterpret_cast<T*>(ptr))->~T();
}

template <class T>
void DefaultSerialize(lua_State* L, void* ptr)
{
	Serialize(L, reinterpret_cast<T*>(ptr));
}

template <class T>
void DefaultUnserialize(lua_State* L, int index, void* ptr)
{
	Unserialize(L, index, reinterpret_cast<T*>(ptr));
}

template <class T>
class LuaPropertyMap : public GenericPropertyMap
{
public:
	struct PropertyAccessors
	{
		using Getter = PropertyOperationResult (lua_State* L, LifetimeHandle const& lifetime, T* object, std::size_t offset, uint64_t flag);
		using Setter = PropertyOperationResult (lua_State* L, T* object, int index, std::size_t offset, uint64_t flag);
		using Serializer = PropertyOperationResult (lua_State* L, T* object, std::size_t offset, uint64_t flag);
		using Unserializer = PropertyOperationResult (lua_State* L, T* object, int index, std::size_t offset, uint64_t flag);
		using Validator = bool (T* object, std::size_t offset, uint64_t flag);
		using FallbackGetter = PropertyOperationResult (lua_State* L, LifetimeHandle const& lifetime, T* object, FixedString const& prop);
		using FallbackSetter = PropertyOperationResult (lua_State* L, T* object, FixedString const& prop, int index);
	};

	inline PropertyOperationResult GetProperty(lua_State* L, LifetimeHandle const& lifetime, T* object, FixedString const& prop) const
	{
#if defined(DEBUG_TRAP_GETTERS)
		__try {
			return GetRawProperty(L, lifetime, (void*)object, prop);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			ERR("Exception while reading property %s.%s", Name.GetString(), prop.GetString());
			return PropertyOperationResult::Unknown;
		}
#else
		return GetRawProperty(L, lifetime, (void*)object, prop);
#endif
	}

	inline PropertyOperationResult SetProperty(lua_State* L, T* object, FixedString const& prop, int index) const
	{
#if defined(DEBUG_TRAP_GETTERS)
		__try {
			return SetRawProperty(L, lifetime, (void*)object, prop, index);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			ERR("Exception while writing property %s.%s", Name.GetString(), prop.GetString());
			return PropertyOperationResult::Unknown;
		}
#else
		return SetRawProperty(L, (void*)object, prop, index);
#endif
	}

	inline PropertyOperationResult GetProperty(lua_State* L, LifetimeHandle const& lifetime, T* object, RawPropertyAccessors const& prop) const
	{
		auto getter = (typename PropertyAccessors::Getter*)prop.Get;

#if defined(DEBUG_TRAP_GETTERS)
		__try {
			return getter(L, lifetime, object, prop.Offset, prop.Flag);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			ERR("Exception while reading property %s.%s", Name.GetString(), prop.Name.GetString());
			return PropertyOperationResult::Unknown;
		}
#else
		return getter(L, lifetime, object, prop.Offset, prop.Flag);
#endif
	}

	inline PropertyOperationResult SetProperty(lua_State* L, T* object, RawPropertyAccessors const& prop, int index) const
	{
		auto setter = (typename PropertyAccessors::Setter*)prop.Set;

#if defined(DEBUG_TRAP_GETTERS)
		__try {
			return setter(L, object, index, prop.Offset, prop.Flag);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			ERR("Exception while writing property %s.%s", Name.GetString(), prop.Name.GetString());
			return PropertyOperationResult::Unknown;
		}
#else
		return setter(L, object, index, prop.Offset, prop.Flag);
#endif
	}

	inline void AddProperty(char const* prop, 
		typename PropertyAccessors::Getter* getter,
		typename PropertyAccessors::Setter* setter, 
		typename PropertyAccessors::Validator* validator,
		typename PropertyAccessors::Serializer* serialize,
		typename PropertyAccessors::Unserializer* unserialize,
		std::size_t offset, uint64_t flag)
	{
		if (setter == nullptr) {
			setter = (typename PropertyAccessors::Setter*)&GenericSetNonWriteableProperty;
		}
		
		if (validator == nullptr) {
			validator = (typename PropertyAccessors::Validator*)&GenericValidateNoopProperty;
		}

		if (serialize == nullptr) {
			serialize = (typename PropertyAccessors::Serializer*)&GenericNullSerializeProperty;
		}

		if (unserialize == nullptr) {
			unserialize = (typename PropertyAccessors::Unserializer*)&GenericNullUnserializeProperty;
		}

		AddRawProperty(prop, 
			(RawPropertyAccessors::Getter*)getter, (RawPropertyAccessors::Setter*)setter, 
			(RawPropertyValidators::Validator*)validator, 
			(RawPropertyAccessors::Serializer*)serialize, (RawPropertyAccessors::Unserializer*)unserialize,
			offset, flag);
	}

	inline void SetFallback(PropertyAccessors::FallbackGetter* getter, PropertyAccessors::FallbackSetter* setter)
	{
		FallbackGetter = (TFallbackGetter*)getter;
		FallbackSetter = (TFallbackSetter*)setter;
	}

	inline void BindConstructors()
	{
		Size = sizeof(T);
		if constexpr (std::is_default_constructible_v<T>) {
			Construct = &(DefaultConstruct<T>);
			Destroy = &(DefaultDestroy<T>);
		}

		Serialize = &(DefaultSerialize<T>);
		Unserialize = &(DefaultUnserialize<T>);
	}
};

template <class T>
struct StaticLuaPropertyMap
{
	static_assert(!std::is_pointer_v<T>, "StaticLuaPropertyMap type should not be a pointer type!");
	static_assert(!IsByVal<T>, "StaticLuaPropertyMap type should not be a by-val type!");
	static_assert(!IsArrayLike<T>::Value && !IsSetLike<T>::Value && !IsMapLike<T>::Value, "StaticLuaPropertyMap type should not be a container type!");

	using ObjectType = T;
	using TPropertyMap = LuaPropertyMap<T>;

	static TPropertyMap PropertyMap;
};


template <class TK>
void SerializeArray(lua_State* L, Array<TK>* obj)
{
	StackCheck _(L, 1);
	lua_createtable(L, (int)obj->size(), 0);
	for (uint32_t i = 0; i < obj->size(); i++) {
		Serialize(L, &(*obj)[i]);
		lua_rawseti(L, -2, i);
	}
}

template <class TK>
void SerializeArray(lua_State* L, ObjectSet<TK>* obj)
{
	StackCheck _(L, 1);
	lua_createtable(L, (int)obj->size(), 0);
	for (uint32_t i = 0; i < obj->size(); i++) {
		Serialize(L, &(*obj)[i]);
		lua_rawseti(L, -2, i);
	}
}

template <class TK, size_t Size>
void SerializeArray(lua_State* L, std::array<TK, Size>* obj)
{
	StackCheck _(L, 1);
	lua_createtable(L, (int)obj->size(), 0);
	for (uint32_t i = 0; i < obj->size(); i++) {
		Serialize(L, &(*obj)[i]);
		lua_rawseti(L, -2, i);
	}
}

template <class TK, class TV>
void SerializeMap(lua_State* L, MultiHashMap<TK, TV>* obj)
{
	StackCheck _(L, 1);
	lua_createtable(L, 0, (int)obj->size());
	for (auto& kv : *obj) {
		Serialize(L, &kv.Key());
		Serialize(L, &kv.Value());
		lua_rawset(L, -2);
	}
}

template <class TK, class TV>
void SerializeMap(lua_State* L, RefMap<TK, TV>* obj)
{
	StackCheck _(L, 1);
	lua_createtable(L, 0, (int)obj->size());
	for (auto& kv : *obj) {
		Serialize(L, &kv.Key);
		Serialize(L, &kv.Value);
		lua_rawset(L, -2);
	}
}

template <class TK, class TV>
void SerializeMap(lua_State* L, Map<TK, TV>* obj)
{
	StackCheck _(L, 1);
	lua_createtable(L, 0, (int)obj->size());
	for (auto& kv : *obj) {
		Serialize(L, &kv.Key);
		Serialize(L, &kv.Value);
		lua_rawset(L, -2);
	}
}

template <class TK>
void SerializeSet(lua_State* L, MultiHashSet<TK>* obj)
{
	StackCheck _(L, 1);
	lua_createtable(L, (int)obj->size(), 0);
	for (uint32_t i = 0; i < obj->size(); i++) {
		Serialize(L, &obj->Keys[i]);
		lua_rawseti(L, -2, i);
	}
}

void SerializeRawObject(lua_State* L, void* obj, GenericPropertyMap const& pm);

template <class T>
inline void SerializeObject(lua_State* L, T* obj)
{
	SerializeRawObject(L, obj, StaticLuaPropertyMap<T>::PropertyMap);
}

template <class T>
inline void Serialize(lua_State* L, T* obj)
{
	static_assert(!std::is_pointer_v<T>);

	if constexpr (IsByVal<T>) {
		push(L, *obj);
	} else if constexpr (IsArrayLike<T>::Value) {
		SerializeArray(L, obj);
	} else if constexpr (IsMapLike<T>::Value) {
		SerializeMap(L, obj);
	} else if constexpr (IsSetLike<T>::Value) {
		SerializeSet(L, obj);
	} else {
		SerializeObject(L, obj);
	}
}

template <class T>
inline void Serialize(lua_State* L, OverrideableProperty<T>* obj)
{
	Serialize(L, &obj->Value);
}


template <class TK>
void UnserializeArray(lua_State* L, int index, Array<TK>* obj)
{
	StackCheck _(L);
	luaL_checktype(L, index, LUA_TTABLE);

	obj->clear();
	for (auto idx : iterate(L, index)) {
		TK value;
		Unserialize(L, idx, &value);
		// FIXME - in-place unserialize
		obj->push_back(value);
	}
}

template <class TK>
void UnserializeArray(lua_State* L, int index, ObjectSet<TK>* obj)
{
	StackCheck _(L);
	luaL_checktype(L, index, LUA_TTABLE);

	obj->clear();
	for (auto idx : iterate(L, index)) {
		TK value;
		Unserialize(L, idx, &value);
		// FIXME - in-place unserialize
		obj->push_back(value);
	}
}

template <class TK, size_t Size>
void UnserializeArray(lua_State* L, int index, std::array<TK, Size>* obj)
{
	StackCheck _(L);
	luaL_checktype(L, index, LUA_TTABLE);

	for (uint32_t i = 0; i < obj->size(); i++) {
		lua_rawgeti(L, index, i + 1);
		TK value;
		Unserialize(L, index + 1, &value);
		// FIXME - in-place unserialize
		(*obj)[i] = value;
		lua_pop(L, 1);
	}
}

template <class TK, class TV>
void UnserializeMap(lua_State* L, int index, MultiHashMap<TK, TV>* obj)
{
	StackCheck _(L);
	luaL_checktype(L, index, LUA_TTABLE);

	obj->clear();
	for (auto idx : iterate(L, index)) {
		auto key = get<TK>(L, idx);
		TV value;
		Unserialize(L, idx + 1, &value);
		// FIXME - in-place unserialize
		obj->Set(key, value);
	}
}

template <class TK, class TV>
void UnserializeMap(lua_State* L, int index, RefMap<TK, TV>* obj)
{
	StackCheck _(L);
	luaL_checktype(L, index, LUA_TTABLE);

	obj->clear();
	for (auto idx : iterate(L, index)) {
		auto key = get<TK>(L, idx);
		auto value = obj->get_or_insert(key);
		Unserialize(L, idx + 1, value);
	}
}

template <class TK, class TV>
void UnserializeMap(lua_State* L, int index, Map<TK, TV>* obj)
{
	StackCheck _(L);
	luaL_checktype(L, index, LUA_TTABLE);

	obj->clear();
	for (auto idx : iterate(L, index)) {
		auto key = get<TK>(L, idx);
		auto value = obj->get_or_insert(key);
		Unserialize(L, idx + 1, value);
	}
}

template <class TK>
void UnserializeSet(lua_State* L, int index, MultiHashSet<TK>* obj)
{
	StackCheck _(L);
	luaL_checktype(L, index, LUA_TTABLE);

	obj->clear();
	for (auto idx : iterate(L, index)) {
		obj->Add(get<TK>(L, idx));
	}
}

void UnserializeRawObject(lua_State* L, int index, void* obj, GenericPropertyMap const& pm);

template <class T>
inline void UnserializeObject(lua_State* L, int index, T* obj)
{
	UnserializeRawObject(L, index, obj, StaticLuaPropertyMap<T>::PropertyMap);
}

template <class T>
inline void Unserialize(lua_State* L, int index, T* obj)
{
	static_assert(!std::is_pointer_v<T>);

	if constexpr (IsByVal<T>) {
		*obj = get<T>(L, index);
	} else if constexpr (IsArrayLike<T>::Value) {
		UnserializeArray(L, lua_absindex(L, index), obj);
	} else if constexpr (IsMapLike<T>::Value) {
		UnserializeMap(L, lua_absindex(L, index), obj);
	} else if constexpr (IsSetLike<T>::Value) {
		UnserializeSet(L, lua_absindex(L, index), obj);
	} else {
		UnserializeObject(L, lua_absindex(L, index), obj);
	}
}

template <class T>
inline void Unserialize(lua_State* L, int index, OverrideableProperty<T>* obj)
{
	Unserialize(L, index, &obj->Value);
}

END_NS()

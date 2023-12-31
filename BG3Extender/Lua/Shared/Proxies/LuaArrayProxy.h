#pragma once

#include <Lua/LuaHelpers.h>
#include <Lua/LuaSerializers.h>
#include <Lua/Shared/LuaLifetime.h>

BEGIN_SE()

BY_VAL(uint8_t);
BY_VAL(int16_t);
BY_VAL(uint16_t);
BY_VAL(int32_t);
BY_VAL(uint32_t);
BY_VAL(int64_t);
BY_VAL(uint64_t);
BY_VAL(float);
BY_VAL(double);
BY_VAL(bool);
BY_VAL(ComponentHandle);
BY_VAL(EntityHandle);
BY_VAL(ecs::EntityRef);
BY_VAL(FixedString);
BY_VAL(STDString);
BY_VAL(STDWString);
BY_VAL(Guid);
BY_VAL(Path);
BY_VAL(NetId);
BY_VAL(UserId);
BY_VAL(Version);
BY_VAL(glm::ivec2);
BY_VAL(glm::vec2);
BY_VAL(glm::vec3);
BY_VAL(glm::vec4);
BY_VAL(glm::mat3);
BY_VAL(glm::mat3x4);
BY_VAL(glm::mat4x3);
BY_VAL(glm::mat4);
BY_VAL(TypeInformationRef);

template <class T> 
struct ByVal<OverrideableProperty<T>> { static constexpr bool Value = IsByVal<T>; };

END_SE()

namespace bg3se::lua
{
	LifetimeHandle GetCurrentLifetime();

	class ArrayProxyImplBase
	{
	public:
		inline virtual ~ArrayProxyImplBase() {};
		virtual char const* GetTypeName() const = 0;
		virtual void* GetRaw() = 0;
		virtual bool GetElement(lua_State* L, unsigned arrayIndex) = 0;
		virtual bool SetElement(lua_State* L, unsigned arrayIndex, int luaIndex) = 0;
		virtual int Next(lua_State* L, int key) = 0;
		virtual unsigned Length() = 0;
	};


	template <class TContainer, class T>
	class DynamicArrayProxyImpl : public ArrayProxyImplBase
	{
	public:
		static_assert(!std::is_pointer_v<T>, "DynamicArrayProxyImpl template parameter should not be a pointer type!");

		DynamicArrayProxyImpl(LifetimeHandle const& lifetime, TContainer* obj)
			: object_(obj), lifetime_(lifetime)
		{}
		
		~DynamicArrayProxyImpl() override
		{}

		T* Get() const
		{
			return object_;
		}

		void* GetRaw() override
		{
			return object_;
		}

		char const* GetTypeName() const override
		{
			return GetTypeInfo<T>().TypeName.GetString();
		}

		bool GetElement(lua_State* L, unsigned arrayIndex) override
		{
			if (arrayIndex > 0 && arrayIndex <= (int)object_->size()) {
				PushAny(L, &(*object_)[arrayIndex - 1], lifetime_);
				return true;
			} else {
				return false;
			}
		}

		bool SetElement(lua_State* L, unsigned arrayIndex, int luaIndex) override
		{
			if constexpr (IsByVal<T>) {
				if (arrayIndex > 0 && arrayIndex <= object_->size()) {
					(*object_)[arrayIndex - 1] = get<T>(L, luaIndex);
					return true;
				} else if (arrayIndex == object_->size() + 1) {
					object_->push_back(get<T>(L, luaIndex));
					return true;
				} else {
					return false;
				}
			} else {
				// Appending/swapping elements to by-ref arrays not supported for now
				return false;
			}
		}

		unsigned Length() override
		{
			return object_->size();
		}

		int Next(lua_State* L, int key) override
		{
			if (key >= 0 && key < (int)object_->size()) {
				push(L, ++key);
				PushAny(L, &(*object_)[key - 1], lifetime_);
				return 2;
			} else {
				return 0;
			}
		}

	private:
		TContainer* object_;
		LifetimeHandle lifetime_;
	};


	template <class TContainer, class T>
	class ConstSizeArrayProxyImpl : public ArrayProxyImplBase
	{
	public:
		static_assert(!std::is_pointer_v<T>, "ConstSizeArrayProxyImpl template parameter should not be a pointer type!");

		ConstSizeArrayProxyImpl(LifetimeHandle const& lifetime, TContainer* obj)
			: object_(obj), lifetime_(lifetime)
		{}
		
		~ConstSizeArrayProxyImpl() override
		{}

		T* Get() const
		{
			return object_;
		}

		void* GetRaw() override
		{
			return object_;
		}

		char const* GetTypeName() const override
		{
			return GetTypeInfo<T>().TypeName.GetString();
		}

		bool GetElement(lua_State* L, unsigned arrayIndex) override
		{
			if (arrayIndex > 0 && arrayIndex <= object_->size()) {
				PushAny(L, &(*object_)[arrayIndex - 1], lifetime_);
				return true;
			} else {
				return false;
			}
		}

		bool SetElement(lua_State* L, unsigned arrayIndex, int luaIndex) override
		{
			if constexpr (IsByVal<T>) {
				if (arrayIndex > 0 && arrayIndex <= object_->size()) {
					(*object_)[arrayIndex - 1] = get<T>(L, luaIndex);
					return true;
				} else {
					return false;
				}
			} else {
				// Appending/swapping elements to by-ref arrays not supported for now
				return false;
			}
		}

		unsigned Length() override
		{
			return (unsigned)object_->size();
		}

		int Next(lua_State* L, int key) override
		{
			if (key >= 0 && key < object_->size()) {
				push(L, ++key);
				PushAny(L, &(*object_)[key - 1], lifetime_);
				return 2;
			} else {
				return 0;
			}
		}

	private:
		TContainer* object_;
		LifetimeHandle lifetime_;
	};


	class ArrayProxy : private Userdata<ArrayProxy>, public Indexable, public NewIndexable,
		public Lengthable, public Iterable, public Stringifiable, public Pushable, public GarbageCollected
	{
	public:
		static char const * const MetatableName;

		template <class TImpl, class... Args>
		inline static TImpl* MakeImpl(lua_State* L, LifetimeHandle const& lifetime, Args... args)
		{
			auto self = NewWithExtraData(L, sizeof(TImpl), lifetime);
			return new (self->GetImpl()) TImpl(lifetime, args...);
		}

		template <class T>
		inline static DynamicArrayProxyImpl<Array<T>, T>* Make(lua_State* L, Array<T>* object, LifetimeHandle const& lifetime)
		{
			return MakeImpl<DynamicArrayProxyImpl<Array<T>, T>>(L, lifetime, object);
		}

		template <class T>
		inline static DynamicArrayProxyImpl<ObjectSet<T>, T>* Make(lua_State* L, ObjectSet<T>* object, LifetimeHandle const& lifetime)
		{
			return MakeImpl<DynamicArrayProxyImpl<ObjectSet<T>, T>>(L, lifetime, object);
		}

		template <class T, int Size>
		inline static ConstSizeArrayProxyImpl<std::array<T, Size>, T>* Make(lua_State* L, std::array<T, Size>* object, LifetimeHandle const& lifetime)
		{
			return MakeImpl<ConstSizeArrayProxyImpl<std::array<T, Size>, T>>(L, lifetime, object);
		}

		template <class T>
		inline static DynamicArrayProxyImpl<std::vector<T>, T>* Make(lua_State* L, std::vector<T>* object, LifetimeHandle const& lifetime)
		{
			return MakeImpl<DynamicArrayProxyImpl<std::vector<T>, T>>(L, lifetime, object);
		}

		inline ArrayProxyImplBase* GetImpl()
		{
			return reinterpret_cast<ArrayProxyImplBase*>(this + 1);
		}

		inline bool IsAlive(lua_State* L) const
		{
			return lifetime_.IsAlive(L);
		}

		template <class T>
		T* Get(lua_State* L)
		{
			if (!lifetime_.IsAlive(L)) {
				return nullptr;
			}

			if (strcmp(GetTypeInfo<T>().TypeName.GetString(), GetImpl()->GetTypeName()) == 0) {
				return reinterpret_cast<T*>(GetImpl()->GetRaw(L));
			} else {
				return nullptr;
			}
		}

	private:
		LifetimeHandle lifetime_;

		ArrayProxy(LifetimeHandle const& lifetime)
			: lifetime_(lifetime)
		{}

		~ArrayProxy()
		{
			GetImpl()->~ArrayProxyImplBase();
		}

	protected:
		friend Userdata<ArrayProxy>;

		int Index(lua_State* L);
		int NewIndex(lua_State* L);
		int Length(lua_State* L);
		int Next(lua_State* L);
		int ToString(lua_State* L);
		int GC(lua_State* L);
	};
}

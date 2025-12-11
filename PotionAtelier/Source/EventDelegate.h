#pragma once
#include <functional>
#include <vector>
#include <concepts>

/* 사용법
std::function의 강화버젼이라고 생각하면 쉬움.
클래스에 외부에서 함수를 등록
-> 등록한 함수를 원하는 타이밍에 호출할 수 있게 임시로 변수에 저장.
-> 원하는 타이밍에 변수를 통해 함수 Call

class A
{
	DECLARE_DELEGATE(Event, int)			// Type을 Event라는 이름으로 선언, 인수는 int.

	Event event;		// 위에서 선언한 내용을 변수로 등록

public:
	Event& GetEvent() { return event; }		// 바깥에서 참조할 수 있는 Get함수
};

int main()
{
	A a;

	// 함수 등록할 변수를 가져온뒤 -> 원하는 함수를 Bind
	a.GetEvent().Bind([](int a){ return a; });

	// 원하는 타이밍에 등록했던 함수를 인수와 함께 호출
	a.Execute(5);
};



Object::Awake() 에서 Component끼리 참조할 함수와 클래스를 등록
-> 등록한 함수를 원하는 타이밍에 호출할 수 있게 임시로 변수에 저장.
-> 원하는 타이밍에 변수를 통해 함수 Call

* 예시
class FooComponent
{
	DECLARE_EVENT(Event, FooComponent, int, float)		// Type을 Event라는 이름으로 선언, 주인은 FooComponent, 인수는 int와 float 2개를 받음.

	Event event;		// 위에서 선언한 내용을 변수로 등록

public:
	Event& GetEvent() { return event; }		// 바깥에서 참조할 수 있는 Get함수
};


class BarComponent
{
	void BarFunction(int a, float b);		// 등록할 함수는 바로 이녀석
};


class Object
{
public:
	void Awake()
	{
		auto& foo = &AddComponent<FooComponent>();
		auto& bar = &AddComponent<BarComponent>();

		foo.GetEvent().AddMember(&BarComponent::BarFunction, *bar);		// 등록할 함수포인터로 넣어주고, Instance 포인터도 함께 등록
	}
};

이제 FooComponent의 event에 함수가 등록이 완료.
원하는 때에 FooComponent내에서 event호출하면 끝.

event.BroadCast(10, 5.f);
event.BroadCast(3, 0.f);

*/

#define DECLARE_DELEGATE(type_name, ...) \
	using type_name = Event::EventDelegate<void(__VA_ARGS__)>;

#define DECLARE_DELEGATE_RETURNVALUE(type_name, return_value, ...) \
	using type_name = Event::EventDelegate<return_value(__VA_ARGS__)>;

#define DECLARE_MULTICAST_DELEGATE(type_name, ...) \
	using type_name = Event::MultiCastDelegate<__VA_ARGS__>; \

#define DECLARE_EVENT(type_name, owner, ...) \
	class type_name : public Event::MultiCastDelegate<__VA_ARGS__> \
	{ \
	private: \
		friend class owner; \
		using MultiCastDelegate::Broadcast; \
		using MultiCastDelegate::RemoveAll; \
		using MultiCastDelegate::Remove; \
	};


// ImGuiGizmo에서 Delegate라는 이름과 겹쳐서...
namespace Event
{
	template<typename...>
	class EventDelegate;

	template<typename ReturnType, typename... Args>
	class EventDelegate<ReturnType(Args...)>
	{
		using DelegateType = std::function<ReturnType(Args...)>;

	public:
		EventDelegate() = default;
		EventDelegate(EventDelegate const&) = default;
		EventDelegate(EventDelegate&&) noexcept = default;
		~EventDelegate() = default;
		EventDelegate& operator=(EventDelegate const&) = default;
		EventDelegate& operator=(EventDelegate&&) noexcept = default;

		template<typename F> requires std::is_constructible_v<DelegateType, F>
		void Bind(F&& callable)
		{
			callback = callable;
		}

		template<typename T>
		void BindMember(ReturnType(T::* mem_pfn)(Args...), T& instance)
		{
			callback = [&instance, mem_pfn](Args&&... args) mutable -> ReturnType {return (instance.*mem_pfn)(std::forward<Args>(args)...); };
		}

		void UnBind()
		{
			callback = nullptr;
		}

		ReturnType Execute(Args&&... args)
		{
			return callback(std::forward<Args>(args)...);
		}

		bool IsBound() const { return callback != nullptr; }

	private:
		DelegateType callback = nullptr;
	};

	class DelegateHandle
	{
	public:
		DelegateHandle() : id(INVALID_ID) {}
		explicit DelegateHandle(int) : id(GenerateID()) {}
		~DelegateHandle() noexcept = default;
		DelegateHandle(DelegateHandle const&) = default;
		DelegateHandle(DelegateHandle&& that) noexcept : id(that.id)
		{
			that.Reset();
		}
		DelegateHandle& operator=(DelegateHandle const&) = default;
		DelegateHandle& operator=(DelegateHandle&& that) noexcept
		{
			id = that.id;
			that.Reset();
			return *this;
		}

		operator bool() const
		{
			return IsValid();
		}

		bool operator==(DelegateHandle const& that) const
		{
			return id == that.id;
		}

		bool operator<(DelegateHandle const& that) const
		{
			return id < that.id;
		}

		bool IsValid() const
		{
			return id != INVALID_ID;
		}

		void Reset()
		{
			id = INVALID_ID;
		}

	private:
		size_t id;

	private:

		inline static constexpr size_t INVALID_ID = size_t(-1);
		static size_t GenerateID()
		{
			static size_t current_id = 0;
			return current_id++;
		}
	};

	template<typename... Args>
	class MultiCastDelegate
	{
		using DelegateType = std::function<void(Args...)>;
		using HandleDelegatePair = std::pair<DelegateHandle, DelegateType>;

	public:
		MultiCastDelegate() = default;
		MultiCastDelegate(MultiCastDelegate const&) = default;
		MultiCastDelegate(MultiCastDelegate&&) noexcept = default;
		~MultiCastDelegate() = default;
		MultiCastDelegate& operator=(MultiCastDelegate const&) = default;
		MultiCastDelegate& operator=(MultiCastDelegate&&) noexcept = default;

		template<typename F> requires std::is_constructible_v<DelegateType, F>
		[[maybe_unused]] DelegateHandle Add(F&& callable)
		{
			delegate_array.emplace_back(DelegateHandle(0), std::forward<F>(callable));
			return delegate_array.back().first;
		}

		template<typename T>
		[[maybe_unused]] DelegateHandle AddMember(void(T::* mem_pfn)(Args...), T& instance)
		{
			delegate_array.emplace_back(DelegateHandle(0), [&instance, mem_pfn](Args&&... args) mutable -> void {return (instance.*mem_pfn)(std::forward<Args>(args)...); });
			return delegate_array.back().first;
		}

		template<typename F> requires std::is_constructible_v<DelegateType, F>
		[[nodiscard]] DelegateHandle operator+=(F&& callable) noexcept
		{
			return Add(std::forward<F>(callable));
		}

		[[maybe_unused]] bool operator-=(DelegateHandle& handle)
		{
			return Remove(handle);
		}

		[[maybe_unused]] bool Remove(DelegateHandle& handle)
		{
			if (handle.IsValid())
			{
				for (size_t i = 0; i < delegate_array.size(); ++i)
				{
					if (delegate_array[i].Handle == handle)
					{
						std::swap(delegate_array[i], delegate_array.back());
						delegate_array.pop_back();
						handle.Reset();
						return true;
					}
				}
			}
			return false;
		}

		void RemoveAll()
		{
			delegate_array.clear();
		}

		void Broadcast(Args... args)
		{
			for (size_t i = 0; i < delegate_array.size(); ++i)
			{
				if (delegate_array[i].first.IsValid()) delegate_array[i].second(std::forward<Args>(args)...);
			}
		}

		bool IsHandleBound(DelegateHandle const& handle) const
		{
			if (handle.IsValid())
			{
				for (size_t i = 0; i < delegate_array.size(); ++i)
				{
					if (delegate_array[i].Handle == handle) return true;
				}
			}
			return false;
		}

	private:
		std::vector<HandleDelegatePair> delegate_array;

	};
}
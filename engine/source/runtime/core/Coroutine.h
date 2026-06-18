#pragma once

#include <coroutine>
#include <exception>
#include <optional>

namespace Lumen
{
	template <typename T = void>
	class Coroutine
	{
	public:
		struct promise_type;
		using HandleType = std::coroutine_handle<promise_type>;

		explicit Coroutine(HandleType h) : coro(h) {}
		~Coroutine() { coro.destroy(); }

		auto operator co_await() { return Awaiter{ coro }; }

		bool is_done() const { return coro.done(); }
		
		// finish interface
		struct promise_type
		{
			std::suspend_never initial_suspend() noexcept { return {}; } // after init, no suspend
			std::suspend_always final_suspend() noexcept { return {}; } // after finish, suspend
			Coroutine get_return_object() { return Coroutine{ HandleType::from_promise(*this) }; }
			void unhandled_exception() { std::terminate(); }
			void return_value(T val) noexcept { value = std::move(val); }
			void return_void() noexcept { }
			// yield_value()

			std::optional<T> value;
		};

	private:
		HandleType coro;

		struct Awaiter
		{
			HandleType coro;

			bool await_ready() const noexcept { return coro.done(); }
			void await_suspend(std::coroutine_handle<> h) const noexcept { coro.resume(); }
			T await_resume() const { return coro.promise().value; } 
		};
	};

	template <>
	class Coroutine<void>
	{
	public:
		struct promise_type;
		using HandleType = std::coroutine_handle<promise_type>;

		explicit Coroutine(HandleType h) : coro(h) {}
		~Coroutine() { if (coro) coro.destroy(); }

		auto operator co_await() { return Awaiter{ coro }; }

		bool is_done() const { return coro.done(); }

		struct promise_type
		{
			std::suspend_never initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }
			Coroutine get_return_object() { return Coroutine{ HandleType::from_promise(*this) }; }
			void unhandled_exception() { std::terminate(); }
			void return_void() noexcept {}
		};

	private:
		HandleType coro;

		struct Awaiter
		{
			HandleType coro;

			bool await_ready() const noexcept { return coro.done(); }
			void await_suspend(std::coroutine_handle<> h) const noexcept { coro.resume(); } 
			void await_resume() const {}
		};
	};
}
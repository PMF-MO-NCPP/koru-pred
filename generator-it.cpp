#include <iostream>
#include <coroutine>
#include <chrono>
#include <cassert>
#include <fmt/format.h>
#include <thread>

template <typename T>
struct [[nodiscard]] Generator{
    struct promise_type{
        T current_value;
        auto get_return_object(){ return Generator(std::coroutine_handle<promise_type>::from_promise(*this)); }
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        void return_void() { }
        void unhandled_exception(){ std::terminate(); }
        auto yield_value(T val){
            current_value = val;
            return std::suspend_always{};
        }
    };
    struct iterator{
        std::coroutine_handle<promise_type> handle;
        iterator(auto h) : handle{h} {}
        void next(){  // Pomoćna rutina koju koristimo u ++ i begin()
            if(handle)
                handle.resume();
            if(handle.done())
                handle = nullptr;
        }
        iterator & operator++(){
            next();
            return *this;
        }
        T operator*() const{
            assert(handle != nullptr);
            return handle.promise().current_value;
        }
        bool operator==(iterator const &) const = default;
    };

    Generator(auto handler) : mHandler{handler} {}
    ~Generator(){
        if(mHandler)
        mHandler.destroy();
    }
    Generator(Generator const &) = delete;
    Generator & operator=(Generator const &) = delete;

    bool next(){
        if(!mHandler || mHandler.done())
            return false;
        mHandler.resume();
        return !mHandler.done(); 
    }
    T getValue(){ return mHandler.promise().current_value; }
	
    iterator begin() const {
        if(!mHandler || mHandler.done())
          return iterator{nullptr};
        iterator it{mHandler};
        it.next(); 
        return it;
    }
    iterator end() const {
        return iterator{nullptr};
    }

    std::coroutine_handle<promise_type> mHandler;
};

template <typename T>
Generator<T> generator(T init, T step){
    for(T x = init; true; x += step){
        co_yield x;
    }
}

int main(){
	using namespace std::literals;
    auto gen = generator(0.0, 2.3);
    for(auto x : gen){
		fmt::print("x = {}\n", x);
		std::this_thread::sleep_for(1s);
		if(x > 20) break;
    }
    return 0;
}

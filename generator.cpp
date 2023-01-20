#include <iostream>
#include <coroutine>


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
    std::coroutine_handle<promise_type> mHandler;
};

template <typename T>
Generator<T> generator(T init, T step){
    for(T x = init; true; x += step){
        co_yield x;
    }
}

int main(){
    auto gen = generator(0.0, 2.3);
    for(int i=0; i<10; ++i){
          gen.next();
          std::cout << i+1 << ". " << gen.getValue() << "\n";
    }
    return 0;
}

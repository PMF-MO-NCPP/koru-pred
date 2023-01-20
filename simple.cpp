#include "fmt/format.h"
#include <coroutine>
#include <thread>
#include <exception> // za terminate()
#include <iostream>

struct Promise{
    auto get_return_object(){
        return std::coroutine_handle<Promise>::from_promise(*this);
    }
    auto initial_suspend(){
        return std::suspend_always{};
    }
    auto final_suspend() noexcept {
        return std::suspend_always{};

    }
    void unhandled_exception(){
        std::terminate();
    }
    void return_void(){
    }
};

class Task{
public:
    using promise_type = Promise;
    Task(auto h) : cHandle{h} {
    }
    ~Task() {
         if(cHandle)
            cHandle.destroy();
    }
    Task(Task const &) = delete;
    Task & operator=(Task const &) = delete;
    bool resume() const {
        if(!cHandle)
            return false;
        cHandle.resume();
        return !cHandle.done();
    }
private:
    std::coroutine_handle<promise_type> cHandle;
};


Task say_hello(){
    fmt::print("korutina:  Hello\n");
    co_await std::suspend_always(); // opoziv rutine, čekaj do nastavka
    fmt::print("korutina:  World\n");
    co_await std::suspend_always();  // opoziv rutine, čekaj do nastavka
    fmt::print("korutina:  !!!\n");
}


int main(){
    Task sh_task = say_hello();
    fmt::print("main:      Korutina say_hello je startana. Unesi znak za nastavak:");

    std::string c;
    std::getline(std::cin, c);
    sh_task.resume();
    std::cout << "main:      Znak za nastavak: "; std::getline(std::cin, c);
    sh_task.resume();
    std::cout << "main:      Znak za nastavak: "; std::getline(std::cin, c);
    sh_task.resume();

    return 0;
}

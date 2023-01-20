#include <coroutine>
#include <fmt/format.h>

struct Promise{
    auto get_return_object(){
        //fmt::print("- promise : get_return_type()\n");
        return std::coroutine_handle<Promise>::from_promise(*this);
    }
    auto initial_suspend(){
        //fmt::print("- promise : initial_suspend()\n");
        return std::suspend_always{};
    }
    auto final_suspend() noexcept {
        //fmt::print("- promise : final_suspend()\n");
        return std::suspend_always{};

    }
    void unhandled_exception(){
        //fmt::print("- promise : unhandeled_exception()\n");
        std::terminate();
    }
    void return_void(){
         //fmt::print("- promise : return_void()\n");
    }
};


class [[nodiscard]] Task{
public:
    using promise_type = Promise;
    Task(auto h) : cHandle{h} {
         //fmt::print("- Task - konstruktor\n");
    }
    ~Task() {
         //fmt::print("- Task - destruktor\n");
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


Task coro()
{
    fmt::print("- coro(): PRVI DIO\n");
    co_await std::suspend_always{};
    fmt::print("- coro(): DRUGI DIO\n");
}


Task callCoro()
{
     fmt::print("- callCoro(): CALL coro()\n");
     auto ctask = coro(); // CALL sub-coroutine
	 while(ctask.resume())
		 fmt::print("     coro opozvan.\n");
     fmt::print("- callCoro(): coro() gotov\n");
     co_await std::suspend_always{}; // SUSPEND
     fmt::print("- callCoro(): gotov\n");
}

int main(){
    auto coroTask = callCoro();
	fmt::print("- MAIN: callCoro() inicijaliziran\n");
    while (coroTask.resume()) {
	   fmt::print("- MAIN: callCoro() opozvan\n");
    }
	fmt::print("- MAIN: callCoro() gotov\n");
	return 0;
}

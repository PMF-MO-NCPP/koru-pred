#include <iostream>
#include <coroutine>
#include <fmt/format.h>

template <typename Hndl>
struct BackAwaiter{
    Hndl hndl = nullptr;
    BackAwaiter() = default;
    bool await_ready() const{ return false; }
    void await_suspend(Hndl h) noexcept {hndl = h;}
    auto await_resume() noexcept {return hndl.promise().input_value;}
};

class [[nodiscard]] Generator
{
	public:
		struct promise_type;
		using Handle = std::coroutine_handle<promise_type>;
	private:
		Handle mHandle;
	public:
		struct promise_type{
		    double output_value;
		    double input_value;

		    auto get_return_object(){
		       return Handle::from_promise(*this);
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
		    void return_void(){ }

                    auto yield_value(double value){
                       output_value = value;
                       return BackAwaiter<Handle>{};
                    }
	      };

       Generator(Handle h) : mHandle{h} { }
       ~Generator() {
                if(mHandle)
                    mHandle.destroy();
            }
       Generator(Generator const &) = delete;
       Generator & operator=(Generator const &) = delete;
   
       bool next() const {
          if(!mHandle || mHandle.done())
              return false;
          mHandle.resume();
          return !mHandle.done();
       }

       double value() const { return mHandle.promise().output_value; }

       void setMultiplier(double mult = 1.0) { mHandle.promise().input_value = mult; }
};


// korutina
Generator generator(int max = 10)
{ 
	fmt::print("generator {} start\n", max);
	double c = 1.0;
	for(int i=0; i<max; ++i){
	    fmt::print("generator {}/{} yield\n", i, max);
	    double new_c = co_yield i*c;
	    fmt::print("generator new_c = {}\n", new_c);
            c = new_c;
	}
}

int main(){
    Generator gen = generator(10);
    int i=0;
    while(gen.next()){
        fmt::print("main {}\n", gen.value());
        double mult = ++i;
        gen.setMultiplier(mult);
    }
    return 0;
}

#include <atomic>
#include <chrono>
#include <coroutine>
#include <thread>
#include <iostream>
#include <cstdlib> // za std::rand()
#include <syncstream>

// Lewis Baker: C++ Coroutines: Understanding operator co_await

class Event{
public:
    Event(bool initiallySet = false) noexcept : mState{initiallySet ? this : nullptr} {}
	Event(Event const & ) = delete;
	Event& operator=(Event const&) = delete;
	bool isSet() const noexcept{ 
		//return mState.load(std::memory_order_acquire) == this; 
		return mState.load() == this; 
	}
	struct Awaiter;
	Awaiter operator co_await() const noexcept;
	void set() noexcept;
	void reset() noexcept{
		void * oldState = this; 
		//mState.compare_exchange_strong(oldState, nullptr, std::memory_order_acquire);
		mState.compare_exchange_strong(oldState, nullptr);
	}
private:
	friend Awaiter;
	// Ako je mState == this onda je to postavljeno stanje (isSet() daje true).
	// Inače stanje nije postavljeno i mState glava povezane liste čekatelja. 
	mutable std::atomic<void*> mState;  // Povezana lista čekatelja.
};

struct Event::Awaiter
{
  friend Event;
  // Awaiter mora znati na koji događaj čeka.
  Awaiter(const Event& event) noexcept : mEvent(event) {}

  bool await_ready() const noexcept{ return mEvent.isSet(); }
  bool await_suspend(std::coroutine_handle<> awaitingCoroutine) noexcept;
  void await_resume() noexcept {}

private:
  const Event& mEvent;   // Događaj na koji čekamo.
  std::coroutine_handle<> mAwaitingCoroutine;  // Korutina koja čeka. 
  Awaiter* mNext; // Sljedeći element u listi.
};


Event::Awaiter Event::operator co_await() const noexcept{ 
	return Awaiter(*this); 
}

void Event::set() noexcept{
		// Postavi this u mState (OK stanje)
       //void * oldValue = mState.exchange(this, std::memory_order_acq_rel);
       void * oldValue = mState.exchange(this);
	   // oldValue je prethodno stanje
	   if(oldValue != this){
		   // postoji povezana lista čekatelja na event.
		   auto waiters = static_cast<Awaiter*>(oldValue); // to je pravi tip pokazivača
		   while(waiters != nullptr){
			   Awaiter * next = waiters->mNext;
               waiters->mAwaitingCoroutine.resume();
			   waiters=next;
		   }
	   }
}

bool Event::Awaiter::await_suspend(
		    std::coroutine_handle<> awaitingCoroutine) noexcept
{
	const void * const setState = &mEvent; 
	mAwaitingCoroutine = awaitingCoroutine;

	// Try to atomically push this awaiter onto the front of the list.
	//void * oldValue = mEvent.mState.load(std::memory_order_acquire);
	void * oldValue = mEvent.mState.load();
	do{
		if(oldValue == setState) // Ovime mEvent signalizira da je stanje postavljeno 
			return false;        // i stoga ne idi u suspenziju. 
        mNext = static_cast<Awaiter*>(oldValue);
	}
/*	while(!mEvent.mState.compare_exchange_weak(oldValue, this, 
				                              std::memory_order_release,
				                              std::memory_order_acquire
											  )); */
	while( !mEvent.mState.compare_exchange_weak(oldValue, this) );
	return true;
}

struct Task {
	struct promise_type
	{
		Task get_return_object() noexcept { return {}; }
		std::suspend_never initial_suspend() noexcept{ return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		void return_void() noexcept {}
		void unhandled_exception() noexcept {}
	};
};


int f(){ 
	using namespace std::literals;
	std::this_thread::sleep_for(1s); 
	return std::rand() % 1000; 
}

int value;
Event event;

void producer(){
	std::osyncstream{std::cout} << "producer thread = " << std::this_thread::get_id() <<"\n";
	value = f();
	event.set();
}

Task consumer(){
	std::osyncstream{std::cout} << "consumer thread = " << std::this_thread::get_id() <<"\n";
	co_await event;
	std::cout << "value = " << value << ", thread no = " 
		      << std::this_thread::get_id() <<"\n";
}

int main(){
	std::osyncstream{std::cout} << "main thread = " << std::this_thread::get_id() <<"\n";
    //std::cout << "No threads = " << std::jthread::hardware_concurrency() << "\n";
	std::jthread c1(consumer);
	std::jthread c2(consumer);
	std::jthread c3(consumer);
	producer();
	std::cout << "reset\n";
	event.reset();
    std::jthread c4(consumer);
	std::jthread c5(consumer);
	std::jthread c6(consumer);
	std::jthread p2(producer);

	return 0;
}

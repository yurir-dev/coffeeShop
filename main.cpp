#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

void coffeePlace();

int main(char* /*argv*/, int /*argc*/)
{
  coffeePlace();
  return 0;
}


struct CoffeeCap
{
  void operator << (size_t v)
  {
    _val = v;
    if (!empty())
      std::cout << "coffee is ready" << std::endl;
  }
  bool empty() const{return _val == 0;}

  size_t _val{0};
};

struct Consume
{
  void operator << (CoffeeCap& cap)
  {
    std::cout << "nice " << std::hex << cap._val << std::endl;
    cap << 0;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
};

void coffeePlace()
{
	std::atomic<bool> alive{true};
	std::mutex mtx; std::condition_variable cv;
	CoffeeCap cap;

	std::thread worker{[&alive, &mtx, &cv, &cap](){
		Consume consume;
	  while (alive)
	  {
		  std::unique_lock lk{mtx};
		  cv.wait(lk, [&alive, &cap](){return !alive || !cap.empty();});
		  consume << cap;
		  cv.notify_one();
	  } }};

  std::thread barista{[&alive, &mtx, &cv, &cap](){
    while (alive)
	  {
	    std::unique_lock lk{mtx};
	    cv.wait(lk, [&alive, &cap](){return !alive || cap.empty();});
	    cap << 0xC0FFEE;
	    cv.notify_one();
		} }};

  [[maybe_unused]] int ch{ getchar() };
  alive = false;
  worker.join(); barista.join();
}
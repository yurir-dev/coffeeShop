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


struct CoffeeCup
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
  void operator << (CoffeeCup& cup)
  {
    if (!cup.empty())
    {
      std::cout << "nice " << std::hex << cup._val << std::endl;
      cup << 0;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    else
      std::cout << "cmon man, not cool" << std::endl;
  }
};

void coffeePlace()
{
  std::atomic<bool> alive{true};
  std::mutex mtx; std::condition_variable cv;
  CoffeeCup cup;
  
  std::thread worker{[&alive, &mtx, &cv, &cup](){
    Consume consume;
    while (alive)
    {
      std::unique_lock lk{mtx};
      cv.wait(lk, [&alive, &cup](){return !alive || !cup.empty();});
      consume << cup;
      cv.notify_one();
    } }};

  std::thread barista{[&alive, &mtx, &cv, &cup](){
    while (alive)
    {
      std::unique_lock lk{mtx};
      cup << 0xC0FFEE;
      cv.notify_one();
      cv.wait(lk, [&alive, &cup](){return !alive || cup.empty();});
    } }};

  [[maybe_unused]] int ch{ getchar() };
  alive = false;
  worker.join(); barista.join();
}
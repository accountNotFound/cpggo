#include <chrono>
#include <string>

#include "context/context.h"
#include "stdio.h"
#include "synchronize/channel.h"
#include "synchronize/mutex.h"

using namespace cppgo;

constexpr int thread_num = 8;
constexpr int foo_bar_num = 3000;
constexpr int loop_num = 10;
const char* data = "abc";

Context ctx(thread_num);

Mutex mtx(&ctx);

int value = 0;

AsyncFunction<void> bar(Channel<std::string, 1>& inchan,
                        Channel<int, 1>& outchan) {
  std::string s = co_await inchan.recv();
  co_await outchan.send(s.size());
}

AsyncFunction<void> foo() {
  Channel<std::string, 1> inchan(&ctx);
  Channel<int, 1> outchan(&ctx);
  for (int i = 0; i < loop_num; i++) {
    ctx.spawn(bar(inchan, outchan));
    co_await inchan.send(std::string(data));
    int v = co_await outchan.recv();
    co_await mtx.lock();
    value += v;
    mtx.unlock();
  }
}

int main() {
  for (int i = 0; i < foo_bar_num; i++) {
    ctx.spawn(foo());
  }
  while (!ctx.idle() &&
         value < foo_bar_num * loop_num * std::string(data).size()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  printf("value=%d\n", value);
  printf("target=%d\n", foo_bar_num * loop_num * std::string(data).size());
  ctx.stop();
  if (value != foo_bar_num * loop_num * std::string(data).size()) {
    return -1;
  }
  return 0;
}
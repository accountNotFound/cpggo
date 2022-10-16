#include <stdio.h>

#include <chrono>
#include <string>

#include "runtime/context.h"
#include "synchronize/channel.h"
#include "synchronize/mutex.h"

using namespace cppgo;

constexpr int thread_num = 10;
constexpr int foo_bar_num = 3000;
constexpr int loop_num = 100;
const char* data = "abc";

Context ctx(thread_num);

Mutex mtx(ctx);

int value = 0;

AsyncFunction<void> bar(Channel<std::string>& inchan, Channel<int>& outchan) {
  std::string s = co_await inchan.recv();
  co_await outchan.send(s.size());
}

AsyncFunction<void> foo() {
  Channel<std::string> inchan(ctx, 1);
  Channel<int> outchan(ctx, 1);
  for (int i = 0; i < loop_num; i++) {
    ctx.go(bar(inchan, outchan));
    co_await inchan.send(std::string(data));
    int v = co_await outchan.recv();
    co_await mtx.lock();
    value += v;
    mtx.unlock();
  }
}

int main() {
  ctx.start();
  for (int i = 0; i < foo_bar_num; i++) {
    ctx.go(foo());
  }
  ctx.wait_until([]() { return value >= foo_bar_num * loop_num * std::string(data).size(); });
  ctx.stop();
  printf("value=%d\n", value);
  if (value != foo_bar_num * loop_num * std::string(data).size()) {
    return -1;
  }
  return 0;
}
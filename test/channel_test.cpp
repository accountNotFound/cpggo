#include "synchronize/channel.h"

#include <stdio.h>

#include <chrono>
#include <string>

#include "context/context.h"
#include "synchronize/mutex.h"

using namespace cppgo;

constexpr int thread_num = 8;
constexpr int foo_bar_num = 3000;
constexpr int loop_num = 10;
const char* data = "abc";

Context ctx(thread_num);

Channel<std::string> chan(&ctx, 3);

Mutex mtx(&ctx);

int value = 0;

AsyncFunction<void> foo() {
  printf("test foo start\n");
  for (int i = 0; i < loop_num; i++) {
    // printf("foo start send\n");
    co_await chan.send(std::string(data));
    // printf("foo send '%s'\n", data);
  }
  printf("test foo end\n");
}

AsyncFunction<void> bar() {
  printf("test bar start\n");
  while (true) {
    // printf("bar start recv\n");
    std::string s = co_await chan.recv();
    // printf("bar receive '%s'\n", s.data());
    co_await mtx.lock();
    value += s.size();
    mtx.unlock();
  }
  co_await mtx.lock();
  printf("test bar end, value=%d\n", value);
  mtx.unlock();
}

int main() {
  for (int i = 0; i < foo_bar_num; i++) {
    ctx.spawn(foo());
    ctx.spawn(bar());
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
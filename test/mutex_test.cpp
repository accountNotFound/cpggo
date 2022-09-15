#include "synchronize/mutex.h"

#include <chrono>
#include <string>

#include "context/context.h"
#include "stdio.h"

using namespace cppgo;

int thread_num = 8;
int coroutine_num = 3000;
int loop_num = 100;

Context ctx(thread_num);

Mutex mtx(&ctx);

int value = 0;

AsyncFunction<void> foo() {
  printf("test function start\n");
  for (int i = 0; i < loop_num; i++) {
    co_await mtx.lock();
    value++;
    // printf("value=%d\n", value);
    mtx.unlock();
  }
  co_await mtx.lock();
  printf("final value=%d\n", value);
  mtx.unlock();
  printf("test function end\n");
}

int main() {
  for (int i = 0; i < coroutine_num; i++) {
    ctx.spawn(foo());
  }
  while (!ctx.idle()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  ctx.stop();
  printf("value=%d\n", value);
  if (value != coroutine_num * loop_num) {
    return -1;
  }
  return 0;
}
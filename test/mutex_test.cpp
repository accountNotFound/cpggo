#include <stdio.h>

#include <functional>

#include "async/functional.h"
#include "core/runtime.h"
#include "core/synchronize.h"

using namespace cppgo;

int thread_num = 10;
int coroutine_num = 3000;
int loop_num = 100;

Context ctx(thread_num);

Mutex mtx(ctx);

int value = 0;

AsyncFunction<void> foo() {
  printf("test function start\n");
  for (int i = 0; i < loop_num; i++) {
    co_await mtx.lock();
    value++;
    mtx.unlock();
  }
  co_await mtx.lock();
  printf("final value=%d\n", value);
  mtx.unlock();
  printf("test function end\n");
}

int main() {
  for (int i = 0; i < coroutine_num; i++) {
    ctx.go(foo());
  }
  ctx.wait_until([]() { return value >= coroutine_num * loop_num; });
  printf("value=%d\n", value);
  if (value != coroutine_num * loop_num) {
    return -1;
  }
  return 0;
}
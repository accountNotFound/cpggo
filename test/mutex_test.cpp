#include "synchronize/mutex.h"

#include <stdio.h>

#include <functional>

#include "async/functional.h"
#include "runtime/context.h"

// #define USE_DEBUG
#include "util/log.h"

using namespace cppgo;

int thread_num = 10;
int coroutine_num = 3000;
int loop_num = 100;

Context ctx(thread_num);

Mutex mtx(ctx);

int value = 0;

AsyncFunction<void> foo() {
  DEBUG("test function start", 0);
  for (int i = 0; i < loop_num; i++) {
    co_await mtx.lock();
    DEBUG("routine {%d} add value", ctx.current_goroutine().id());
    value++;
    mtx.unlock();
  }
  co_await mtx.lock();
  DEBUG("final value=%d", value);
  mtx.unlock();
  DEBUG("test function end", 0);
}

int main() {
  ctx.start();
  for (int i = 0; i < coroutine_num; i++) {
    ctx.go(foo());
  }
  ctx.wait_until([]() { return value >= coroutine_num * loop_num; });
  ctx.stop();
  printf("value=%d\n", value);
  if (value != coroutine_num * loop_num) {
    return -1;
  }
  return 0;
}
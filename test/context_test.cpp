#include "context/context.h"

#include <string>
#include <chrono>

#include "stdio.h"

using namespace cppgo;

int thread_num = 8;
int coroutine_num = 3000;
int loop_num = 100;

Context ctx(thread_num);

Monitor monitor(&ctx);

int value = 0;

AsyncFunction<void> worker() {
  printf("test function start\n");
  for (int i = 0; i < loop_num; i++) {
    co_await monitor.enter();
    value++;
    // printf("value=%d\n", value);
    co_await monitor.exit();
  }
  co_await monitor.enter();
  printf("final value=%d\n", value);
  co_await monitor.exit();
  printf("test function end\n");
}

int main() {
  for (int i = 0; i < coroutine_num; i++) {
    ctx.spawn(worker());
  }
  while(!ctx.idle()){
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  ctx.stop();
  printf("value=%d\n", value);
  if (value != coroutine_num * loop_num) {
    return -1;
  }
  return 0;
}
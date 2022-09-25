#include <stdio.h>

#include <chrono>
#include <thread>

#include "event/time_event_context.h"

using namespace cppgo;

constexpr size_t thread_num = 8;
constexpr size_t loop_num = 10;

time::TimeEventContext ctx(thread_num);

AsyncFunction<void> sleep(unsigned long long millisecond) {
  // printf("sleep(%u)\n", millisecond);
  auto fd = time::TimeFd(millisecond);
  ctx.add(fd, Event::IN);
  // printf("start await signal of fd {%u}\n", size_t(fd));
  co_await ctx.signal(fd);
}

AsyncFunction<void> foo(size_t id, unsigned long long millisecond) {
  printf("foo {%d} start\n", id);
  unsigned long long start_ts = 0;
  for (int i = 0; i < loop_num; i++) {
    co_await sleep(millisecond);
    start_ts += millisecond;
    printf("foo {%d} time: {%u}\n", id, start_ts);
  }
  printf("foo {%d} end\n", id);
}

int main() {
  ctx.spawn(foo(0, 800));
  ctx.spawn(foo(1, 200));
  ctx.spawn(foo(2, 500));
  ctx.spawn(foo(3, 1000));
  ctx.spawn(foo(4, 2000));
  ctx.spawn(foo(5, 1500));
  auto p_ctx = &ctx;
  auto t = std::thread([p_ctx]() {
    while (!p_ctx->idle()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    p_ctx->stop();
  });
  ctx.evloop();
  if (t.joinable()) {
    t.join();
  }
  return 0;
}
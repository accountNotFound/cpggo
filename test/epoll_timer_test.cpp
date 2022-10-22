#include <chrono>

#include "event/handler.h"
#include "event/time.h"
#include "runtime/context.h"
#include "runtime/goroutine.h"
#include "synchronize/mutex.h"

// #define USE_DEBUG
#include "util/log.h"

using namespace cppgo;

constexpr int thread_num = 10;
constexpr int foo_num = 3000;
constexpr int loop_num = 100;

constexpr long sleep_millisec = 50;

Context ctx(thread_num);

Mutex mtx(ctx);

int value = 0;

AsyncFunction<void> foo() {
  long ts = 0;
  DEBUG("foo {%d} start", ctx.current_goroutine().id());
  for (int i = 0; i < loop_num; ++i) {
    co_await cppgo::sleep(ctx, sleep_millisec + i);
    ts += sleep_millisec + i;
    DEBUG("foo {%d} at {%u} millisec", ctx.current_goroutine().id(), ts);
  }
  DEBUG("foo {%d} end", ctx.current_goroutine().id());
  co_await mtx.lock();
  ++value;
  DEBUG("value=%d", value);
  mtx.unlock();
}

std::chrono::milliseconds current_millisec() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}

int main() {
  ctx.add(EventHandler(ctx));
  ctx.start();
  auto begin_ms = current_millisec();
  for (int i = 0; i < foo_num; ++i) {
    ctx.go(foo());
  }
  ctx.get<EventHandler>().loop_until([]() { return value >= foo_num; });
  ctx.stop();
  auto end_ms = current_millisec();
  size_t theory_ms_cost = (sleep_millisec + sleep_millisec + loop_num) * loop_num / 2;
  printf("theory time cost: %u\n", theory_ms_cost);
  printf("real time cost: %u\n", (end_ms - begin_ms).count());
  size_t tolerance_ms = 1000;
  if (end_ms - begin_ms > std::chrono::milliseconds(theory_ms_cost + tolerance_ms) ||
      end_ms - begin_ms < std::chrono::microseconds(theory_ms_cost)) {
    return -1;
  }
  return 0;
}
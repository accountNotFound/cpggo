#pragma once

#include "../context.h"
#include "../executor.h"
#include "../goroutine.h"
#include "util/lock_free_hashset.h"
#include "util/lock_free_queue.h"

namespace cppgo {

class Context::Impl {
 public:
  Impl(Context& _this_wrapper, size_t executor_num);

 public:
  Goroutine& go(AsyncFunctionBase&& fn);
  Goroutine& current_goroutine() { return *this_thread_goroutine; }
  void start();
  void wait_until(const std::function<bool()>& pred);
  void stop();

 public:
  static thread_local Goroutine* this_thread_goroutine;

 public:
  LockFreeHashset<Executor> executors;
  LockFreeHashset<Goroutine> goroutines;
  LockFreeQueue<Goroutine*> runnable_queue;

 private:
  Context* _this_wrapper;
  size_t _executor_num;
};

}  // namespace cppgo

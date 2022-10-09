#pragma once

#include <set>

#include "runtime.h"
#include "util/lock_free_queue.h"
#include "util/spin_lock.h"

namespace cppgo {

class Context::Impl {
  friend class Executor::Impl;
  friend class Goroutine::Impl;

 public:
  Impl(Context& _this_wrapper, size_t executor_num);

 public:
  Goroutine& go(AsyncFunctionBase&& fn);
  Goroutine& current_goroutine();
  void start();
  void wait_until(const std::function<bool()>& pred);

 private:
  Context* _this() { return _this_wrapper; }

 private:
  static thread_local Goroutine* _this_thread_goroutine;

 private:
  struct Less {
    bool operator()(const Executor& lhs, const Executor& rhs) const { return lhs.id() < rhs.id(); }
    bool operator()(const Goroutine& lhs, const Goroutine& rhs) const { return lhs.id() < rhs.id(); }
  };

 private:
  Context* _this_wrapper;
  size_t _executor_num;
  SpinLock _mtx;
  std::set<Executor, Less> _executors;
  std::set<Goroutine, Less> _goroutines;
  LockFreeQueue<Goroutine*> _runnable_queue;
  LockFreeQueue<Goroutine*> _done_queue;
};

}  // namespace cppgo

#pragma once

#include <functional>
#include <queue>
#include <unordered_set>

#include "common.h"
#include "common/spinlock.h"
#include "coroutine/coroutine.h"

namespace cppgo {

class Monitor {
  friend class Context;
  friend class Worker;
  friend class Task;

 public:
  using Id = size_t;

 public:
  Monitor(Context* ctx);
  Monitor(Monitor&) = delete;

  Id id() { return id_; }

  AsyncFunction<void> enter();
  // lock automatically when co_return
  AsyncFunction<void> wait();
  // won't lock automatically when co_return
  AsyncFunction<void> block();
  void notify_one();
  void exit();

 private:
  // call in Task's callback. should be protected by monitor's lock
  void notify_one_with_lock_();

 private:
  // just for unique id generation
  static SpinLock cls_;
  static Id cls_id_;

  Id id_;

  Context* ctx_;
  std::unordered_set<Task*> blocked_set_;  // this set is protected by
                                           // context's mutex in callback

  SpinLock self_;  // protect following members
  bool busy_flag_ = false;
};

}  // namespace  cppgo

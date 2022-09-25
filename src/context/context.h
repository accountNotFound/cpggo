#pragma once

#include <unordered_map>
#include <unordered_set>

#include "header.h"
#include "common/allocator.h"
#include "common/spinlock.h"
#include "common/time_order_set.h"
#include "coroutine/coroutine.h"

namespace cppgo {

class Context {
  friend class Task;
  friend class Worker;
  friend class Monitor;

 public:
  Context(size_t worker_num);

  bool done() { return done_; }

  void spawn(AsyncFunction<void>&& func);
  bool idle();
  void stop();
  Task* this_running_task();

 protected:
  SpinLock self_;

  // resource managers
  Allocator<Worker> worker_mgr_;
  Allocator<Task> task_mgr_;

  // just reference to resource above
  TimeOrderSet<Task*> runnable_set_;
  TimeOrderSet<Task*> blocked_set_;
  std::unordered_map<Task*, Worker*> running_map_;

  bool done_ = false;
};

}  // namespace cppgo

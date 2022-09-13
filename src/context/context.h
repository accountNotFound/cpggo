#pragma once

#include <unordered_map>
#include <unordered_set>

#include "common.h"
#include "common/allocator.h"
#include "common/spinlock.h"
#include "coroutine/coroutine.h"

namespace cppgo {

class Context {
  friend class Task;
  friend class Worker;
  friend class Monitor;

 public:
  Context(size_t worker_num);

  void spawn(AsyncFunction<void>&& func);
  bool idle();
  void stop();
  Task* this_running_task();

 private:
  SpinLock self_;

  // resource managers
  Allocator<Worker> worker_mgr_;
  Allocator<Task> task_mgr_;

  // just reference to resource above
  std::unordered_set<Task*> runnable_set_;
  std::unordered_set<Task*> blocked_set_;
  std::unordered_map<Task*, Worker*> running_map_;
};

}  // namespace cppgo

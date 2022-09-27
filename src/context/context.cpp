#include "context.h"

#include "task.h"
#include "worker.h"

namespace cppgo {

Context::Context(size_t worker_num) {
  for (int i = 0; i < worker_num; i++) {
    worker_mgr_.add(new Worker(this));
  }
  for (auto& worker : worker_mgr_) {
    worker->start();
  }
}

void Context::spawn(AsyncFunction<void>&& func) {
  std::unique_lock guard(self_);
  auto task = task_mgr_.add(new Task(this, std::move(func)));
  runnable_set_.insert(task);
}

bool Context::idle() {
  std::unique_lock guard(self_);
  return runnable_set_.empty() && blocked_set_.empty() && running_map_.empty();
}

void Context::stop() {
  for (auto& worker : worker_mgr_) {
    worker->stop();
  }
  for (auto& worker : worker_mgr_) {
    worker->join();
  }
  done_ = true;
}

// get current running task on this thread
Task* Context::this_running_task() {
  static Task no_schedule_task;
  std::unique_lock guard(self_);
  for (auto& [task, worker] : running_map_) {
    if (worker->thread_id() == std::this_thread::get_id()) {
      return task;
    }
  }
  return &no_schedule_task;
}

}  // namespace cppgo

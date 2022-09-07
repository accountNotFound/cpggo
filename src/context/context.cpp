#include "context.h"

#include "common/error.h"

// #define USE_DEBUG
#include "common/log.h"

namespace cppgo {

Context::Context(size_t worker_num) {
  for (int i = 0; i < worker_num; i++) {
    workers_.insert(new Worker(this));
    // delete in Context::~Context
  }
  for (auto& w : workers_) {
    w->start();
  }
}

Context::~Context() {
  for (auto& w : workers_) {
    delete w;
  }
  for (auto& t : tasks_) {
    delete t;
  }
}

// get current running task on this thread
Context::TaskPointer Context::this_running_task() {
  std::unique_lock guard(mtx_);
  for (auto& [task, worker] : running_map_) {
    if (worker->id() == std::this_thread::get_id()) {
      return task;
    }
  }
  RAISE("this thread has no relevance to context");
}

void Context::event_loop() {
  // TODO
}

void Context::stop() {
  DEBUG("emit context stop signal", 0);
  done_ = true;
  for (auto& w : workers_) {
    w->join();
  }
}

bool Context::idle() {
  std::unique_lock guard(mtx_);
  return runnable_set_.empty() && blocked_set_.empty() && running_map_.empty();
}

}  // namespace cppgo

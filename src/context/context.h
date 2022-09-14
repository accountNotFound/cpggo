#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "common/allocator.h"
#include "common/spinlock.h"
#include "coroutine/coroutine.h"

namespace cppgo {

class Task;
class Worker;
class Context;
class Monitor;

class Task {
  friend class Worker;
  friend class Monitor;

 public:
  enum Status {
    RUNNABLE = 0,
    BLOCKED,
    RUNNING,
  };

 public:
  Task(Context* ctx, std::unique_ptr<AsyncFuncBase>&& func)
      : ctx_(ctx), func_(std::move(func)) {
    func_->init();
  }
  Task(Task&) = delete;

  size_t id() { return reinterpret_cast<size_t>(this); }
  void resume() { func_->resume(); }
  bool done() { return func_->done(); }
  Status status() { return status_; }

 private:
  Context* ctx_;  // just reference
  std::unique_ptr<AsyncFuncBase> func_;
  std::function<void()> callback_;  // only use to update this task's status
                                    // called by worker with context's mutex
  Status status_ = Status::RUNNABLE;
};

class Worker {
 public:
  enum Status {
    INIT = 0,
    RUNNING,
    DONE,
  };

 public:
  Worker(Context* ctx) : ctx_(ctx) {}
  Worker(Worker&) = delete;

  std::thread::id id() { return thread_.get_id(); }
  Status status() { return status_; }

  void start();
  void join();

 private:
  Context* ctx_;  // just reference
  std::thread thread_;
  Status status_ = Status::INIT;
};

class Context {
  friend class Task;
  friend class Worker;
  friend class Monitor;

 public:
  using TaskPointer = Task*;
  using WorkerPointer = Worker*;

 public:
  Context(size_t worker_num);
  Context(Context&) = delete;

  bool done() { return done_; }

  // it is meanfulless to support future or promise with return value in
  // coroutine programming, instead you should simply co_await an async_function
  // to get its return value
  void spawn(AsyncFunction<void>&& func);
  TaskPointer this_running_task();
  void event_loop();
  void stop();
  bool idle();

 private:
  SpinLock mtx_;

  // resource pool
  Allocator<Worker> workers_;
  Allocator<Task> tasks_;

  // for tasks status change, point to object in resource pool
  std::unordered_set<TaskPointer> runnable_set_;
  std::unordered_set<TaskPointer> blocked_set_;
  std::unordered_map<TaskPointer, WorkerPointer> running_map_;

  bool done_ = false;
};

class Monitor {
 public:
  Monitor(Context* ctx) : ctx_(ctx) {}
  Monitor(Monitor&) = delete;

  AsyncFunction<void> enter();
  AsyncFunction<void> wait();
  void notify_one();
  void exit();

 private:
  Context* ctx_;  // just reference
  std::unordered_set<Context::TaskPointer> blocked_set_;

  // once notify_one() is called, it will push a
  // callback in this queue.
  // these functions will be called with context's
  // mutex when exit() or wait().
  std::queue<std::function<void()>> notify_callback_queue_;

  SpinLock mtx_;  // just protect members below
                  // the blocked_set_ will  be protected by ctx_->mtx_
  bool idle_ = true;
};

}  // namespace cppgo
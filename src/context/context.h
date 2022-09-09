#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

  // funtion added by this method will be executed when this running task
  // co_await, normally use to change relevant tasks' status
  void add_callback(std::function<void()>&& fn) { callbacks_.push_back(fn); }

  // you should make sure that this running task can continue to be executed
  // after call this method
  void submit_callback_delegate();

 private:
  Context* ctx_;  // just reference
  std::unique_ptr<AsyncFuncBase> func_;
  std::vector<std::function<void()>> callbacks_;
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
  ~Context();

  template <typename AsyncFuncDerived>
  requires std::is_base_of_v<AsyncFuncBase, AsyncFuncDerived>
  void spawn(AsyncFuncDerived&& func) {
    auto p_task =
        new Task(this, std::make_unique<AsyncFuncDerived>(std::move(func)));
    // delete in Worker::start() & Context::~Context
    std::unique_lock guard(mtx_);
    tasks_.insert(p_task);
    runnable_set_.insert(p_task);
  }

  bool done() { return done_; }

  TaskPointer this_running_task();
  void event_loop();
  void stop();
  bool idle();

 private:
  SpinLock mtx_;

  // resource pool
  std::unordered_set<WorkerPointer> workers_;
  std::unordered_set<TaskPointer> tasks_;

  // for tasks status change, point to object in resource pool
  std::unordered_set<TaskPointer> runnable_set_;
  std::unordered_set<TaskPointer> blocked_set_;
  std::unordered_map<TaskPointer, WorkerPointer> running_map_;

  std::unordered_map<TaskPointer, std::vector<std::function<void()>>>
      delegate_map_;

  bool done_ = false;
};

class Monitor {
 public:
  Monitor(Context* ctx) : ctx_(ctx) {}
  Monitor(Monitor&) = delete;

  AsyncFunction<void> enter();
  AsyncFunction<void> exit();
  AsyncFunction<void> wait();
  void notify_one();
  void exit_nowait();

 private:
  Context* ctx_;  // just reference
  std::unordered_set<Context::TaskPointer> blocked_set_;

  SpinLock mtx_;  // just protect idle_
                  // the blocked_set_ will  be protected by ctx_->mtx_
  bool idle_ = true;
};

}  // namespace cppgo
#pragma once

#include <functional>
#include <stack>

#include "common/spinlock.h"
#include "coroutine/coroutine.h"
#include "header.h"

namespace cppgo {

class Task {
  friend class Context;
  friend class Worker;
  friend class Monitor;

 public:
  enum Status {
    RUNNABLE = 0,
    RNUNING,
    BLOCKED,
  };
  using Id = size_t;

 public:
  static const char* statu_str(Status s) {
    switch (s) {
      case Status::RUNNABLE:
        return "runnable";
      case Status::RNUNING:
        return "running";
      case Status::BLOCKED:
        return "blocked";
    }
    return "";
  }

 public:
  Task(Task&) = delete;

  Id id() { return id_; }
  Status status() { return status_; }
  void init() { func_.init(); }
  void resume() { func_.resume(); }
  bool done() { return func_.done(); }

 private:
  Task();
  Task(Context* ctx, AsyncFunction<void>&& func);

  // called in Task's callback, should be protected by context's lock
  void change_status_(Status from, Status to);

 private:
  // just for unique id generation
  static SpinLock cls_;
  static Id cls_id_;

  Id id_;

  Context* ctx_;
  Status status_;
  AsyncFunction<void> func_;

  // After task co_await to Worker schedule loop, this callback will be executed
  // to update status, with context's mutex protection.
  // Return true means it is still runnable, or otherwise
  std::function<bool()> callback_ = nullptr;
};

}  // namespace cppgo

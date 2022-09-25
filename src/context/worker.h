#pragma once

#include <thread>
#include <unordered_map>

#include "header.h"
#include "common/spinlock.h"

namespace cppgo {

class Worker {
  friend class Context;
  friend class Task;
  friend class Monitor;

 public:
  enum Status {
    INIT = 0,
    RUNNING,
    DONE,
  };
  using Id = size_t;

 public:
  Worker(Worker&) = delete;

  Id id() { return id_; }
  std::thread::id thread_id() { return thread_.get_id(); }
  Status status() { return status_; }
  void stop() { stop_flag_ = true; }

  void start();
  void join();

 private:
  Worker(Context* ctx);

 private:
  // just for unique id generation
  static SpinLock cls_;
  static Id cls_id_;

  Id id_;
  Context* ctx_;
  Status status_;
  std::thread thread_;
  bool stop_flag_;
};

}  // namespace cppgo

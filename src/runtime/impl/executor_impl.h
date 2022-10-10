#pragma once

#include <thread>

#include "../context.h"
#include "../executor.h"
#include "util/spin_lock.h"

namespace cppgo {

class Executor::Impl {
 public:
  Impl(Context& ctx);

 public:
  void start() { _thread = std::thread(&Impl::_loop, this); }
  void join() { _thread.join(); }
  void stop() { _stop_flag = true; }
  size_t id() { return _id; }

 private:
  void _loop();

 private:
  static SpinLock _cls_mtx;
  static size_t _cls_id;

 private:
  size_t _id;
  std::thread _thread;
  bool _stop_flag = false;

 private:
  Context::Impl* _ctx_impl;
};

}  // namespace cppgo

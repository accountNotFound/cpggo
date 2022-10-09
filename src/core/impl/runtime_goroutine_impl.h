#pragma once

#include "runtime_context_impl.h"
#include "util/spin_lock.h"

namespace cppgo {

class Goroutine::Impl {
 public:
  Impl(Goroutine& this_wrapper, Context& ctx, AsyncFunctionBase&& fn);

 public:
  size_t id() { return _id; }
  void set_runnable() { _ctx_impl->_runnable_queue.enqueue(_this()); }
  void set_blocked() { _blocked_flag = true; }
  void run();
  bool done() { return _func.done(); }

 private:
  Goroutine* _this() { return _this_wrapper; }

 private:
  static SpinLock _cls_mtx;
  static size_t _cls_id;

 private:
  size_t _id;
  Goroutine* _this_wrapper;
  Context::Impl* _ctx_impl;
  SpinLock _mtx;
  AsyncFunctionBase _func;
  bool _blocked_flag = false;
};

}  // namespace cppgo

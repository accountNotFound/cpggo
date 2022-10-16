#pragma once

#include "../context.h"
#include "../goroutine.h"
#include "context_impl.h"
#include "util/spin_lock.h"

namespace cppgo {

class Goroutine::Impl {
 public:
  Impl(Goroutine& this_wrapper, Context& ctx, AsyncFunctionBase&& fn);

 public:
  size_t id() { return _id; }
  void set_runnable() { _ctx_impl->runnable_queue.enqueue(std::move(_this_wrapper)); }
  void set_blocked() { _blocked_flag = true; }
  void run();
  bool done() { return _func.done(); }

 private:
  static SpinLock _cls_mtx;
  static size_t _cls_id;

 private:
  size_t _id;
  SpinLock _mtx;
  bool _blocked_flag = false;

 private:
  Goroutine* _this_wrapper;
  Context::Impl* _ctx_impl;
  AsyncFunctionBase _func;
};

}  // namespace cppgo

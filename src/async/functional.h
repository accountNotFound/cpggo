#pragma once

#include "promise.h"

namespace cppgo {

class AsyncFunctionBase {
  friend class PromiseBase;

 public:
  AsyncFunctionBase() = delete;
  AsyncFunctionBase(PromiseBase& promise);
  AsyncFunctionBase(AsyncFunctionBase&) = delete;
  AsyncFunctionBase(AsyncFunctionBase&&);
  virtual ~AsyncFunctionBase();

 public:
  void init();
  void resume();
  bool done();

 protected:
  void _await_suspend(PromiseBase& caller);
  std::any _await_resume();

 protected:
  class Impl;
  std::unique_ptr<Impl> _impl;
};

template <typename T>
class AsyncFunction : public AsyncFunctionBase {
 public:
  struct promise_type : public Promise<T> {
    AsyncFunction get_return_object() {
      this->init(std::coroutine_handle<promise_type>::from_promise(*this));
      return AsyncFunction(*this);
    }
  };

 public:
  bool await_ready() { return this->done(); }

  template <typename PromiseDerived>
  requires std::is_base_of_v<PromiseBase, PromiseDerived>
  void await_suspend(std::coroutine_handle<PromiseDerived> caller) { this->_await_suspend(caller.promise()); }

  T await_resume() {
    if constexpr (!std::is_same_v<T, void>) {
      return std::any_cast<T>(std::move(this->_await_resume()));
    }
  }

 private:
  AsyncFunction(PromiseBase& promise) : AsyncFunctionBase(promise) {}
};

}  // namespace cppgo

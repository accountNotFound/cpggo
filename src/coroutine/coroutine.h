#pragma once

#include "promise.h"

namespace cppgo {

template <typename T>
class AsyncFuncPromise;

template <typename T>
class AsyncFunction;

template <typename T>
class AsyncFuncPromise : public ValuePromiseBase<T> {
  template <typename U>
  friend class AsyncFunction;

 public:
  AsyncFuncPromise() = default;

  AsyncFunction<T> get_return_object() {
    return {std::coroutine_handle<AsyncFuncPromise>::from_promise(*this)};
  }
};

class AsyncFuncBase {
 public:
  virtual void init() = 0;
  virtual void resume() = 0;
  virtual bool done() = 0;
};

template <typename T>
class AsyncFunction : public AsyncFuncBase {
  template <typename U>
  friend class AsyncFuncPromise;

  using TypeHandler = std::coroutine_handle<AsyncFuncPromise<T>>;

 public:
  using promise_type = AsyncFuncPromise<T>;

  AsyncFunction() = delete;
  AsyncFunction(const AsyncFunction&) = delete;
  AsyncFunction(AsyncFunction&& afn) {
    if (type_handler_) {
      type_handler_.destroy();
    }
    type_handler_ = afn.type_handler_;
    afn.type_handler_ = nullptr;
  }

  ~AsyncFunction() {
    if (type_handler_) {
      type_handler_.destroy();
    }
  }

  void init() {
    type_handler_.promise().p_stack_ =
        new std::stack<RawHandler>({type_handler_});
  }

  void resume() {
    std::stack<RawHandler>* stk = type_handler_.promise().p_stack_;
    while (!stk->empty() && stk->top().done()) {
      stk->pop();
    }
    if (!stk->empty()) {
      stk->top().resume();
    }
  }

  bool done() { return await_ready(); }

  /*
  awaiter trait below
  */
  template <typename PromiseDerived>
  requires std::is_base_of_v<PromiseBase, PromiseDerived>
  void await_suspend(std::coroutine_handle<PromiseDerived> caller) {
    caller.promise().p_stack_->push(type_handler_);
    type_handler_.promise().p_stack_ = caller.promise().p_stack_;
    type_handler_.promise().caller_ = caller;
  }

  T await_resume() {
    if constexpr (!std::is_same_v<T, void>) {
      return std::move(type_handler_.promise().value_);
    }
  }

  bool await_ready() const { return !type_handler_ || type_handler_.done(); }

 private:
  TypeHandler type_handler_;

  AsyncFunction(TypeHandler type_handler) : type_handler_(type_handler) {}
};

}  // namespace cppgo

#pragma once

#include <exception>
#include <stack>

#include "functional.h"
#include "promise.h"

namespace cppgo {

class PromiseBase::Impl {
  friend class PromiseBase;
  friend class AsyncFunctionBase::Impl;

 public:
  void init(std::coroutine_handle<> handler) { _handler = handler; }
  void unhandled_exception() noexcept { _exception = std::current_exception(); }
  std::any& any() { return _value; }

 protected:
  std::suspend_always _yield_any(std::any&& value) {
    _value = std::move(value);
    return {};
  }

 private:
  std::coroutine_handle<> _handler = nullptr;
  std::exception_ptr _exception = nullptr;
  std::any _value = nullptr;
  std::shared_ptr<std::stack<std::coroutine_handle<>>> _stack = nullptr;
};

}  // namespace cppgo

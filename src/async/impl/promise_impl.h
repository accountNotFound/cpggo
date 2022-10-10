#pragma once

#include <exception>
#include <stack>

#include "../functional.h"
#include "../promise.h"

namespace cppgo {

class PromiseBase::Impl {
 public:
  void init(std::coroutine_handle<> handler) { this->handler = handler; }
  void unhandled_exception() noexcept { this->error = std::current_exception(); }
  std::any& any() { return this->value; }
  std::suspend_always _yield_any(std::any&& value) {
    this->value = std::move(value);
    return {};
  }

 public:
  std::coroutine_handle<> handler = nullptr;
  std::exception_ptr error = nullptr;
  std::any value = nullptr;
  std::shared_ptr<std::stack<std::coroutine_handle<>>> stack = nullptr;
};

}  // namespace cppgo

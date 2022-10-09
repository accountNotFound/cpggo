#pragma once

#include <any>
#include <coroutine>
#include <memory>

namespace cppgo {

class PromiseBase;
class AsyncFunctionBase;

class PromiseBase {
  friend class AsyncFunctionBase;

 public:
  PromiseBase();
  PromiseBase(PromiseBase&) = delete;
  PromiseBase(PromiseBase&&) = default;
  virtual ~PromiseBase();

 public:
  void init(std::coroutine_handle<> handler);
  std::suspend_always initial_suspend() { return {}; }
  std::suspend_always final_suspend() noexcept { return {}; }
  void unhandled_exception() noexcept;
  std::any& any();

 protected:
  std::suspend_always _yield_any(std::any&& value);

 protected:
  class Impl;
  std::unique_ptr<Impl> _impl;
};

template <typename T>
class Promise : public PromiseBase {
 public:
  std::suspend_always yield_value(T&& value) { return this->_yield_any(value); }
  void return_value(T&& value) { this->_yield_any(std::move(value)); }
};

template <>
class Promise<void> : public PromiseBase {
 public:
  void return_void() {}
};

}  // namespace cppgo

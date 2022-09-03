#pragma once

#include <coroutine>
#include <stack>

namespace cppgo {

using RawHandler = std::coroutine_handle<>;

struct PromiseBase {
 public:
  PromiseBase() = default;

  std::suspend_always initial_suspend() const { return {}; }
  std::suspend_always final_suspend() const noexcept { return {}; }
  void unhandled_exception() noexcept {}

 protected:
  std::stack<RawHandler>* p_stack_ = nullptr;
};

template <typename T>
struct ValuePromiseBase : public PromiseBase {
 public:
  ValuePromiseBase() = default;

  void return_value(T& value) { value_ = value; }
  void return_value(T&& value) { value_ = std::move(value); }

 protected:
  T value_;
};

template <>
struct ValuePromiseBase<void> : public PromiseBase {
 public:
  ValuePromiseBase() = default;

  void return_void() {}
};

}  // namespace cppgo

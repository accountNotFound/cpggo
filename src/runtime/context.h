#pragma once

#include <functional>
#include <memory>

#include "async/functional.h"
#include "util/unwrapper.h"

namespace cppgo {

class Goroutine;

class Context {
  template <__detail::HasImpl T>
  friend typename T::Impl& __detail::impl(T& wrapper);

 public:
  Context(size_t executor_num);
  ~Context();

 public:
  Goroutine& go(AsyncFunctionBase&& fn);
  Goroutine& current_goroutine();
  void start();
  void wait_until(const std::function<bool()>& pred);

 public:
  class Impl;

 private:
  std::unique_ptr<Impl> _impl;
};

}  // namespace cppgo
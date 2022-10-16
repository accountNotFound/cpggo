#pragma once

#include <functional>
#include <memory>

#include "util/unwrapper.h"

namespace cppgo {

class Goroutine {
  template <__detail::HasImpl T>
  friend typename T::Impl& __detail::impl(T& wrapper);

 public:
  Goroutine(Context& ctx, AsyncFunctionBase&& fn);
  ~Goroutine();

 public:
  size_t id() const;
  operator size_t() const { return id(); }

 private:
  class Impl;
  std::unique_ptr<Impl> _impl;
};

}  // namespace cppgo
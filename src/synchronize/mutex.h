#pragma once

#include <memory>

#include "async/functional.h"
#include "runtime/context.h"

namespace cppgo {

class Mutex {
 public:
  Mutex(Context& ctx);
  Mutex(const Mutex& rhs) = default;
  ~Mutex();

 public:
  AsyncFunction<void> lock();
  void unlock();

 private:
  class Impl;
  std::shared_ptr<Impl> _impl;
};

}  // namespace cppgo

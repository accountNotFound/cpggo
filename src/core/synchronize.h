#pragma once

#include <memory>

#include "async/functional.h"
#include "runtime.h"

namespace cppgo {

class Mutex;
class ConditonVariable;

class Mutex {
  friend class ConditionVariable;

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

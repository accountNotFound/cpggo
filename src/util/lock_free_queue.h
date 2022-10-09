#pragma once

#include <any>
#include <memory>

namespace cppgo {

class LockFreeQueueBase {
 public:
  LockFreeQueueBase();
  virtual ~LockFreeQueueBase();

 public:
  void enqueue(std::any&& value);
  std::any dequeue();

 private:
  class Impl;
  std::unique_ptr<Impl> _impl;
};

template <typename T>
class LockFreeQueue : public LockFreeQueueBase {
 public:
  void enqueue(T&& value) { LockFreeQueueBase::enqueue(std::move(value)); }
  std::pair<T, bool> dequeue() {
    auto v = LockFreeQueueBase::dequeue();
    if (v.type() == typeid(std::nullptr_t)) return {T(), false};
    return {std::any_cast<T>(std::move(v)), true};
  }
};

}  // namespace cppgo

#include "../lock_free_queue.h"

#include <queue>

#include "spin_lock.h"

namespace cppgo {

class LockFreeQueueBase::Impl {
 public:
  void enqueue(std::any&& value) {
    std::unique_lock guard(_mtx);
    _que.emplace(std::move(value));
  }
  std::any dequeue() {
    std::unique_lock guard(_mtx);
    if (_que.empty()) return nullptr;
    auto res = std::move(_que.front());
    _que.pop();
    return res;
  }

 private:
  SpinLock _mtx;
  std::queue<std::any> _que;
};

LockFreeQueueBase::LockFreeQueueBase() : _impl(std::make_unique<Impl>()) {}

LockFreeQueueBase::~LockFreeQueueBase() = default;

void LockFreeQueueBase::enqueue(std::any&& value) { _impl->enqueue(std::move(value)); }

std::any LockFreeQueueBase::dequeue() { return _impl->dequeue(); }

}  // namespace cppgo

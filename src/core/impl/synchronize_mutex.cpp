#include "runtime_goroutine_impl.h"
#include "synchronize.h"
#include "util/lock_free_queue.h"
#include "util/spin_lock.h"

namespace cppgo {

class Mutex::Impl {
 public:
  Impl(Context& ctx) : _ctx() {}

 public:
  AsyncFunction<void> lock() {
    while (true) {
      std::unique_lock guard(_mtx);
      if (_locked_flag) {
        _blocked_queue.enqueue(&_ctx->current_goroutine());
        __detail::unwrap(_ctx->current_goroutine()).change_blocked();
        co_await std::suspend_always{};
      } else {
        _locked_flag = true;
        break;
      }
    }
  }
  void unlock() {
    std::unique_lock guard(_mtx);
    auto [next_goroutine, ok] = _blocked_queue.dequeue();
    if (ok) __detail::unwrap(*next_goroutine).change_runnable();
    _locked_flag = false;
  }

 private:
  SpinLock _mtx;
  Context* _ctx;
  LockFreeQueue<Goroutine*> _blocked_queue;
  bool _locked_flag = false;
};

Mutex::Mutex(Context& ctx) : _impl(std::make_shared<Impl>(ctx)) {}

Mutex::~Mutex() = default;

AsyncFunction<void> Mutex::lock() { co_await _impl->lock(); }

void Mutex::unlock() { _impl->unlock(); }

}  // namespace cppgo

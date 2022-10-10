#include "../mutex.h"

#include "runtime/impl/goroutine_impl.h"
#include "util/lock_free_queue.h"
#include "util/spin_lock.h"

// #define USE_DEBUG
#include "util/log.h"

namespace cppgo {

class Mutex::Impl {
 public:
  Impl(Context& ctx) : _ctx(&ctx) {}

 public:
  AsyncFunction<void> lock() {
    while (true) {
      _mtx.lock();
      if (_locked_flag) {
        DEBUG("routine {%u} lock failed -> blocked", _ctx->current_goroutine().id());
        _blocked_queue.enqueue(&_ctx->current_goroutine());
        __detail::impl(_ctx->current_goroutine()).set_blocked();
        _mtx.unlock();
        co_await std::suspend_always{};
      } else {
        DEBUG("routine {%u} lock", _ctx->current_goroutine().id());
        _locked_flag = true;
        _mtx.unlock();
        break;
      }
    }
  }
  void unlock() {
    std::unique_lock guard(_mtx);
    DEBUG("routine {%u} unlock", _ctx->current_goroutine().id());
    auto [next_goroutine, ok] = _blocked_queue.dequeue();
    if (ok) {
      DEBUG("routine {%u} is notified -> runnable", next_goroutine->id());
      __detail::impl(*next_goroutine).set_runnable();
    }
    _locked_flag = false;
  }

 private:
  Context* _ctx;
  SpinLock _mtx;
  LockFreeQueue<Goroutine*> _blocked_queue;
  bool _locked_flag = false;
};

Mutex::Mutex(Context& ctx) : _impl(std::make_shared<Impl>(ctx)) {}

Mutex::~Mutex() = default;

AsyncFunction<void> Mutex::lock() { co_await _impl->lock(); }

void Mutex::unlock() { _impl->unlock(); }

}  // namespace cppgo

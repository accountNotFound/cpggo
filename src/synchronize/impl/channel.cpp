#include "../channel.h"

#include <queue>

#include "runtime/impl/goroutine_impl.h"
#include "util/lock_free_queue.h"
#include "util/spin_lock.h"

// #define USE_DEBUG
#include "util/log.h"

namespace cppgo {

class ChannelBase::Impl {
 public:
  Impl(Context& ctx, size_t capacity) : _ctx(&ctx), _capacity(capacity) {}

 public:
  AsyncFunction<void> send_any(std::any&& value) {
    while (true) {
      _mtx.lock();
      if (_buffer.size() == _capacity) {
        DEBUG("routine {%d} send failed", _ctx->current_goroutine().id());
        _blocked_writers.enqueue(&_ctx->current_goroutine());
        __detail::impl(_ctx->current_goroutine()).set_blocked();
        _mtx.unlock();
        co_await std::suspend_always{};
      } else {
        DEBUG("routine {%d} send ok", _ctx->current_goroutine().id());
        _buffer.push(std::move(value));
        _notify_one(_blocked_readers);
        if (_buffer.size() < _capacity) _notify_one(_blocked_writers);
        _mtx.unlock();
        break;
      }
    }
  }
  AsyncFunction<std::any> recv_any() {
    std::any res;
    while (true) {
      _mtx.lock();
      if (_buffer.size() == 0) {
        DEBUG("routine {%d} recv failed", _ctx->current_goroutine().id());
        _blocked_readers.enqueue(&_ctx->current_goroutine());
        __detail::impl(_ctx->current_goroutine()).set_blocked();
        _mtx.unlock();
        co_await std::suspend_always{};
      } else {
        DEBUG("routine {%d} recv ok", _ctx->current_goroutine().id());
        res = std::move(_buffer.front());
        _buffer.pop();
        _notify_one(_blocked_writers);
        if (_buffer.size() > 0) _notify_one(_blocked_readers);
        _mtx.unlock();
        break;
      }
    }
    co_return std::move(res);
  }

  bool send_any_noblock(std::any&& value) {
    std::unique_lock guard(_mtx);
    if (_buffer.size() == _capacity) return false;
    DEBUG("routine {%d} send ok", _ctx->current_goroutine().id());
    _buffer.push(std::move(value));
    _notify_one(_blocked_readers);
    return true;
  }

 private:
  void _notify_one(LockFreeQueue<Goroutine*>& blocked_queue) {
    auto [next_goroutine, ok] = blocked_queue.dequeue();
    if (ok) __detail::impl(*next_goroutine).set_runnable();
  }

 private:
  Context* _ctx;
  size_t _capacity;
  SpinLock _mtx;
  std::queue<std::any> _buffer;
  LockFreeQueue<Goroutine*> _blocked_readers, _blocked_writers;
};

ChannelBase::ChannelBase(Context& ctx, size_t capacity) : _impl(std::make_shared<Impl>(ctx, capacity)) {}

ChannelBase::~ChannelBase() = default;

AsyncFunction<void> ChannelBase::send_any(std::any&& value) { co_await _impl->send_any(std::move(value)); }

AsyncFunction<std::any> ChannelBase::recv_any() { co_return (co_await _impl->recv_any()); }

bool ChannelBase::send_any_noblock(std::any&& value) { return _impl->send_any_noblock(std::move(value)); }

}  // namespace cppgo

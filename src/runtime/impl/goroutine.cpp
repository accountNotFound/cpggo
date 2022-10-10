#include "goroutine_impl.h"

namespace cppgo {

SpinLock Goroutine::Impl::_cls_mtx;
size_t Goroutine::Impl::_cls_id = 0;

Goroutine::Impl::Impl(Goroutine& this_wrapper, Context& ctx, AsyncFunctionBase&& fn)
    : _this_wrapper(&this_wrapper), _ctx(&ctx), _func(std::move(fn)) {
  std::unique_lock guard(_cls_mtx);
  _id = ++_cls_id;
  _func.init();
}

void Goroutine::Impl::run() {
  _mtx.lock();
  while (!_func.done()) {
    _func.resume();
    if (_blocked_flag) {
      _blocked_flag = false;
      break;
    }
  }

  // copy these value to destroy goroutine

  auto done = _func.done();
  auto gid = id();
  auto& ctx_impl = __detail::impl(*_ctx);  // Context's life time is longer than `this`

  _mtx.unlock();

  // try destroy goroutine if it is done
  // use copy above instead of `this` pointer, because `this` may be already destroyed by other threads
  if (done) ctx_impl.goroutines.erase(gid);
}

Goroutine::Goroutine(Context& ctx, AsyncFunctionBase&& fn) : _impl(std::make_unique<Impl>(*this, ctx, std::move(fn))) {}

Goroutine::~Goroutine() = default;

size_t Goroutine::id() const { return _impl->id(); }

}  // namespace cppgo

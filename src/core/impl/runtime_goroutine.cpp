#include "runtime_goroutine_impl.h"

namespace cppgo {

SpinLock Goroutine::Impl::_cls_mtx;
size_t Goroutine::Impl::_cls_id = 0;

Goroutine::Impl::Impl(Goroutine& this_wrapper, Context& ctx, AsyncFunctionBase&& fn)
    : _this_wrapper(&this_wrapper), _ctx_impl(ctx._impl.get()), _func(std::move(fn)) {
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
  _mtx.unlock();
  if (_func.done()) _this_wrapper->~Goroutine();
}

Goroutine::Goroutine(Context& ctx, AsyncFunctionBase&& fn) : _impl(std::make_unique<Impl>(*this, ctx, std::move(fn))) {}

Goroutine::~Goroutine() = default;

size_t Goroutine::id() const { return _impl->id(); }

}  // namespace cppgo

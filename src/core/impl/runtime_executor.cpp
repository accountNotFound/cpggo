#include "runtime_executor_impl.h"
#include "runtime_goroutine_impl.h"

namespace cppgo {

SpinLock Executor::Impl::_cls_mtx;
size_t Executor::Impl::_cls_id = 0;

Executor::Impl::Impl(Context& ctx) : _ctx_impl(ctx._impl.get()) {
  std::unique_lock guard(_cls_mtx);
  _id = ++_cls_id;
}

void Executor::Impl::_loop() {
  while (!_stop_flag) {
    auto [routine, ok] = _ctx_impl->_runnable_queue.dequeue();
    if (!ok) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      continue;
    }
    _ctx_impl->_this_thread_goroutine = routine;
    routine->_impl->run();
    _ctx_impl->_this_thread_goroutine = nullptr;
  }
}

Executor::Executor(Context& ctx) : _impl(std::make_unique<Impl>(ctx)) {}

Executor::~Executor() = default;

size_t Executor::id() const { return _impl->id(); }

}  // namespace cppgo

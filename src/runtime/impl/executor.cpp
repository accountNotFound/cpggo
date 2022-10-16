#include "executor_impl.h"
#include "goroutine_impl.h"

// #define USE_DEBUG
#include "util/log.h"

#define LOOP_INTERVAL 50

namespace cppgo {

SpinLock Executor::Impl::_cls_mtx;
size_t Executor::Impl::_cls_id = 0;

Executor::Impl::Impl(Context& ctx) : _ctx_impl(&__detail::impl(ctx)) {
  std::unique_lock guard(_cls_mtx);
  _id = ++_cls_id;
}

void Executor::Impl::_loop() {
  while (!_stop_flag) {
    auto [routine, ok] = _ctx_impl->runnable_queue.dequeue();
    if (!ok) {
      DEBUG("executor {%d} sleep", 0);
      std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_INTERVAL));
      continue;
    }
    _ctx_impl->this_thread_goroutine = routine;
    DEBUG("executor {%d} start run routine {%d}", id(), routine->id());
    __detail::impl(*routine).run();
    DEBUG("executor {%d} run routine {%d} end", id(), routine->id());
    _ctx_impl->this_thread_goroutine = nullptr;

    // finish goroutines have been already destroy in Goroutine::Impl::run()
  }
}

Executor::Executor(Context& ctx) : _impl(std::make_unique<Impl>(ctx)) {}

Executor::~Executor() = default;

size_t Executor::id() const { return _impl->id(); }

}  // namespace cppgo

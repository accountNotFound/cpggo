#include <chrono>
#include <thread>

#include "runtime_context_impl.h"
#include "runtime_executor_impl.h"

#define USE_DEBUG
#include "util/log.h"

namespace cppgo {

thread_local Goroutine* Context::Impl::_this_thread_goroutine = nullptr;

Context::Impl::Impl(Context& this_wrapper, size_t executor_num)
    : _this_wrapper(&this_wrapper), _executor_num(executor_num) {
  // can not create Executors here because `this` pointer is still imcomplete
}

Goroutine& Context::Impl::go(AsyncFunctionBase&& fn) {
  auto [iter, ok] = _goroutines.emplace(*_this_wrapper, std::move(fn));
  _runnable_queue.enqueue(&const_cast<Goroutine&>(*iter));
  return const_cast<Goroutine&>(*iter);
}

Goroutine& Context::Impl::current_goroutine() { return *_this_thread_goroutine; }

void Context::Impl::start() {
  for (int i = 0; i < _executor_num; ++i) _executors.emplace(*_this_wrapper);
  for (auto& exec : _executors) exec._impl->start();
}

void Context::Impl::wait_until(const std::function<bool()>& pred) {
  while (!pred()) std::this_thread::sleep_for(std::chrono::milliseconds(50));
  for (auto& exec : _executors) exec._impl->stop();
  for (auto& exec : _executors) exec._impl->join();
}

Context::Context(size_t executor_num) : _impl(std::make_unique<Impl>(*this, executor_num)) {}

Context::~Context() = default;

Goroutine& Context::current_goroutine() { return _impl->current_goroutine(); }

Goroutine& Context::go(AsyncFunctionBase&& fn) { return _impl->go(std::move(fn)); }

void Context::start() { _impl->start(); }

void Context::wait_until(const std::function<bool()>& pred) { _impl->wait_until(pred); }

}  // namespace cppgo

#include <chrono>
#include <thread>

#include "context_impl.h"
#include "executor_impl.h"

// #define USE_DEBUG
#include "util/log.h"

namespace cppgo {

thread_local Goroutine* Context::Impl::this_thread_goroutine = nullptr;

Context::Impl::Impl(Context& this_wrapper, size_t executor_num)
    : _this_wrapper(&this_wrapper), _executor_num(executor_num) {
  // can not create Executors here because `this` pointer is still imcomplete
}

Goroutine& Context::Impl::go(AsyncFunctionBase&& fn) {
  auto [iter, ok] = goroutines.emplace(*_this_wrapper, std::move(fn));
  runnable_queue.enqueue(&const_cast<Goroutine&>(*iter));
  return const_cast<Goroutine&>(*iter);
}

void Context::Impl::start() {
  for (int i = 0; i < _executor_num; ++i) executors.emplace(*_this_wrapper);
  for (auto& exec : executors) __detail::impl(exec).start();
}

void Context::Impl::wait_until(const std::function<bool()>& pred) {
  while (!pred()) std::this_thread::sleep_for(std::chrono::milliseconds(50));
  for (auto& exec : executors) __detail::impl(exec).stop();
  for (auto& exec : executors) __detail::impl(exec).join();
}

Context::Context(size_t executor_num) : _impl(std::make_unique<Impl>(*this, executor_num)) {}

Context::~Context() = default;

Goroutine& Context::current_goroutine() { return _impl->current_goroutine(); }

Goroutine& Context::go(AsyncFunctionBase&& fn) { return _impl->go(std::move(fn)); }

void Context::start() { _impl->start(); }

void Context::wait_until(const std::function<bool()>& pred) { _impl->wait_until(pred); }

}  // namespace cppgo

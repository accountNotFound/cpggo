#include "promise.h"

#include "promise_impl.h"

namespace cppgo {

PromiseBase::PromiseBase() : _impl(std::make_unique<Impl>()) {}

PromiseBase::~PromiseBase() = default;

void PromiseBase::init(std::coroutine_handle<> handler) { _impl->init(handler); }

void PromiseBase::unhandled_exception() noexcept { _impl->unhandled_exception(); }

std::any& PromiseBase::any() { return _impl->any(); }

std::suspend_always PromiseBase::_yield_any(std::any&& value) { return _impl->_yield_any(std::move(value)); }

}  // namespace cppgo

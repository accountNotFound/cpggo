#include "promise_impl.h"

namespace cppgo {

class AsyncFunctionBase::Impl {
  friend class AsyncFunctionBase;

 public:
  Impl(PromiseBase& promise) : _prom_impl(&__detail::impl(promise)) {}

  Impl(Impl&& rhs) {
    this->~Impl();
    std::swap(_prom_impl, rhs._prom_impl);
  }

  ~Impl() {
    if (_prom_impl->handler && !_prom_impl->handler.done()) _prom_impl->handler.destroy();
    _prom_impl->handler = nullptr;
  }

 public:
  void init() {
    _prom_impl->stack = std::make_shared<std::stack<std::coroutine_handle<>>>();
    _prom_impl->stack->push(_prom_impl->handler);
  }

  void resume() {
    auto& stk = *_prom_impl->stack;
    while (!stk.empty() && stk.top().done()) stk.pop();
    if (!stk.empty()) stk.top().resume();
    auto& err = _prom_impl->error;
    if (err) std::rethrow_exception(err);
  }

  bool done() {
    auto& hdlr = _prom_impl->handler;
    bool done = !hdlr || hdlr.done();
    return done;
  }

 protected:
  void _await_suspend(PromiseBase& caller) {
    _prom_impl->stack = __detail::impl(caller).stack;
    _prom_impl->stack->push(_prom_impl->handler);
  }

  std::any _await_resume() {
    auto v = std::move(_prom_impl->value);
    _prom_impl->value = nullptr;
    return v;
  }

 private:
  PromiseBase::Impl* _prom_impl;
};

AsyncFunctionBase::AsyncFunctionBase(PromiseBase& promise) : _impl(std::make_unique<Impl>(promise)) {}

AsyncFunctionBase::AsyncFunctionBase(AsyncFunctionBase&& rhs) = default;

AsyncFunctionBase::~AsyncFunctionBase() = default;

void AsyncFunctionBase::init() { _impl->init(); }

void AsyncFunctionBase::resume() { _impl->resume(); }

bool AsyncFunctionBase::done() { return _impl->done(); }

void AsyncFunctionBase::_await_suspend(PromiseBase& caller) { _impl->_await_suspend(caller); }

std::any AsyncFunctionBase::_await_resume() { return _impl->_await_resume(); }

}  // namespace cppgo

#include "promise_impl.h"

namespace cppgo {

class AsyncFunctionBase::Impl {
  friend class AsyncFunctionBase;

 public:
  Impl(PromiseBase& promise) : _prom_impl(promise._impl.get()) {}

  Impl(Impl&& rhs) {
    this->~Impl();
    std::swap(_prom_impl, rhs._prom_impl);
  }

  ~Impl() {
    if (_prom_impl->_handler && !_prom_impl->_handler.done()) _prom_impl->_handler.destroy();
    _prom_impl->_handler = nullptr;
  }

 public:
  void init() {
    _prom_impl->_stack = std::make_shared<std::stack<std::coroutine_handle<>>>();
    _prom_impl->_stack->push(_prom_impl->_handler);
  }

  void resume() {
    auto& stk = _prom_impl->_stack;
    while (!stk->empty() && stk->top().done()) stk->pop();
    if (!stk->empty()) stk->top().resume();
    auto& err = _prom_impl->_exception;
    if (err) throw err;
  }

  bool done() {
    auto& hdlr = _prom_impl->_handler;
    return !hdlr || hdlr.done();
  }

 protected:
  void _await_suspend(PromiseBase& caller) {
    _prom_impl->_stack = caller._impl->_stack;
    _prom_impl->_stack->push(_prom_impl->_handler);
  }

  std::any _await_resume() {
    auto v = _prom_impl->_value;
    _prom_impl->_value = nullptr;
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

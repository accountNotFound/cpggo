#include "promise_impl.h"

namespace cppgo {

class AsyncFunctionBase::Impl {
  friend class AsyncFunctionBase;

 public:
  Impl(PromiseBase& promise) : _promise(&promise) {}

  Impl(Impl&& rhs) {
    this->~Impl();
    std::swap(_promise, rhs._promise);
  }

  ~Impl() {
    if (__detail::impl(*_promise).handler && !__detail::impl(*_promise).handler.done())
      __detail::impl(*_promise).handler.destroy();
    __detail::impl(*_promise).handler = nullptr;
  }

 public:
  void init() {
    __detail::impl(*_promise).stack = std::make_shared<std::stack<std::coroutine_handle<>>>();
    __detail::impl(*_promise).stack->push(__detail::impl(*_promise).handler);
  }

  void resume() {
    auto& stk = *__detail::impl(*_promise).stack;
    while (!stk.empty() && stk.top().done()) stk.pop();
    if (!stk.empty()) stk.top().resume();
    auto& err = __detail::impl(*_promise).error;
    if (err) std::rethrow_exception(err);
  }

  bool done() {
    auto& hdlr = __detail::impl(*_promise).handler;
    bool done = !hdlr || hdlr.done();
    return done;
  }

 protected:
  void _await_suspend(PromiseBase& caller) {
    __detail::impl(*_promise).stack = __detail::impl(caller).stack;
    __detail::impl(*_promise).stack->push(__detail::impl(*_promise).handler);
  }

  std::any _await_resume() {
    auto v = std::move(__detail::impl(*_promise).value);
    __detail::impl(*_promise).value = nullptr;
    return v;
  }

 private:
  PromiseBase* _promise;
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

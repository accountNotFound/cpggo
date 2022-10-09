#pragma once

#include <any>
#include <functional>
#include <memory>

#include "async/functional.h"
#include "util/lock_free_queue.h"

namespace cppgo {

class Goroutine;
class Executor;
class Context;

namespace __detail {

template <typename T>
typename T::Impl& unwrap(T& wrapper) {
  return *wrapper._impl.get();
}

};  // namespace __detail

class Context {
  friend class Executor;
  friend class Goroutine;

  template <typename T>
  friend typename T::Impl& __detail::unwrap(T&);

 public:
  Context(size_t executor_num);
  ~Context();

 public:
  Goroutine& go(AsyncFunctionBase&& fn);
  Goroutine& current_goroutine();
  void wait_until(const std::function<bool()>& pred);

 private:
  class Impl;
  std::unique_ptr<Impl> _impl;
};

class Executor {
  friend class Context;

  template <typename T>
  friend typename T::Impl& __detail::unwrap(T&);

 public:
  Executor(Context& ctx);
  ~Executor();

 public:
  size_t id() const;
  operator size_t() const { return id(); }

 private:
  class Impl;
  std::unique_ptr<Impl> _impl;
};

class Goroutine {
  friend class Executor;
  friend class Context;

  template <typename T>
  friend typename T::Impl& __detail::unwrap(T&);

 public:
  Goroutine(Context& ctx, AsyncFunctionBase&& fn);
  ~Goroutine();

 public:
  size_t id() const;
  operator size_t() const { return id(); }

 private:
  class Impl;
  std::unique_ptr<Impl> _impl;
};

}  // namespace cppgo

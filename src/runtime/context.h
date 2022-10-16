#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include "coroutine/functional.h"
#include "util/nocopy_any.h"
#include "util/unwrapper.h"

namespace cppgo {

class Goroutine;

class Context {
  template <__detail::HasImpl T>
  friend typename T::Impl& __detail::impl(T& wrapper);

 public:
  Context(size_t executor_num);
  ~Context();

 public:
  Goroutine& go(AsyncFunctionBase&& fn);
  Goroutine& current_goroutine();
  void start();
  void wait_until(const std::function<bool()>& pred);
  void stop();

 public:
  // should be called before start()
  template <typename T>
  void add(T&& value) {
    _variables.emplace(typeid(T).hash_code(), NoCopyAny(std::move(value)));
  }

  template <typename T>
  T& get() {
    return _variables.at(typeid(T).hash_code()).cast<T>();
  }

 public:
  class Impl;

 private:
  std::unique_ptr<Impl> _impl;
  std::unordered_map<size_t, NoCopyAny> _variables;
};

}  // namespace cppgo
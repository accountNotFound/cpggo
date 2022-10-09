#pragma once

#include <any>
#include <memory>

#include "async/functional.h"
#include "runtime.h"

#define USE_DEBUG
#include "util/log.h"

namespace cppgo {

class Mutex {
 public:
  Mutex(Context& ctx);
  Mutex(const Mutex& rhs) = default;
  ~Mutex();

 public:
  AsyncFunction<void> lock();
  void unlock();

 private:
  class Impl;
  std::shared_ptr<Impl> _impl;
};

class ChannelBase {
 public:
  ChannelBase(Context& ctx, size_t capacity);
  ChannelBase(const ChannelBase& rhs) = default;
  virtual ~ChannelBase();

 public:
  AsyncFunction<void> send_any(std::any&& value);
  AsyncFunction<std::any> recv_any();

 private:
  class Impl;
  std::shared_ptr<Impl> _impl;
};

template <typename T>
class Channel : public ChannelBase {
 public:
  Channel(Context& ctx, size_t capacity) : ChannelBase(ctx, capacity) {}

 public:
  AsyncFunction<void> send(T&& value) { co_await this->send_any(std::move(value)); }
  AsyncFunction<T> recv() {
    std::any res = co_await this->recv_any();
    if constexpr (std::is_same_v<T, std::any>) {
      co_return std::move(res);
    } else {
      co_return std::any_cast<T>(std::move(res));
    }
  }
};

}  // namespace cppgo

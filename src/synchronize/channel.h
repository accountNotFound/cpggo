#pragma once

#include <queue>

#include "common/spinlock.h"
#include "context/context.h"
#include "context/monitor.h"
#include "context/task.h"
#include "coroutine/coroutine.h"

// #define USE_DEBUG
#include "common/log.h"

namespace cppgo {

template <typename T>
class Channel {
 public:
  Channel(Context* ctx, size_t capacity)
      : ctx_(ctx),
        capacity_(capacity),
        resource_(1),
        r_mon_(ctx_, &resource_),
        w_mon_(ctx_, &resource_) {}

  AsyncFunction<void> send(T&& data) {
    while (true) {
      resource_.lock();
      if (buffer_.size() < capacity_) {
        buffer_.push(std::move(data));
        r_mon_.notify_one_with_guard();
        if (buffer_.size() < capacity_) {
          w_mon_.notify_one_with_guard();
        }
        DEBUG("task {%u} write finish", ctx_->this_running_task()->id());
        resource_.unlock();
        break;
      }
      r_mon_.notify_one_with_guard();
      DEBUG("task {%u} write fail, wait", ctx_->this_running_task()->id());
      co_await w_mon_.suspend_with_guard_unlock();
    }
  }

  AsyncFunction<T> recv() {
    T res;
    while (true) {
      resource_.lock();
      if (buffer_.size() > 0) {
        res = std::move(buffer_.front());
        buffer_.pop();
        w_mon_.notify_one_with_guard();
        if (buffer_.size() > 0) {
          r_mon_.notify_one_with_guard();
        }
        DEBUG("task {%u} read finish", ctx_->this_running_task()->id());
        resource_.unlock();
        break;
      }
      w_mon_.notify_one_with_guard();
      DEBUG("task {%u} read fail, wait", ctx_->this_running_task()->id());
      co_await r_mon_.suspend_with_guard_unlock();
    }
    co_return std::move(res);
  }

  bool send_noblock(T&& data) {
    auto current = ctx_->this_running_task();
    resource_.lock();
    if (buffer_.size() < capacity_) {
      buffer_.push(std::move(data));
      r_mon_.notify_one_with_guard();
      if (buffer_.size() < capacity_) {
        w_mon_.notify_one_with_guard();
      }
      DEBUG("task {%u} write finish", current ? current->id() : -1);
      resource_.unlock();
      return true;
    }
    r_mon_.notify_one_with_guard();
    DEBUG("task {%u} write fail, wait", current ? current->id() : -1);
    resource_.unlock();
    return false;
  }

 private:
  Context* ctx_;
  std::queue<T> buffer_;
  size_t capacity_;
  Resource resource_;
  Monitor r_mon_, w_mon_;
};

}  // namespace cppgo

#include <queue>

// #define USE_DEBUG
#include "common/log.h"
#include "common/spinlock.h"
#include "context/context.h"
#include "context/monitor.h"
#include "context/task.h"
#include "coroutine/coroutine.h"

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

  AsyncFunction<void> send(const T& data) {
    while (true) {
      {
        resource_.lock();
        if (buffer_.size() < capacity_) {
          buffer_.push(data);
          r_mon_.notify_one_with_guard();
          w_mon_.notify_one_with_guard();
          DEBUG("task {%u} write finish", ctx_->this_running_task()->id());
          resource_.unlock();
          break;
        }
        
        r_mon_.notify_one_with_guard();
        DEBUG("task {%u} write fail, wait", ctx_->this_running_task()->id());
        co_await w_mon_.suspend_with_guard_unlock();
      }
    }

    // implementation below has some bugs

    // co_await w_mon_.enter();
    // DEBUG("task {%u} start write", ctx_->this_running_task()->id());
    // while (true) {
    //   if (buffer_.size() < capacity_) {
    //     buffer_.push(data);
    //     r_mon_.notify_one();
    //     w_mon_.notify_one();
    //     DEBUG("task {%u} write finish", ctx_->this_running_task()->id());
    //     break;
    //   }
    //   DEBUG("task {%u} write fail, wait", ctx_->this_running_task()->id());
    //   r_mon_.notify_one();
    //   co_await w_mon_.wait();
    // }
    // w_mon_.exit();
  }

  AsyncFunction<T> recv() {
    T res;
    while (true) {
      {
        resource_.lock();
        if (buffer_.size() > 0) {
          res = std::move(buffer_.front());
          buffer_.pop();
          r_mon_.notify_one_with_guard();
          w_mon_.notify_one_with_guard();
          DEBUG("task {%u} read finish", ctx_->this_running_task()->id());
          resource_.unlock();
          break;
        }
        w_mon_.notify_one_with_guard();
        DEBUG("task {%u} read fail, wait", ctx_->this_running_task()->id());
        co_await r_mon_.suspend_with_guard_unlock();
      }
    }
    co_return std::move(res);

    // implementation below has some bugs

    // T res;
    // co_await r_mon_.enter();
    // DEBUG("task {%u} start read", ctx_->this_running_task()->id());
    // while (true) {
    //   if (buffer_.size() > 0) {
    //     res = std::move(buffer_.front());
    //     buffer_.pop();
    //     r_mon_.notify_one();
    //     w_mon_.notify_one();
    //     DEBUG("task {%u} read finish", ctx_->this_running_task()->id());
    //     break;
    //   }
    //   DEBUG("task {%u} read fail, wait", ctx_->this_running_task()->id());
    //   w_mon_.notify_one();
    //   co_await r_mon_.wait();
    // }
    // r_mon_.exit();
  }

 private:
  Context* ctx_;
  std::queue<T> buffer_;
  size_t capacity_;
  Resource resource_;
  Monitor r_mon_, w_mon_;
};

}  // namespace cppgo

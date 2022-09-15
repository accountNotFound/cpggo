#include <queue>

// #define USE_DEBUG
#include "common/log.h"
#include "common/spinlock.h"
#include "context/context.h"
#include "coroutine/coroutine.h"

namespace cppgo {

template <typename T, size_t N = 1>
class Channel {
 public:
  Channel(Context* ctx) : ctx_(ctx), reader_(ctx), writer_(ctx) {}

  AsyncFunction<void> send(const T& data) {
    auto current = ctx_->this_running_task();
    while (true) {
      bool ok = false;
      {
        std::unique_lock guard(mtx_);
        if (buffer_.size() < N) {
          buffer_.push(data);
          ok = true;
        }
      }
      reader_.notify_one();
      if (ok) {
        DEBUG("task {%u} write finish", current->id());
        co_return;
      } else {
        DEBUG("task {%u} write fail, hang up", current->id());
        co_await writer_.await();
      }
    }
  }

  AsyncFunction<T> recv() {
    T res;
    auto current = ctx_->this_running_task();
    while (true) {
      bool ok = false;
      {
        std::unique_lock guard(mtx_);
        if (!buffer_.empty()) {
          res = std::move(buffer_.front());
          buffer_.pop();
          ok = true;
        }
      }
      writer_.notify_one();
      if (ok) {
        DEBUG("task {%u} read finish", current->id());
        co_return std::move(res);
      } else {
        DEBUG("task {%u} read fail, hang up", current->id());
        co_await reader_.await();
      }
    }
  }

 private:
  Context* ctx_;
  SpinLock mtx_;
  HangupCtrl reader_;
  HangupCtrl writer_;
  std::queue<T> buffer_;
};

}  // namespace cppgo

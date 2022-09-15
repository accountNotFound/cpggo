#include <array>

#define USE_DEBUG
#include "common/log.h"
#include "common/spinlock.h"
#include "context/context.h"

namespace cppgo {

template <typename T, size_t N = 1>
class Channel {
 public:
  Channel(Context* ctx)
      : ctx_(ctx),
        r_mon_(Monitor(ctx)),
        w_mon_(Monitor(ctx)),
        r_idx_(0),
        w_idx_(0) {
    for (int i = 0; i < N; i++) {
      buffer_[i] = nullptr;
    }
  }
  Channel(Channel&) = delete;

  AsyncFunction<void> send(T&& data) {
    co_await w_mon_.enter();
    while (true) {
      bool inserted = false;
      {
        std::unique_lock guard(mtx_);
        if (!buffer_[w_idx_]) {
          buffer_[w_idx_] = new T(std::move(data));
          w_idx_ = (w_idx_ + 1) % N;
          inserted = true;
        }
        r_mon_.notify_one();
      }
      if (inserted) {
        break;
      } else {
        co_await w_mon_.wait();
      }
    }
    w_mon_.exit();
  }

  AsyncFunction<T> recv() {
    T data;
    co_await r_mon_.enter();
    while (true) {
      bool got = false;
      {
        std::unique_lock guard(mtx_);
        if (buffer_[r_idx_]) {
          data = std::move(*buffer_[r_idx_]);
          delete buffer_[r_idx_];
          buffer_[r_idx_] = nullptr;
          r_idx_ = (r_idx_ + 1) % N;
          got = true;
        }
        w_mon_.notify_one();
      }
      if (got) {
        break;
      } else {
        co_await r_mon_.wait();
      }
    }
    r_mon_.exit();
    co_return std::move(data);
  }

 private:
  Context* ctx_;
  SpinLock mtx_;
  std::array<T*, N> buffer_;
  Monitor r_mon_;
  Monitor w_mon_;
  size_t r_idx_;
  size_t w_idx_;
};

}  // namespace cppgo

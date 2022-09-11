#include <array>

#include "context/context.h"

namespace cppgo {

template <typename T, size_t N = 1>
class Channel {
 public:
  Channel() = default;
  Channel(Channel&) = delete;

  AsyncFunction<void> send(T&& data) {
    while (true) {
      co_await w_monitor_.enter();
      if (!buffer_[w_idx_]) {
        buffer_[w_idx_] = new T(std::forward<T>(data));
        w_idx_ = (w_idx_ + 1) % N;
        w_monitor_.exit_nowait();
        co_return;
      }
      // TODO: notify reader monitor
      co_await w_monitor_.wait();
    }
  }

  AsyncFunction<T> recv() {}

 private:
  std::array<T*, N> buffer_;
  Monitor r_monitor_;
  Monitor w_monitor_;
  size_t r_idx_ = 0;
  size_t w_idx_ = 0;
};

}  // namespace cppgo

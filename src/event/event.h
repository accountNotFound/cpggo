#include "runtime/context.h"
#include "synchronize/channel.h"

namespace cppgo {

enum Event { IN = 0x1, OUT = 0x2, ERR = 0x4, ONESHOT = 0x8 };

extern Event operator|(Event a, Event b);

struct Fd {
 public:
  Fd() = default;
  Fd(size_t opened_fd) : _fd(opened_fd) {}

  size_t id() const { return _fd; }
  operator size_t() const { return _fd; }
  bool operator==(const Fd& rhs) const { return _fd == rhs._fd; }

 private:
  size_t _fd = -1;
};

struct Signal : public Fd {
 public:
  Signal(Context& ctx, const Fd& fd) : Fd(fd), _chan(ctx, 1) {}

  AsyncFunction<void> wait() { co_await _chan.recv(); }
  void notify() { _chan.send_noblock(true); }

 private:
  Channel<bool> _chan;
};

}  // namespace cppgo

template <typename T>
requires std::convertible_to<T, cppgo::Fd>
struct std::hash<T> {
 public:
  size_t operator()(const T& fd) const { return size_t(fd); }
};
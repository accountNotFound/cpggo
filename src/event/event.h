#pragma once

#include "runtime/context.h"
#include "synchronize/channel.h"

namespace cppgo {

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

class Event {
 public:
  enum Type { IN = 0x1, OUT = 0x2, ERR = 0x4, ONESHOT = 0x8 };

 public:
  Event() = default;
  Event(Context& ctx, Fd fd, Type type);
  ~Event();

 public:
  Fd fd();
  Type type();
  Channel<bool>& chan();
  AsyncFunction<void> wait();

 private:
  class Impl;
  std::shared_ptr<Impl> _impl;
};

extern Event::Type operator|(Event::Type a, Event::Type b);

}  // namespace cppgo

template <typename T>
requires std::convertible_to<T, cppgo::Fd>
struct std::hash<T> {
 public:
  size_t operator()(const T& fd) const { return size_t(fd); }
};
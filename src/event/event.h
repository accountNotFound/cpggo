#pragma once

#include "runtime/context.h"
#include "synchronize/channel.h"

namespace cppgo {

using Fd = size_t;

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
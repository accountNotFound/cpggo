#pragma once

#include "context/context.h"

namespace cppgo {

enum Event { IN = 0x1, OUT = 0x2, ERR = 0x4, ONESHOT = 0x8 };

// Event operator|(Event a, Event b) {
//   return static_cast<Event>(static_cast<size_t>(a) | static_cast<size_t>(b));
// }
// Event operator|=(Event a, Event b) {
//   return static_cast<Event>(static_cast<size_t>(a) | static_cast<size_t>(b));
// }

struct Fd {
 public:
  struct Equal {
    size_t operator()(const Fd& a, const Fd& b) const { return a.uid == b.uid; }
  };
  struct Hash {
    size_t operator()(const Fd& fd) const { return fd.uid; }
  };

 public:
  Fd() = default;
  Fd(size_t opened_fd) : uid(opened_fd) {}

  virtual operator size_t() const { return uid; }

 public:
  size_t uid;
};

class EventContext : public Context {
 public:
  EventContext(size_t thread_num) : Context(thread_num) {}

  virtual void add(const Fd& fd, Event listen_on) {}
  virtual void mod(const Fd& fd, Event listen_on) {}
  virtual void del(const Fd& fd) {}
  // co_return when fd is ready and selected
  virtual AsyncFunction<void> signal(const Fd& fd) = 0;
  virtual void evloop() = 0;
};

}  // namespace cppgo

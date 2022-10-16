#pragma once

#include <functional>
#include <memory>

#include "coroutine/functional.h"
#include "event.h"
#include "runtime/context.h"

namespace cppgo {

class EventHandler {
  friend class Context;

 public:
  EventHandler(Context& ctx);
  EventHandler(const EventHandler& rhs) = delete;
  EventHandler(EventHandler&& rhs);
  ~EventHandler();

  void add(const Fd& fd, Event listen_on);
  void mod(const Fd& fd, Event listen_on);
  void del(const Fd& fd);

  AsyncFunction<void> wait(const Fd& fd);
  void loop_until(const std::function<bool()>& pred);

 private:
  class Impl;
  std::unique_ptr<Impl> _impl;
};

}  // namespace cppgo

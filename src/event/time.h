#pragma once

#include "handler.h"
#include "synchronize/channel.h"

namespace cppgo {

class Timer {
 public:
  Timer(Context& ctx, unsigned long long millisec);
  ~Timer();
  AsyncFunction<void> wait();
  Channel<bool>& chan();

 private:
  class Impl;
  std::shared_ptr<Impl> _impl;
};

AsyncFunction<void> sleep(Context& ctx, unsigned long long millisec);

}  // namespace cppgo

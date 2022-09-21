#include "context/common.h"
#include "context/monitor.h"

namespace cppgo {

class Mutex {
 public:
  Mutex(Context* ctx) : ctx_(ctx), resource_(1), monitor_(ctx_, &resource_) {}

  AsyncFunction<void> lock() { co_await monitor_.enter(); }
  void unlock() { monitor_.exit(); }

 private:
  Context* ctx_;
  Resource resource_;
  Monitor monitor_;
};

}  // namespace cppgo

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

/* NOTE: is it necessary to implement a condition variable class ?
 *
 * Current monitor implementation supports following operations:
 * ...
 * co_await monitor.enter();
 * ...
 * monitor.notify_one();
 * ...
 * monitor.wait();
 * ...
 *
 * However, notification between multi monitors on different resources may occur
 * infinite waiting since there is no mechanism checking idle monitor/resource.
 *
 * On the other hand, Channel is implemented for multi coroutine/resource
 * synchronization, which is equivalent to condition variable.
 * See "src/sychronize/channel.h" to get more detail.
 */
class ConditionVariable {};

}  // namespace cppgo

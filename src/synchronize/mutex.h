#include "common/spinlock.h"
#include "context/context.h"

namespace cppgo {

class Mutex {
 public:
  Mutex(Context* ctx) : ctx_(ctx), hangup_ctrl_(ctx) {}

  AsyncFunction<void> lock();
  void unlock();

 protected:
  Context* ctx_;
  SpinLock mtx_;
  HangupCtrl hangup_ctrl_;
  bool idle_ = true;
};

class ConditionVariable : public Mutex {
 public:
  ConditionVariable(Context* ctx) : Mutex(ctx) {}

  AsyncFunction<void> wait();
  void notify_one();
};

/* since co_await operator can't be placed in constructor, you have to lock the
 * given mutex manually, for example:
 *
 * Mutex mtx();
 * ...
 * co_await mtx.lock();
 * LockGuard guard(mtx);
 * ...
 *
 * then, you don't need to call mtx.unlock() explicitly anymore
 */
class LockGuard {
 public:
  LockGuard(Mutex& mutex) : mutex_(&mutex) {}
  ~LockGuard() { mutex_->unlock(); }

 private:
  Mutex* mutex_;
};

}  // namespace cppgo

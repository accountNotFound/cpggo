#include "context/context.h"

namespace cppgo {

class Mutex {
 public:
  Mutex(Context* ctx) : monitor_(ctx) {}

  AsyncFunction<void> lock() { co_await monitor_.enter(); }
  AsyncFunction<void> unlock() { co_await monitor_.exit(); }
  void unlock_nowait() { monitor_.exit_nowait(); }

 protected:
  Monitor monitor_;
};

class ConditionVariable : public Mutex {
 public:
  ConditionVariable(Context* ctx) : Mutex(ctx) {}

  AsyncFunction<void> wait() { co_await monitor_.wait(); }
  void notify_one() { monitor_.notify_one(); }
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
  ~LockGuard() { mutex_->unlock_nowait(); }

 private:
  Mutex* mutex_;
};

}  // namespace cppgo

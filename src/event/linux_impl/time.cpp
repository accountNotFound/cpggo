#include "../time.h"

#include "sys/timerfd.h"

namespace cppgo {

class Timer::Impl {
 public:
  Impl(Context& ctx, unsigned long long millisec) : _ev_hdlr(&ctx.get<EventHandler>()), _chan(ctx, 1) {
    _fd = Fd(timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK));
    itimerspec ts;
    ts.it_interval = timespec{0, 0};
    ts.it_value = timespec{(long)millisec / 1000, ((long)millisec % 1000) * 1000000};
    timerfd_settime(size_t(_fd), 0, &ts, nullptr);
    _ev_hdlr->add(_fd, Event::IN);
  }

  AsyncFunction<void> wait() {
    co_await _ev_hdlr->wait(_fd);
    _chan.send_noblock(true);
  }

  Channel<bool>& chan() { return _chan; }

 private:
  EventHandler* _ev_hdlr;
  Fd _fd;
  Channel<bool> _chan;
};

AsyncFunction<void> sleep(Context& ctx, unsigned long long millisec) { co_await Timer(ctx, millisec).wait(); }

Timer::Timer(Context& ctx, unsigned long long millisec) : _impl(std::make_unique<Impl>(ctx, millisec)) {}

Timer::~Timer() = default;

AsyncFunction<void> Timer::wait() { co_await _impl->wait(); }

Channel<bool>& Timer::chan() { return _impl->chan(); }

}  // namespace cppgo

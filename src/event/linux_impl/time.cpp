#include "../time.h"

#include "sys/timerfd.h"

namespace cppgo {

class Timer::Impl {
 public:
  Impl(Context& ctx, unsigned long long millisec) : _ev_hdlr(&ctx.get<EventHandler>()) {
    _fd = Fd(timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK));
    itimerspec ts;
    ts.it_interval = timespec{0, 0};
    ts.it_value = timespec{(long)millisec / 1000, ((long)millisec % 1000) * 1000000};
    timerfd_settime(size_t(_fd), 0, &ts, nullptr);

    _event = Event(ctx, _fd, Event::IN | Event::ONESHOT);
    _ev_hdlr->add(_fd, _event);
  }

  AsyncFunction<void> wait() { co_await _event.wait(); }

  Channel<bool>& chan() { return _event.chan(); }

 private:
  EventHandler* _ev_hdlr;
  Fd _fd;
  Event _event;
};

AsyncFunction<void> sleep(Context& ctx, unsigned long long millisec) { co_await Timer(ctx, millisec).wait(); }

Timer::Timer(Context& ctx, unsigned long long millisec) : _impl(std::make_unique<Impl>(ctx, millisec)) {}

Timer::~Timer() = default;

AsyncFunction<void> Timer::wait() { co_await _impl->wait(); }

Channel<bool>& Timer::chan() { return _impl->chan(); }

}  // namespace cppgo

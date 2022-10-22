#include "../event.h"

#include <sys/epoll.h>
#include <unistd.h>

namespace cppgo {

class Event::Impl {
  friend class Event;

 public:
  Impl(Context& ctx, Fd fd, Type type) : _fd(fd), _type(type), _chan(ctx, 1) {}

  AsyncFunction<void> wait() { co_await _chan.recv(); }

 private:
  Fd _fd;
  Type _type;
  Channel<bool> _chan;
};

Event::Event(Context& ctx, Fd fd, Type type) : _impl(std::make_shared<Impl>(ctx, fd, type)) {}

Event::~Event() = default;

Fd Event::fd() { return _impl->_fd; }

Event::Type Event::type() { return _impl->_type; }

Channel<bool>& Event::chan() { return _impl->_chan; }

AsyncFunction<void> Event::wait() { co_await _impl->wait(); }

Event::Type operator|(Event::Type a, Event::Type b) {
  return static_cast<Event::Type>(static_cast<size_t>(a) | static_cast<size_t>(b));
}

}  // namespace cppgo

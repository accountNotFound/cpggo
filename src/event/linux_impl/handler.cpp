#include "../handler.h"

#include <sys/epoll.h>
#include <unistd.h>

#include <array>

#include "../event.h"
#include "runtime/context.h"
#include "util/unwrapper.h"

#define EPOLL_CAPACITY 2048
#define EPOLL_WAIT_MAX_SIZE 64
#define LOOP_INTERVAL 100

namespace cppgo {

class EventHandler::Impl {
 public:
  Impl(Context& ctx) : _ctx(&ctx) { _epoll_fd = epoll_create(EPOLL_CAPACITY); }
  ~Impl() { close(_epoll_fd); }

  void add(Fd fd, Event& event) {
    epoll_event listen;
    listen.events = _to_epoll(event);
    listen.data.ptr = &event;
    epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, size_t(fd), &listen);
  }

  void mod(Fd fd, Event& event) {
    epoll_event listen;
    listen.events = _to_epoll(event);
    listen.data.ptr = &event;
    epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, size_t(fd), &listen);
  }

  void del(Fd fd) { epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, size_t(fd), nullptr); }

  void loop_until(const std::function<bool()>& pred) {
    std::array<epoll_event, EPOLL_WAIT_MAX_SIZE> buffer;
    while (!pred()) {
      size_t active_num = epoll_wait(_epoll_fd, buffer.data(), buffer.size(), LOOP_INTERVAL);
      for (int i = 0; i < active_num; ++i) static_cast<Event*>(buffer[i].data.ptr)->chan().send_noblock(true);
    }
  }

 private:
  size_t _to_epoll(Event& event) {
    size_t res = 0;
    if (event.type() | Event::Type::IN) res |= EPOLLIN;
    if (event.type() | Event::Type::OUT) res |= EPOLLOUT;
    if (event.type() | Event::Type::ERR) res |= EPOLLERR;
    if (event.type() | Event::Type::ONESHOT) res |= EPOLLONESHOT;
    return res;
  }

 private:
  Context* _ctx;
  size_t _epoll_fd;
};

EventHandler::EventHandler(Context& ctx) : _impl(std::make_unique<Impl>(ctx)) {}

EventHandler::EventHandler(EventHandler&& rhs) = default;

EventHandler::~EventHandler() = default;

void EventHandler::add(Fd fd, Event& event) { _impl->add(fd, event); }

void EventHandler::mod(Fd fd, Event& event) { _impl->mod(fd, event); }

void EventHandler::del(Fd fd) { _impl->del(fd); }

void EventHandler::loop_until(const std::function<bool()>& pred) { _impl->loop_until(pred); }

}  // namespace cppgo

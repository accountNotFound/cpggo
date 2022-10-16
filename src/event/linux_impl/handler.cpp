#include "../handler.h"

#include <sys/epoll.h>
#include <unistd.h>

#include <array>

#include "runtime/context.h"
#include "util/lock_free_hashset.h"
#include "util/unwrapper.h"

#define EPOLL_CAPACITY 2048
#define EPOLL_WAIT_MAX_SIZE 128
#define LOOP_INTERVAL 100

namespace cppgo {

class EventHandler::Impl {
 public:
  Impl(Context& ctx) : _ctx(&ctx) { _epoll_fd = epoll_create(EPOLL_CAPACITY); }
  ~Impl() { close(_epoll_fd); }

  void add(const Fd& fd, Event listen_on) {
    _signals.emplace(*_ctx, fd);
    epoll_event listen;
    listen.data.fd = size_t(fd);
    listen.events = _as_epoll_event(listen_on);
    epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, size_t(fd), &listen);
  }

  void mod(const Fd& fd, Event listen_on) {
    epoll_event listen;
    listen.data.fd = size_t(fd);
    listen.events = _as_epoll_event(listen_on);
    epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, size_t(fd), &listen);
  }

  void del(const Fd& fd) {
    _signals.erase(size_t(fd));
    epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, size_t(fd), nullptr);
  }

  AsyncFunction<bool> wait(const Fd& fd) {
    co_await _signals.at(size_t(fd)).wait();
    _signals.erase(size_t(fd));
    // oneshot, no need to del fd
  }

  void loop_until(const std::function<bool()>& pred) {
    std::array<epoll_event, EPOLL_WAIT_MAX_SIZE> buffer;
    while (!pred()) {
      size_t active_num = epoll_wait(_epoll_fd, buffer.data(), buffer.size(), LOOP_INTERVAL);
      for (int i = 0; i < active_num; ++i) _signals.at(buffer[i].data.fd).notify();
    }
  }

 private:
  size_t _as_epoll_event(Event event) {
    // oneshot is always enable
    size_t res = 0;
    res |= EPOLLONESHOT;
    if (event | Event::IN) res |= EPOLLIN;
    if (event | Event::OUT) res |= EPOLLOUT;
    if (event | Event::ERR) res |= EPOLLERR;
    return res;
  }

 private:
  Context* _ctx;
  size_t _epoll_fd;
  LockFreeHashset<Signal> _signals;
};

EventHandler::EventHandler(Context& ctx) : _impl(std::make_unique<Impl>(ctx)) {}

EventHandler::EventHandler(EventHandler&& rhs) = default;

EventHandler::~EventHandler() = default;

void EventHandler::add(const Fd& fd, Event listen_on) { _impl->add(fd, listen_on); }

void EventHandler::mod(const Fd& fd, Event listen_on) { _impl->mod(fd, listen_on); }

void EventHandler::del(const Fd& fd) { _impl->del(fd); }

AsyncFunction<void> EventHandler::wait(const Fd& fd) { co_await _impl->wait(fd); }

void EventHandler::loop_until(const std::function<bool()>& pred) { _impl->loop_until(pred); }

}  // namespace cppgo

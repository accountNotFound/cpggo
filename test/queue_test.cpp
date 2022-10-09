#include <stdio.h>

#include <thread>
#include <vector>

#include "util/lock_free_queue.h"

using namespace cppgo;

int thread_num = 10;
int loop_num = 30000;

LockFreeQueue<std::string> que;

int value = 0;

void work() {
  for (int i = 0; i < loop_num; i++) {
    que.enqueue("xxx");
    // printf("enqueue xxx\n");
  }
}

int main() {
  std::vector<std::thread> workers;
  for (int i = 0; i < thread_num; i++) {
    workers.emplace_back(std::thread(work));
  }
  for (auto& w : workers) {
    w.join();
  }
  while (true) {
    auto [_, ok] = que.dequeue();
    // printf("dequeue %d\n", ok);
    if (!ok) break;
    ++value;
  }
  printf("value=%d\n", value);
  if (value != thread_num * loop_num) {
    return -1;
  }
  return 0;
}
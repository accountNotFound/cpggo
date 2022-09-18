#include "common/spinlock.h"

#include <stdio.h>

#include <thread>
#include <vector>

using namespace cppgo;

int thread_num = 10;
int loop_num = 30000;

SpinLock lock;

int value = 0;

void work() {
  for (int i = 0; i < loop_num; i++) {
    std::unique_lock guard(lock);
    value++;
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
  printf("value=%d\n", value);
  if (value != thread_num * loop_num) {
    return -1;
  }
  return 0;
}
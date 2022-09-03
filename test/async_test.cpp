#include "coroutine/coroutine.h"

#include <string>

#include "stdio.h"

cppgo::AsyncFunction<int> bar(int n) {
  int res = 0;
  for (int i = 0; i < n; i++) {
    printf("   bar %d\n", i);
    co_await std::suspend_always{};
    res += i + 1;
  }
  co_return res;
}

cppgo::AsyncFunction<std::string> foo(int n) {
  std::string res = "";
  for (int i = 0; i < n; i++) {
    printf("  foo %d\n", i);
    res += std::to_string(co_await bar(i)) + " ";
  }
  co_return res;
}

cppgo::AsyncFunction<void> biz() {
  printf("biz start\n");
  auto res = co_await foo(5);
  printf("\"%s\"\n", res.data());
  printf("biz end\n");
}

int main() {
  auto e = biz();
  for (e.start(); !e.done(); e.resume()) {
    printf("main\n");
  }
  printf("async test end\n");
}
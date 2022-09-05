include_directories("../src")

add_executable(async_test async_test.cpp)

add_executable(context_test context_test.cpp)
target_link_libraries(context_test context)

add_executable(spinlock_test spinlock_test.cpp)


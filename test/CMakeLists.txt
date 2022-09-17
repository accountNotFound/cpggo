include_directories("../src")

add_executable(coroutine_test coroutine_test.cpp)

add_executable(spinlock_test spinlock_test.cpp)

add_executable(context_test context_test.cpp)
target_link_libraries(context_test context)

add_executable(channel_test channel_test.cpp)
target_link_libraries(channel_test context)
include_directories("../src")

add_executable(coroutine_test coroutine_test.cpp)
target_link_libraries(coroutine_test cppgo_async)

add_executable(spinlock_test spinlock_test.cpp)
target_link_libraries(spinlock_test cppgo_util)

add_executable(queue_test queue_test.cpp)
target_link_libraries(queue_test cppgo_util)

add_executable(mutex_test mutex_test.cpp)
target_link_libraries(mutex_test cppgo_async cppgo_runtime cppgo_synchronize cppgo_util)

add_executable(multi_channel_test multi_channel_test.cpp)
target_link_libraries(multi_channel_test cppgo_async cppgo_runtime cppgo_synchronize cppgo_util)
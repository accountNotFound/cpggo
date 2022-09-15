include_directories("../src")

add_executable(async_test async_test.cpp)

add_executable(spinlock_test spinlock_test.cpp)

add_executable(mutex_test mutex_test.cpp)
target_link_libraries(mutex_test context synchronize)

add_executable(channel_test channel_test.cpp)
target_link_libraries(channel_test context synchronize)
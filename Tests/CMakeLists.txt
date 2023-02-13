set(TARGET_NAME "run_tests")

file(GLOB TEST_SRC *.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(${TARGET_NAME} ${TEST_SRC})
include_directories(${PROJECT_SOURCE_DIR}/Inc)

target_link_libraries(${TARGET_NAME}
    PUBLIC
        ""
    PRIVATE
        Threads::Threads
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES}
        gmock
        gmock_main
)
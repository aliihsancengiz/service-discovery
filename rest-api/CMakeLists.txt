set(TARGET_NAME "sd-backend")

file(GLOB BACKEND_SRC *.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Drogon REQUIRED)

add_executable(${TARGET_NAME} ${BACKEND_SRC})
include_directories(${PROJECT_SOURCE_DIR}/Inc)

target_link_libraries(${TARGET_NAME}
    PUBLIC
        ""
    PRIVATE
		drogon
		trantor
)
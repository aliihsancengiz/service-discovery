
set(SERVICE_DISCOVERY_TARGET_NAME "service-discovery")
set(SERVICE_ANNOUNCE_TARGET_NAME "service-announcement")


add_executable(${SERVICE_ANNOUNCE_TARGET_NAME} service_announcement.cpp)
add_executable(${SERVICE_DISCOVERY_TARGET_NAME} service_discovery.cpp)

target_link_libraries(${SERVICE_ANNOUNCE_TARGET_NAME} -lpthread)
target_link_libraries(${SERVICE_DISCOVERY_TARGET_NAME} -lpthread)

target_include_directories(${SERVICE_ANNOUNCE_TARGET_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/Inc
    PRIVATE
        ""
)

target_include_directories(${SERVICE_DISCOVERY_TARGET_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/Inc
    PRIVATE
        ""
)
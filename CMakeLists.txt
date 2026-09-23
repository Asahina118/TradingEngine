cmake_minimum_required(VERSION 3.10)

project(HFT_learn)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_VERBOSE_MAKEFILE on)

file(GLOB_RECURSE EXCHANGE_FILES
    "utils/*.cpp"
    "exchange/*.cpp"
    "Common/*.cpp"
    "order_server/*.cpp"
)
add_executable(exchange_run ${EXCHANGE_FILES})

target_include_directories(exchange_run PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# list(APPEND LIBS libcommon)
# list(APPEND LIBS libexchange)
list(APPEND LIBS pthread)

# include_directories(${PROJECT_SOURCE_DIR})
# include_directories(${PROJECT_SOURCE_DIR}/exchange)

target_link_libraries(exchange_run PUBLIC ${LIBS})
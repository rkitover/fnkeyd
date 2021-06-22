cmake_minimum_required(VERSION 3.02)

project(fnkeyd)

include(FindPkgConfig)

pkg_check_modules(EVDEV REQUIRED libevdev)

add_executable(fnkeyd fnkeyd.c)

target_link_libraries(fnkeyd ${EVDEV_LIBRARIES})
target_include_directories(fnkeyd PRIVATE ${EVDEV_INCLUDE_DIRS})

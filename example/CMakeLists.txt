# minimum cmake version
cmake_minimum_required(VERSION 3.10)

# project name, version
project(libnm_example)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNM REQUIRED libnm)
pkg_check_modules(GLIB REQUIRED glib-2.0)

link_libraries( ${LIBNM_LIBRARIES} ${GLIB_LIBRARIES})
include_directories( ${LIBNM_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
add_compile_options( ${LIBNM_CFLAGS_OTHER} ${GLIB_CFLAGS_OTHER})

include_directories(
  include include/
)

add_executable(get_ipv4 get_ipv4.cpp)
add_executable(list_device device_type.cpp)
add_executable(modify_connection modify_connection.cpp)
add_executable(modify_password modify_password.cpp)
add_executable(add_wifi add_wifi_connection.cpp)
add_executable(add_ethernet add_ethernet_connection.cpp)
add_executable(get_password get_password.cpp)
add_executable(show_active show_active.cpp)
add_executable(update_ipv4 update_ipv4.cpp)

# minimum cmake version
cmake_minimum_required(VERSION 3.10)

# project name, version
project(libnm_example)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNM REQUIRED libnm)
pkg_check_modules(GLIB REQUIRED glib-2.0)
add_executable(nmcli wifi.cpp)

target_link_libraries( nmcli PRIVATE  ${LIBNM_LIBRARIES} ${GLIB_LIBRARIES})
target_include_directories( nmcli PRIVATE ${LIBNM_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
target_compile_options( nmcli PRIVATE ${LIBNM_CFLAGS_OTHER} ${GLIB_CFLAGS_OTHER})

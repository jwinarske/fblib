
cmake_minimum_required(VERSION 3.10.2)

project(fb-example LANGUAGES C)

add_executable(fbgrad fbgrad.c)

set_property(TARGET fbgrad PROPERTY C_STANDARD 99)

install(TARGETS fbgrad DESTINATION bin)


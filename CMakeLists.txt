cmake_minimum_required(VERSION 4.0)

find_package(cmake-bare REQUIRED PATHS node_modules/cmake-bare)
find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)

project(bare_yoga C CXX)

fetch_package("github:facebook/yoga@3.2.1")

add_bare_module(bare_yoga)

target_sources(
  ${bare_yoga}
  PRIVATE
    binding.c
)

target_include_directories(
  ${bare_yoga}
  PRIVATE
    $<TARGET_PROPERTY:yogacore,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(
  ${bare_yoga}
  PUBLIC
    yogacore
)

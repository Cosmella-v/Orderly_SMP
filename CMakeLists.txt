cmake_minimum_required(VERSION 3.15)
project(MinecraftProfileCreator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libs" FORCE)

include(FetchContent)

FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 1.9.2
)
FetchContent_MakeAvailable(cpr)

add_executable(MinecraftProfileCreator
    main.cpp
    downloader.cpp
    unzipper.cpp
    parser.cpp
)

target_link_libraries(MinecraftProfileCreator PRIVATE cpr::cpr)
target_include_directories(MinecraftProfileCreator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

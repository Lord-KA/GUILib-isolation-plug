cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fPIC" CACHE STRING "Comment" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -D NDEBUG" CACHE STRING "Comment" FORCE)
set(CMAKE_CXX_FLAGS_SANITIZER "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wformat=2 -fsanitize=address,undefined -g" CACHE STRING "Comment" FORCE)

# Enable debug symbols by default
# must be done before project() statement
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()
# (you can also set it on the command line: -D CMAKE_BUILD_TYPE=Release)

include(FetchContent)

FetchContent_Declare(
  qrexec
  GIT_REPOSITORY https://github.com/Lord-KA/qrexec-c-adapter
  GIT_TAG        master #TODO set release
)

FetchContent_GetProperties(qrexec)
if(NOT qrexec_POPULATED)
  FetchContent_Populate(qrexec)
  add_subdirectory(${CMAKE_BINARY_DIR}/_deps/qrexec-src)
  include_directories(${CMAKE_BINARY_DIR}/_deps/qrexec-src/include)
endif()

FetchContent_Declare(
  ESL
  GIT_REPOSITORY https://github.com/mishaglik/Elpidifor-s-legacy.git
  GIT_TAG        v2.0.0
)

FetchContent_GetProperties(ESL)
if(NOT ESL_POPULATED)
    FetchContent_Populate(ESL)
    include_directories(./build/_deps/esl-src)
endif()

include_directories(./shared)

# Define sources and executable
add_executable(server ./server/proxy-server.cpp)
add_library(client SHARED ./client/proxy-tool.cpp)

target_link_libraries(client PUBLIC qrexec-c-adapter)

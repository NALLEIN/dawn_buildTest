cmake_minimum_required(VERSION 3.10)
# set the project name and version
project(WEBGPU VERSION 1.0)

option(HAVE_WEBGPU "use webgpy flag" ON)
option(WITH_WEBGPU "Flag" ON)
if(HAVE_WEBGPU OR WITH_WEBGPU)
  # include(cmake/OpenCVDetectVulkan.cmake)
  set(WEBGPU_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")
  set(WEBGPU_LIBRARIES "${PROJECT_SOURCE_DIR}/lib")
endif()

add_executable(WEBGPU test.cpp ${PROJECT_SOURCE_DIR}/lib/webgpu_cpp.cpp)
set_property(TARGET WEBGPU PROPERTY CXX_STANDARD 11)
include_directories(${WEBGPU_INCLUDE_DIRS})
# target_include_directories(WEBGPU PUBLIC ${WEBGPU_INCLUDE_DIRS})
target_link_directories(WEBGPU PUBLIC ${WEBGPU_LIBRARIES})
target_link_libraries(WEBGPU PUBLIC dawn_proc dawn_native dawn_wire )

try_compile(VALID_WEBGPU
      "${PROJECT_SOURCE_DIR}/lib"
      "${PROJECT_SOURCE_DIR}/test.cpp"
      CMAKE_FLAGS "-DINCLUDE_DIRECTORIES:STRING=${WEBGPU_INCLUDE_DIRS}"
      OUTPUT_VARIABLE TRY_OUT
      )
if(NOT ${VALID_WEBGPU})
  message(WARNING "Can't use webgpu")
  return()
endif()
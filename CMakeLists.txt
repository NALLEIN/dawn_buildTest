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

add_executable(WEBGPU test.cpp)
set_property(TARGET WEBGPU PROPERTY CXX_STANDARD 11)
include_directories(${WEBGPU_INCLUDE_DIRS})
# link_directories(WEBGPU  ${WEBGPU_INCLUDE_DIRS}/dawn
#                                ${WEBGPU_INCLUDE_DIRS}/dawn)
target_include_directories(WEBGPU PUBLIC ${WEBGPU_INCLUDE_DIRS})
target_link_directories(WEBGPU PUBLIC ${WEBGPU_LIBRARIES})
target_link_libraries(WEBGPU dawn_proc dawn_native)

try_compile(VALID_DAWN
      ${WEBGPU_LIBRARIES}
      ${PROJECT_SOURCE_DIR}/test.cpp
      )
if(NOT ${VALID_DAWN})
  message(WARNING "Can't use Dawn")
  return()
endif()
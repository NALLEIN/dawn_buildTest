cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(WEBGPU VERSION 1.0)

option(HAVE_WEBGPU "use webgpy flga" ON)
option(WITH_WEBGPU "Flag" ON)
if(HAVE_WEBGPU OR WITH_WEBGPU)
  # include(cmake/OpenCVDetectVulkan.cmake)
  set(WEBGPU_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include" CACHE PATH "WEBGPU include directory")
  set(WEBGPU_LIBRARIES "${PROJECT_SOURCE_DIR}/lib")
endif()
include_directories("include")
#add_library(${WEBGPU_LIBRARIES})
add_executable(WEBGPU test.cpp)
target_link_libraries(WEBGPU PUBLIC "${PROJECT_SOURCE_DIR}/lib")
target_include_directories(WEBGPU PUBLIC 
                            "${PROJECT_BINARY_DIR}"
                            "${PROJECT_SOURCE_DIR}/include")
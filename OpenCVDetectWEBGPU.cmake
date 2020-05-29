include_directories("include")
add_library("lib")
set(WEBGPU_INCLUDE_DIRS "include/dawn" CACHE PATH "WEBGPU include directory")
set(WEBGPU_LIBRARIES "lib")

try_compile(VALID_WEBGPU
      "${WEBGPU_INCLUDE_DIRS}/lib"
      "test.cpp"
      CMAKE_FLAGS "-DINCLUDE_DIRECTORIES:STRING=${WEBGPU_INCLUDE_DIRS}"
      OUTPUT_VARIABLE TRY_OUT
      )
if(NOT ${VALID_WEBGPU})
  message(WARNING "Can't use WEBGPU")
  return()
endif()

set(HAVE_WEBGPU 1)

if(HAVE_WEBGPU)
  add_definitions(-DVK_NO_PROTOTYPES)
  include_directories(${WEBGPU_INCLUDE_DIRS})
endif()
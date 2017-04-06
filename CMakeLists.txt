# author: Viktor Richter vrichter@techfak.uni-bielefeld.de
# licence: MIT
project(fformation-rsb)
cmake_minimum_required(VERSION 2.8)

set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSIOM_PATCH 0)
set(PROJECT_NUMBER "v${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSIOM_PATCH}")

# add c++11 compiler flag
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DBOOST_SIGNALS_NO_DEPRECATION_WARNING")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()


find_package(Boost 1.54 COMPONENTS program_options REQUIRED)
find_package(fformation 2.1 REQUIRED)
find_package(RSB REQUIRED)
find_package(RCT REQUIRED)
find_package(RST REQUIRED sandbox deprecated)
find_package(RST_EXPERIMENTAL)
add_definitions(${RST_CONVERTERS_CFLAGS})
set(RSB_INCLUDE_DIRS
  ${RCT_INCLUDE_DIRS}
  ${RSB_INCLUDE_DIRS}
  ${RST_INCLUDE_DIRS}
  ${RST_EXPERIMENTAL_INCLUDE_DIRS}
  )
set(RSB_LIBRARY_DIRS
  ${RSB_RUNTIME_LIBRARY_DIRS}
  ${RCT_LIBRARY_DIRS}
  ${RST_RUNTIME_LIBRARY_DIRS}
  ${RST_EXPERIMENTAL_RUNTIME_LIBRARY_DIRS}
  )
set(RSB_LIBRARIES
  ${RCT_LIBRARIES}
  ${RST_LIBRARIES}
  ${RSTSANDBOX_LIBRARIES}
  ${RST_EXPERIMENTAL_LIBRARIES}
  ${RSB_LIBRARIES}
  )
set(RSB_DEFINITIONS
  ${RST_CFLAGS}
  ${RSTSANDBOX_CFLAGS}
  ${RST_EXPERIMENTAL_CFLAGS}
  )

# check if gco plugin is available
if(EXISTS "${PROJECT_SOURCE_DIR}/gco/CMakeLists.txt")
  add_subdirectory(gco)
  set(HAVE_GCO TRUE)
  set(GCO_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/gco/src/")
  set(GCO_LIBRARIES "fformation-gco" "gco")
  add_definitions(-DHAVE_GCO)
endif()

# get all header files
FILE(GLOB HEADERS "${PROJECT_SOURCE_DIR}/src/*.h")
# get all sources
FILE(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
# get all apps
FILE(GLOB APPS "${PROJECT_SOURCE_DIR}/app/*.cpp")

add_library("${PROJECT_NAME}" STATIC  ${SOURCES} ${HEADERS})

include_directories(
  "${PROJECT_SOURCE_DIR}/src"
  ${FFORMATION_INCLUDE_DIRS}
  ${GCO_INCLUDE_DIRS}
  ${RSB_INCLUDE_DIRS}
)

link_directories(
  ${Boost_LIBRARY_DIRS}
  ${FFORMATION_DIRS}
  ${RSB_LIBRARY_DIRS}
  )

add_definitions(${RSB_DEFINITIONS})

foreach(APP ${APPS})
  STRING(REGEX REPLACE "/.*/" "" APP ${APP})
  STRING(REGEX REPLACE "[.]cpp" "" APP ${APP})
  message(STATUS "-- Adding executable: ${APP}")
  add_executable("${PROJECT_NAME}-${APP}" "${PROJECT_SOURCE_DIR}/app/${APP}.cpp")
  target_link_libraries("${PROJECT_NAME}-${APP}"
    ${PROJECT_NAME}
    ${FFORMATION_LIBRARIES}
    ${Boost_LIBRARIES}
    ${GCO_LIBRARIES}
    ${RSB_LIBRARIES}
    )
  install(TARGETS "${PROJECT_NAME}-${APP}"
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE
    )
endforeach(APP)

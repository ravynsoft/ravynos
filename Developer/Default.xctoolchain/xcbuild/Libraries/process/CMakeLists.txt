#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

add_library(process
            Sources/Context.cpp
            Sources/DefaultContext.cpp
            Sources/MemoryContext.cpp
            Sources/Launcher.cpp
            Sources/DefaultLauncher.cpp
            Sources/MemoryLauncher.cpp
            Sources/User.cpp
            Sources/DefaultUser.cpp
            Sources/MemoryUser.cpp
            )

target_link_libraries(process PUBLIC ext util_xcbuild)
target_include_directories(process PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
install(TARGETS process DESTINATION usr/lib)

find_package(Threads REQUIRED)
target_link_libraries(process PRIVATE ${CMAKE_THREAD_LIBS_INIT})

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
  if ("${CMAKE_CXX_PLATFORM_ID}" STREQUAL "MinGW")
    target_link_libraries(process PRIVATE userenv shell32 advapi32)
  else ()
    target_link_libraries(process PRIVATE UserEnv shell32 AdvAPI32)
  endif ()
endif ()

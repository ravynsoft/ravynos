
set(DISPATCH_USE_SANITIZER "" CACHE STRING
    "Define the sanitizer used to build binaries and tests.")

if(CMAKE_SYSTEM_NAME STREQUAL Darwin AND DISPATCH_USE_SANITIZER)
  message(FATAL_ERROR "building libdispatch with sanitization is not supported on Darwin")
endif()

if(DISPATCH_USE_SANITIZER)
  # TODO(compnerd) ensure that the compiler supports these options before adding
  # them.  At the moment, assume that this will just be used with a GNU
  # compatible driver and that the options are spelt correctly in light of that.
  add_compile_options("-fno-omit-frame-pointer")
  if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_options("-O1")
  elseif(NOT CMAKE_BUILD_TYPE MATCHES "Debug" AND
         NOT CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    add_compile_options("-gline-tables-only")
  endif()

  if(LLVM_USE_SANITIZER STREQUAL "Address")
    add_compile_options("-fsanitize=address")
  elseif(DISPATCH_USE_SANITIZER MATCHES "Memory(WithOrigins)?")
    add_compile_options("-fsanitize=memory")
    if(DISPATCH_USE_SANITIZER STREQUAL "MemoryWithOrigins")
    add_compile_options("-fsanitize-memory-track-origins")
    endif()
  elseif(DISPATCH_USE_SANITIZER STREQUAL "Undefined")
    add_compile_options("-fsanitize=undefined")
    add_compile_options("-fno-sanitize=vptr,function")
    add_compile_options("-fno-sanitize-recover=all")
  elseif(DISPATCH_USE_SANITIZER STREQUAL "Thread")
    add_compile_options("-fsanitize=thread")
  elseif(DISPATCH_USE_SANITIZER STREQUAL "Address;Undefined" OR
         DISPATCH_USE_SANITIZER STREQUAL "Undefined;Address")
    add_compile_options("-fsanitize=address,undefined")
    add_compile_options("-fno-sanitize=vptr,function")
    add_compile_options("-fno-sanitize-recover=all")
  elseif(DISPATCH_USE_SANITIZER STREQUAL "Leaks")
    add_compile_options("-fsanitize=leak")
  else()
    message(FATAL_ERROR "unsupported value of DISPATCH_USE_SANITIZER: ${DISPATCH_USE_SANITIZER}")
  endif()
endif()

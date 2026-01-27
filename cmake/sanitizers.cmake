# TODO: turn into expression generators
# Use /RTCcsu on MSVC

function(enable_sanitizers)
  include(CheckCSourceCompiles)

  set(CMAKE_REQUIRED_LIBRARIES asan)
  check_c_source_compiles("int main() { return 0; }" HAVE_LIBASAN)

  set(CMAKE_REQUIRED_LIBRARIES ubsan)
  check_c_source_compiles("int main() { return 0; }" HAVE_LIBUBSAN)

  if(HAVE_LIBASAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
    message(STATUS "AddressSanitizer enabled")
  else()
    message(WARNING "AddressSanitizer not available")
  endif()

  if(HAVE_LIBUBSAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined")
    message(STATUS "UndefinedBehaviorSanitizer enabled")
  else()
    message(WARNING "UndefinedBehaviorSanitizer not available")
  endif()

  # Now update parent with the accumulated flags
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" PARENT_SCOPE)
endfunction()

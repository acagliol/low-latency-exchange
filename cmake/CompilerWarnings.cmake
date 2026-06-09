# call set_project_warnings(my_target) on every compiled target
# catches bad conversions and shadowed vars before they hit prod

function(set_project_warnings target)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wpedantic  # baseline strictness
      -Wconversion -Wshadow     # common latency bugs in trading code
    )
  elseif(MSVC)
    target_compile_options(${target} PRIVATE /W4)
  endif()
endfunction()

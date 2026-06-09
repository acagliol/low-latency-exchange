function(set_project_warnings target)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wpedantic -Wconversion -Wshadow
    )
  elseif(MSVC)
    target_compile_options(${target} PRIVATE /W4)
  endif()
endfunction()

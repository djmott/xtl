
add_custom_target(examples)

add_library(dummy_library_example SHARED dummy_library.cpp)
add_dependencies(dummy_library_example XTL)
add_dependencies(examples dummy_library_example)


function(build_example target)
  set(BUILD_${target}_EXAMPLE YES CACHE BOOLEAN "Build ${target}")
  if(BUILD_${target}_EXAMPLE)
    add_executable("example_${target}" EXCLUDE_FROM_ALL "example_${target}.cpp")
    add_dependencies("example_${target}" XTL)
    add_dependencies(examples "example_${target}")
  endif()
endfunction()

build_example(callback)
build_example(dynamic_library)
build_example(event_trace)
build_example(exception)
build_example(logging)
build_example(nlp)
build_example(parse1)
build_example(parse2)
build_example(string)

if(XTD_COMPILER MATCHES "XTD_COMPILER_GCC")
  target_compile_options(example_event_trace PRIVATE -finstrument-functions)
elseif(XTD_COMPILER MATCHES "XTD_COMPILER_MSVC")
  target_compile_options(example_event_trace PRIVATE /Gh /GH)
endif()

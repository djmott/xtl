
add_custom_target(examples)

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
build_example(parse1)
build_example(parse2)
build_example(string)

target_compile_options(example_event_trace PRIVATE -finstrument-functions)


option(XTL_BUILD_EVENT_TRACE "Build event trace library" ON)
if(XTL_BUILD_EVENT_TRACE)
  add_subdirectory(event_trace)
endif()

option(XTL_BUILD_CONFIG "Build the build config tool" ON)
if(XTL_BUILD_CONFIG)
  add_subdirectory(build_config)
endif()

add_executable(build_config
  xtl.cpp
  ${XTD_COM_HEADERS}
  ${XTD_CONCURRENT_HEADERS}
  ${XTD_GRAMMARS}
  ${XTD_NLP_HEADERS}
  ${XTD_WINDOWS_HEADERS}
  ${XTD_HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}
  )
target_link_libraries(build_config PRIVATE xtl)
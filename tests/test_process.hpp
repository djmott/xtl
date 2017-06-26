#pragma once

#include <xtd/process.hpp>

TEST(test_process, initialization){
  ASSERT_NO_THROW(xtd::process::this_process());
}

TEST(test_process, DISABLED_system_processes){
  ASSERT_NO_THROW(xtd::process::this_process());
  auto oSystemProcesses = xtd::process::system_processes();
  ASSERT_GT(oSystemProcesses.size(), static_cast<size_t>(1));
}

#if ((XTD_OS_LINUX | XTD_OS_WINDOWS) & XTD_OS)
TEST(test_process, DISABLED_enum_libraries){
  xtd::dynamic_library::map oLibs;
  ASSERT_NO_THROW(oLibs = xtd::process::this_process().libraries());
  ASSERT_GT(oLibs.size(), static_cast<size_t>(0));
}
#endif

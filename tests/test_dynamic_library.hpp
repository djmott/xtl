/** @file
xtd::dynamic_library system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <xtd/dynamic_library.hpp>
#include <xtd/executable.hpp>

#if 0
auto get_dummy_library = []() -> xtd::dynamic_library::pointer {
  auto sPath = xtd::executable::this_executable().path();
#if (XTD_OS_UNIX == XTD_OS)
  sPath.replace_filename("libdummy_library_test.so");
#elif (XTD_OS_MSYS == XTD_OS)
  sPath.replace_filename("msys-dummy_library_test.dll");
#elif (XTD_OS_MINGW == XTD_OS)
  sPath.replace_filename("libdummy_library_test.dll");
#elif (XTD_COMPILER_MSVC == XTD_COMPILER)
  sPath.replace_filename("dummy_library_test.dll");
#endif
    return xtd::dynamic_library::make(sPath);
};

TEST(test_dynamic_library, initialization) {
  xtd::dynamic_library::pointer oLib;
  ASSERT_NO_THROW(oLib = get_dummy_library());
  ASSERT_NE(oLib->handle(), nullptr);
}

TEST(test_dynamic_library, invocation){
  auto oLib = get_dummy_library();
  ASSERT_NE(oLib->handle(), nullptr);
  auto Echo = oLib->get<const char *, const char *>("Echo");
  auto Add = oLib->get<int, int, int>("Add");

  ASSERT_EQ(3, Add(1, 2));
  xtd::string s(Echo("hello"));
  ASSERT_EQ(static_cast<size_t>(5), s.size());
}
#endif


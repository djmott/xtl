/**
 * \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */


auto get_dummy_library = []() -> xtd::dynamic_library::ptr {
  auto sPath = xtd::this_executable::get_path();
#if (XTD_OS_UNIX == XTD_OS)
  sPath.replace_filename("libdummy.so");
#elif (XTD_OS_MINGW == XTD_OS)
  sPath.replace_filename("libdummy.dll");
#elif (XTD_COMPILER_MSVC == XTD_COMPILER)
  sPath.replace_filename("dummy.dll");
#endif
    return xtd::dynamic_library::make(sPath);
};

TEST(test_dynamic_library, initialization) {
  xtd::dynamic_library::ptr oLib;
  ASSERT_NO_THROW(oLib = get_dummy_library());
  ASSERT_NE(oLib->handle(), nullptr);
}

TEST(test_dynamic_library, Invocation){
  auto oLib = get_dummy_library();
  ASSERT_NE(oLib->handle(), nullptr);
  auto Echo = oLib->get<const char *, const char *>("Echo");
  auto Add = oLib->get<int, int, int>("Add");

  ASSERT_EQ(3, Add(1, 2));
  xtd::string s(Echo("hello"));
  ASSERT_EQ(5, s.size());
}



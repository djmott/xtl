/**
 * \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */


using namespace xtd;

TEST(test_var, initialization) {
  ASSERT_NO_THROW(var v1);
}

TEST(test_var, test_empty) {
  var v1;
  EXPECT_EQ(typeid(xtd::var::empty), v1.get_type());
  EXPECT_THROW(v1.is_pod(), std::runtime_error);
  EXPECT_THROW(v1.size(), std::runtime_error);
}

TEST(test_var, initialization_with_value) {
  ASSERT_NO_THROW(var v1(123));
  ASSERT_NO_THROW(var v2(3.14));
  ASSERT_NO_THROW(var v3("abc123"));
  ASSERT_NO_THROW(var v4(L"xyzpdq"));
}

TEST(test_var, assignments){
  var v1 = 123;
  ASSERT_EQ(v1.get_type(), typeid(int));
  ASSERT_EQ(v1.as<int>(), 123);
  ASSERT_EQ(v1.size(), sizeof(int));
  ASSERT_TRUE(v1.is_pod());
  v1 = (double)3.14;
  var v2=v1;
  ASSERT_EQ(v1.get_type(), typeid(double));
  ASSERT_EQ(v1.as<double>(), 3.14);
  v1 = "abc123";
  v2=v1;
  ASSERT_EQ(v1.get_type(), typeid(const char *));
  ASSERT_STREQ(v1.as<const char *>(), "abc123");
  v1 = L"xyz";
  ASSERT_EQ(v1.get_type(), typeid(const wchar_t *));
  ASSERT_STREQ(v1.as<const wchar_t *>(), L"xyz");
}

TEST(test_var, xstring){
  xtd::string s1="12345";
  ASSERT_NO_THROW(var t(s1));
  var x(s1);
  ASSERT_TRUE(x.get_type() == typeid(s1));
  ASSERT_STREQ(x.as<xtd::string>().c_str(), s1.c_str());
  x.as<xtd::string>() = "ABC";
  ASSERT_STRNE(x.as<xtd::string>().c_str(), s1.c_str());
}


TEST(test_var, is_pod){
  xtd::var v1 = 123;
  ASSERT_TRUE(v1.is_pod());
  xtd::string xs = "abc";
  v1 = xs;
  ASSERT_FALSE(v1.is_pod());
  struct s1{
    char c;
    short s;
    int i;
    long long l;
  };
  s1 oS1 {0,0,0,0};
  v1 = oS1;
  ASSERT_TRUE(v1.is_pod());
  struct s2 : s1{
    double d;
    s2() : s1(), d(0.0) {}
    virtual void Snafoo(){ i *= c; }
  };
  s2 oS2;
  v1 = oS2;
  ASSERT_FALSE(v1.is_pod());
}


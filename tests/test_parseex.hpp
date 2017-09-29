
#include <xtd/parseex.hpp>

namespace grammar {
  using namespace xtd::parseex;
  CHARACTER_(X, 'x');
  CHARACTER_(Y, 'y');
  CHARACTER_(Z, 'z');
  using grammar_t = and_<grammar::X, grammar::Y, grammar::Z>;
}


TEST(test_parseex, can_parse_character) {
  using namespace grammar;
  std::string s = "xyz";
  auto oAST = parse<X>(s);
}
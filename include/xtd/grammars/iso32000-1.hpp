/** @file
 * PDF file format
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/xtd.hpp>
#include <xtd/parse.hpp>

namespace xtd{
  namespace grammar{
    namespace iso32000{
      template <typename ... _Args> using and_ = parse::and_<_Args...>;
      template <typename ... _Args> using or_ = parse::or_<_Args...>;
      template <typename _Arg> using zero_or_one_ = parse::zero_or_one_<_Arg>;
      template <typename _Arg> using zero_or_more_ = parse::zero_or_more_<_Arg>;
      template <typename _Arg> using one_or_more_ = parse::one_or_more_<_Arg>;
      template <typename _ImplT, typename _DeclT> using rule = parse::rule<_ImplT, _DeclT>;

      CHARACTER_(NUL, '\0');
      CHARACTER_(HT, '\x09');
      CHARACTER_(LF, '\x0a');
      CHARACTER_(FF, '\x0c');
      CHARACTER_(CR, '\x0d');
      CHARACTER_(SP, '\x20');
      using whitespace = parse::whitespace<'\x00', '\x09', '\x0a', '\x0c', '\x0d', '\x20'>;

      CHARACTER_(OPEN_PARAN, '\x28');
      CHARACTER_(CLOSE_PARAN, '\x29');
      CHARACTER_(LESS_THAN, '\x3c');
      CHARACTER_(GREATER_THAN, '\x3e');
      CHARACTER_(OPEN_BRACKET, '[');
      CHARACTER_(CLOSE_BRACKET, ']');
      CHARACTER_(OPEN_BRACE, '{');
      CHARACTER_(CLOSE_BRACE, '}');
      CHARACTER_(SOLIDUS, '/');
      CHARACTER_(FORWARD_SLASH, '/');
      CHARACTER_(PERCENT, '%');
      CHARACTER_(PLUS, '+');
      CHARACTER_(HYPHEN, '-');
      CHARACTER_(PERIOD, '\x2e');
      CHARACTERS_(DIGIT, '0', '9');
      using EOL = parse::and_<CR,LF>;
      REGEX(COMMENT, "\\%[^\\r\\n]*");

      STRING(True, "true");
      STRING(False, "false");

      using number = one_or_more_<DIGIT>;
      struct Integer : rule<Integer, and_< zero_or_one_<or_<PLUS, HYPHEN>>, number>>{};

      using period_number = and_<PERIOD, one_or_more_<DIGIT>>;
      using number_period_number = and_<one_or_more_<DIGIT>, PERIOD, zero_or_more_<DIGIT>>;

      struct Real : rule<Real, and_< zero_or_one_<or_<PLUS, HYPHEN>>, or_<period_number, number_period_number, number>>>{};

      struct literal_string : rule<literal_string, and_< OPEN_PARAN, CLOSE_PARAN>>{};

    }
  }
}

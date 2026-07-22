/** @file
 * RFC 5234 Augmented BNF for Syntax Specifications: ABNF (core rules).
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 *
 * https://tools.ietf.org/html/rfc5234
 */

#pragma once

#include "xtd/xtd.hpp"
#include "xtd/parse.hpp"

namespace xtd {
  namespace Grammars {
    namespace RFC5234 {

      namespace P = xtd::parse;

      CHARACTER_(CR, '\r');
      CHARACTER_(LF, '\n');
      CHARACTER_(SP, ' ');
      CHARACTER_(HTAB, '\t');
      CHARACTER_(DQUOTE, '"');

      CHARACTERS_(DIGIT, '0', '9');
      CHARACTERS_(ALPHA_UPPER, 'A', 'Z');
      CHARACTERS_(ALPHA_LOWER, 'a', 'z');
      CHARACTERS_(HEX_LOWER, 'a', 'f');
      CHARACTERS_(HEX_UPPER, 'A', 'F');
      CHARACTERS_(VCHAR, '\x21', '\x7e');
      CHARACTERS_(OCTET, '\x00', '\xff');
      CHARACTERS_(CTL_LOW, '\x00', '\x1f');
      CHARACTER_(CTL_DEL, '\x7f');

      using ALPHA = P::or_<ALPHA_UPPER, ALPHA_LOWER>;
      using HEXDIG = P::or_<DIGIT, HEX_LOWER, HEX_UPPER>;
      using CTL = P::or_<CTL_LOW, CTL_DEL>;
      using WSP = P::or_<SP, HTAB>;

      struct CRLF : P::rule<CRLF, P::and_<CR, LF>> {
        template <typename... Ts>
        CRLF(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // LWSP = *(WSP / CRLF WSP)
      struct LWSP : P::rule<LWSP, P::zero_or_more_<P::or_<WSP, P::and_<CRLF, WSP>>>> {
        template <typename... Ts>
        LWSP(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

    }
  }
}

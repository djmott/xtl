/** @file
 * RFC 4647 Matching of Language Tags (basic language-range).
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 *
 * https://tools.ietf.org/html/rfc4647
 *
 * Implements Section 2.1 Basic Language Range only (referenced by RFC 7231
 * Accept-Language). There is no Repeat<> in xtd::parse; 1*8ALPHA is
 * hand-expanded with and_ / zero_or_one_.
 */

#pragma once

#include "xtd/xtd.hpp"
#include "xtd/parse.hpp"
#include "xtd/grammars/RFC5234.hpp"

namespace xtd {
  namespace Grammars {
    namespace RFC4647 {

      namespace P = xtd::parse;

#pragma region("imports from RFC5234")
      using ALPHA = RFC5234::ALPHA;
#pragma endregion

#pragma region("punctuation terminals")
      CHARACTER_(Hyphen, '-');
      CHARACTER_(Asterisk, '*');
#pragma endregion

#pragma region("bounded helpers")
      // 1*8ALPHA = ALPHA [ALPHA [ALPHA [ALPHA [ALPHA [ALPHA [ALPHA [ALPHA]]]]]]]
      using alpha_1_to_8 = P::and_<ALPHA, P::zero_or_one_<P::and_<
        ALPHA, P::zero_or_one_<P::and_<
          ALPHA, P::zero_or_one_<P::and_<
            ALPHA, P::zero_or_one_<P::and_<
              ALPHA, P::zero_or_one_<P::and_<
                ALPHA, P::zero_or_one_<P::and_<
                  ALPHA, P::zero_or_one_<ALPHA>
                >>
              >>
            >>
          >>
        >>
      >>>;
#pragma endregion

#pragma region("rules")
      // language-range = (1*8ALPHA *("-" 1*8ALPHA)) / "*"
      struct language_range : P::rule<language_range, P::or_<
        P::and_<alpha_1_to_8, P::zero_or_more_<P::and_<Hyphen, alpha_1_to_8>>>,
        Asterisk
      >> {
        template <typename... Ts>
        language_range(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

    }
  }
}

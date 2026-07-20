/** @file
 * RFC 1035 domain-name grammar (preferred name syntax, §2.3.1).
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#pragma once

#include "xtd/xtd.hpp"
#include "xtd/parse.hpp"

namespace xtd {
  namespace Grammars {
    namespace RFC1035 {
      // https://www.ietf.org/rfc/rfc1035.txt §2.3.1 Preferred name syntax
      // subdomain rewritten without left recursion: label *( "." label )

      namespace _ = xtd::parse;

      CHARACTERS_(letter_lower, 'a', 'z');
      CHARACTERS_(letter_upper, 'A', 'Z');
      CHARACTERS_(digit, '0', '9');
      CHARACTER_(hyphen, '-');
      CHARACTER_(dot, '.');

      using letter = _::or_<letter_lower, letter_upper>;
      using let_dig = _::or_<letter, digit>;
      using let_dig_hyp = _::or_<let_dig, hyphen>;
      using ldh_str = _::one_or_more_<let_dig_hyp>;
      // <label> ::= <letter> [ [ <ldh-str> ] <let-dig> ]
      using label = _::and_<letter, _::zero_or_one_<_::and_<_::zero_or_one_<ldh_str>, let_dig>>>;
      using subdomain = _::and_<label, _::zero_or_more_<_::and_<dot, label>>>;

      struct domain : _::rule<domain, subdomain> {
        template <typename... _ArgTs>
        domain(_ArgTs&&... oArgs) : rule(std::forward<_ArgTs>(oArgs)...) {}
      };

    }
  }
}

/** @file
 * DICT protocol parser https://tools.ietf.org/html/rfc2229
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <xtd/xtd.hpp>
#include <xtd/parse.hpp>

namespace xtd {
  namespace parse {
    namespace grammar {

      namespace rfc2229 {
        namespace _ = xtd::parse;

        namespace sz {
          STRING_(DEFINE);
        }

        CHARACTERS_(CHAR,0, 127);
        CHARACTERS_(CTL,0, 31);
        CHARACTER_(CR,13);
        CHARACTER_(LF,10);
        CHARACTER_(SPACE,32);
        CHARACTER_(HTAB,9);
        CHARACTER_(DQUOTE,34);
        CHARACTER_(SQUOTE,39);
        CHARACTER_(BSLASH,'\\');
        CHARACTER_(LT,'<');
        CHARACTER_(GT,'>');
        CHARACTER_(PERIOD,'.');
        CHARACTER_(AT,'@');
        using CRLF = _::and_<CR, LF>;
        using WS = _::one_or_more_<_::or_<SPACE, HTAB>>;
        using dqtext = _::and_<_::not_<DQUOTE, BSLASH, CTL>, CHAR>;
        using sqtext = _::and_<_::not_<SQUOTE, BSLASH, CTL>, CHAR>;
        using quoted_pair = _::and_<BSLASH, CHAR>;
        using atom = _::one_or_more_<_::and_<_::not_<SPACE, CTL, SQUOTE, DQUOTE, BSLASH>, CHAR> >;
        using sqstring = _::and_<SQUOTE, _::zero_or_more_<_::or_<dqtext, quoted_pair> >, SQUOTE>;
        using dqstring = _::and_<DQUOTE, _::zero_or_more_<_::or_<dqtext, quoted_pair> >, DQUOTE>;
        using string = _::zero_or_more_<_::or_<dqstring, sqstring, quoted_pair> >;
        using word = _::zero_or_more_<_::or_<atom, string> >;
        using description = _::zero_or_more_<_::or_<word, WS> >;
        using text = _::zero_or_more_<_::or_<word, WS> >;

        using cmd_word = atom;
        using database = atom;
        using strategy = atom;
        using cmd_param = _::or_<database, strategy, word>;

        struct command : _::rule<command, _::and_<cmd_word, _::zero_or_more_<_::and_<WS, cmd_param> >>> {
          template<typename ... _ArgTs>
          command(_ArgTs &&...oArgs) : rule(std::forward<_ArgTs>(oArgs)...) {}
        };

        struct msg_atom
          : _::rule<msg_atom, _::one_or_more_<_::and_<_::not_<SPACE, CTL, LT, GT, PERIOD, BSLASH>, CHAR>>> {
          template<typename ... _ArgTs>
          msg_atom(_ArgTs &&...oArgs) : rule(std::forward<_ArgTs>(oArgs)...) {}
        };

        struct capabilities
          : _::rule<capabilities, _::zero_or_one_<_::and_<LT, msg_atom, _::zero_or_more_<_::and_<PERIOD, msg_atom> >, GT >>> {
          template<typename ... _ArgTs>
          capabilities(_ArgTs &&...oArgs) : rule(std::forward<_ArgTs>(oArgs)...) {}
        };

        using local_part = _::and_<msg_atom, _::zero_or_more_<_::and_<PERIOD, msg_atom>>>;
        using domain = _::and_<msg_atom, _::zero_or_more_<_::and_<PERIOD, msg_atom>>>;
        struct spec : _::rule<spec, _::and_<local_part, AT, domain>> {
        };
        struct msg_id : _::rule<msg_id, _::and_<LT, spec, GT>> {
        };

        struct define_command : _::rule<define_command, _::and_<sz::DEFINE, database, word >> {

        };

      }
    }

  }
}

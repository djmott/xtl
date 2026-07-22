/** @file
 * RFC 5646 Tags for Identifying Languages (BCP 47 Language-Tag).
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 *
 * https://tools.ietf.org/html/rfc5646
 *
 * PEG notes:
 * - There is no Repeat<> in xtd::parse; N*M bounds are hand-expanded.
 * - Language_Tag tries grandfathered before langtag so tags like "zh-min-nan"
 *   are not partially absorbed as langtag + leftover (ordered choice is not ABNF /).
 * - language prefers 5*8ALPHA / 4ALPHA before 2*3ALPHA for the same reason.
 * - Subtags use not_<ALPHA>/not_<alphanum> boundaries so a longer subtag is not
 *   partially absorbed by a shorter NALPHA alternative (e.g. Hant vs Han).
 * - singleton excludes 'x'/'X' so privateuse owns that prefix.
 */

#pragma once

#include "xtd/xtd.hpp"
#include "xtd/parse.hpp"
#include "xtd/grammars/RFC5234.hpp"

namespace xtd {
  namespace Grammars {
    namespace RFC5646 {

      namespace P = xtd::parse;

#pragma region("imports from RFC5234")
      using ALPHA = RFC5234::ALPHA;
      using DIGIT = RFC5234::DIGIT;
#pragma endregion

#pragma region("punctuation / ranges")
      CHARACTER_(Hyphen, '-');
      CHARACTER_(_x, 'x');
      CHARACTERS_(SingletonUpperAW, '\x41', '\x57'); // A-W
      CHARACTERS_(SingletonUpperYZ, '\x59', '\x5A'); // Y-Z
      CHARACTERS_(SingletonLowerAW, '\x61', '\x77'); // a-w
      CHARACTERS_(SingletonLowerYZ, '\x79', '\x7A'); // y-z
      using alphanum = P::or_<ALPHA, DIGIT>;
#pragma endregion

#pragma region("string terminals — grandfathered")
      STRING(en_GB_oed, "en-GB-oed");
      STRING(i_ami, "i-ami");
      STRING(i_bnn, "i-bnn");
      STRING(i_default, "i-default");
      STRING(i_enochian, "i-enochian");
      STRING(i_hak, "i-hak");
      STRING(i_klingon, "i-klingon");
      STRING(i_lux, "i-lux");
      STRING(i_mingo, "i-mingo");
      STRING(i_navajo, "i-navajo");
      STRING(i_pwn, "i-pwn");
      STRING(i_tao, "i-tao");
      STRING(i_tay, "i-tay");
      STRING(i_tsu, "i-tsu");
      STRING(sgn_BE_FR, "sgn-BE-FR");
      STRING(sgn_BE_NL, "sgn-BE-NL");
      STRING(sgn_CH_DE, "sgn-CH-DE");
      STRING(art_lojban, "art-lojban");
      STRING(cel_gaulish, "cel-gaulish");
      STRING(no_bok, "no-bok");
      STRING(no_nyn, "no-nyn");
      STRING(zh_guoyu, "zh-guoyu");
      STRING(zh_hakka, "zh-hakka");
      STRING(zh_min_nan, "zh-min-nan");
      STRING(zh_min, "zh-min");
      STRING(zh_xiang, "zh-xiang");
#pragma endregion

#pragma region("forward declarations")
      struct Language_Tag;
      struct langtag;
      struct language;
      struct extlang;
      struct script;
      struct region;
      struct variant;
      struct extension;
      struct singleton;
      struct privateuse;
      struct grandfathered;
      struct irregular;
      struct regular;
#pragma endregion

#pragma region("bounded helpers")
      // Subtag boundary: next char must not continue the same ALPHA/alphanum run.
      using not_ALPHA = P::not_<ALPHA>;
      using not_alphanum = P::not_<alphanum>;
      using not_DIGIT = P::not_<DIGIT>;

      // 2*3ALPHA
      using alpha_2_to_3 = P::and_<ALPHA, ALPHA, P::zero_or_one_<ALPHA>>;
      // 3ALPHA
      using alpha_3 = P::and_<ALPHA, ALPHA, ALPHA>;
      // 4ALPHA
      using alpha_4 = P::and_<ALPHA, ALPHA, ALPHA, ALPHA>;
      // 5*8ALPHA
      using alpha_5_to_8 = P::and_<ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, P::zero_or_one_<P::and_<
        ALPHA, P::zero_or_one_<P::and_<ALPHA, P::zero_or_one_<ALPHA>>>
      >>>;
      // 3DIGIT
      using digit_3 = P::and_<DIGIT, DIGIT, DIGIT>;
      // 2ALPHA
      using alpha_2 = P::and_<ALPHA, ALPHA>;
      // 1*8alphanum
      using alphanum_1_to_8 = P::and_<alphanum, P::zero_or_one_<P::and_<
        alphanum, P::zero_or_one_<P::and_<
          alphanum, P::zero_or_one_<P::and_<
            alphanum, P::zero_or_one_<P::and_<
              alphanum, P::zero_or_one_<P::and_<
                alphanum, P::zero_or_one_<P::and_<
                  alphanum, P::zero_or_one_<alphanum>
                >>
              >>
            >>
          >>
        >>
      >>>;
      // 2*8alphanum
      using alphanum_2_to_8 = P::and_<alphanum, alphanum, P::zero_or_one_<P::and_<
        alphanum, P::zero_or_one_<P::and_<
          alphanum, P::zero_or_one_<P::and_<
            alphanum, P::zero_or_one_<P::and_<
              alphanum, P::zero_or_one_<P::and_<
                alphanum, P::zero_or_one_<alphanum>
              >>
            >>
          >>
        >>
      >>>;
      // 3alphanum
      using alphanum_3 = P::and_<alphanum, alphanum, alphanum>;
      // 5*8alphanum
      using alphanum_5_to_8 = P::and_<alphanum, alphanum, alphanum, alphanum, alphanum, P::zero_or_one_<P::and_<
        alphanum, P::zero_or_one_<P::and_<alphanum, P::zero_or_one_<alphanum>>>
      >>>;
      // *2("-" 3ALPHA) for extlang — each 3ALPHA is a full subtag
      using hyphen_alpha3_subtag = P::and_<Hyphen, alpha_3, not_ALPHA>;
      using up_to_2_hyphen_alpha3 = P::zero_or_one_<P::and_<
        hyphen_alpha3_subtag, P::zero_or_one_<hyphen_alpha3_subtag>
      >>;
#pragma endregion

#pragma region("2.1 Syntax")
      // singleton = DIGIT / %x41-57 / %x59-5A / %x61-77 / %x79-7A
      struct singleton : P::rule<singleton, P::or_<
        DIGIT, SingletonUpperAW, SingletonUpperYZ, SingletonLowerAW, SingletonLowerYZ
      >> {
        template <typename... Ts>
        singleton(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // extlang = 3ALPHA *2("-" 3ALPHA)
      struct extlang : P::rule<extlang, P::and_<alpha_3, not_ALPHA, up_to_2_hyphen_alpha3>> {
        template <typename... Ts>
        extlang(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // language = 5*8ALPHA / 4ALPHA / (2*3ALPHA ["-" extlang])
      // not_ALPHA prevents partial subtag absorption (e.g. "Hant" as extlang "Han").
      struct language : P::rule<language, P::or_<
        P::and_<alpha_5_to_8, not_ALPHA>,
        P::and_<alpha_4, not_ALPHA>,
        P::and_<alpha_2_to_3, not_ALPHA, P::zero_or_one_<P::and_<Hyphen, extlang>>>
      >> {
        template <typename... Ts>
        language(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // script = 4ALPHA
      struct script : P::rule<script, P::and_<alpha_4, not_ALPHA>> {
        template <typename... Ts>
        script(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // region = 2ALPHA / 3DIGIT
      struct region : P::rule<region, P::or_<
        P::and_<alpha_2, not_ALPHA>,
        P::and_<digit_3, not_DIGIT>
      >> {
        template <typename... Ts>
        region(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // variant = 5*8alphanum / (DIGIT 3alphanum)
      struct variant : P::rule<variant, P::or_<
        P::and_<alphanum_5_to_8, not_alphanum>,
        P::and_<DIGIT, alphanum_3, not_alphanum>
      >> {
        template <typename... Ts>
        variant(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // extension = singleton 1*("-" (2*8alphanum))
      struct extension : P::rule<extension, P::and_<
        singleton,
        P::one_or_more_<P::and_<Hyphen, alphanum_2_to_8, not_alphanum>>
      >> {
        template <typename... Ts>
        extension(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // privateuse = "x" 1*("-" (1*8alphanum))
      struct privateuse : P::rule<privateuse, P::and_<
        _x,
        P::one_or_more_<P::and_<Hyphen, alphanum_1_to_8, not_alphanum>>
      >> {
        template <typename... Ts>
        privateuse(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // irregular — longest / most specific strings first where prefixes overlap
      struct irregular : P::rule<irregular, P::or_<
        en_GB_oed, i_default, i_enochian, i_klingon, i_navajo,
        i_ami, i_bnn, i_hak, i_lux, i_mingo, i_pwn, i_tao, i_tay, i_tsu,
        sgn_BE_FR, sgn_BE_NL, sgn_CH_DE
      >> {
        template <typename... Ts>
        irregular(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // regular — zh-min-nan before zh-min
      struct regular : P::rule<regular, P::or_<
        art_lojban, cel_gaulish, no_bok, no_nyn,
        zh_guoyu, zh_hakka, zh_min_nan, zh_xiang, zh_min
      >> {
        template <typename... Ts>
        regular(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // grandfathered = irregular / regular
      struct grandfathered : P::rule<grandfathered, P::or_<irregular, regular>> {
        template <typename... Ts>
        grandfathered(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // langtag = language ["-" script] ["-" region] *("-" variant) *("-" extension) ["-" privateuse]
      struct langtag : P::rule<langtag, P::and_<
        language,
        P::zero_or_one_<P::and_<Hyphen, script>>,
        P::zero_or_one_<P::and_<Hyphen, region>>,
        P::zero_or_more_<P::and_<Hyphen, variant>>,
        P::zero_or_more_<P::and_<Hyphen, extension>>,
        P::zero_or_one_<P::and_<Hyphen, privateuse>>
      >> {
        template <typename... Ts>
        langtag(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Language-Tag = grandfathered / privateuse / langtag  (PEG-safe order)
      struct Language_Tag : P::rule<Language_Tag, P::or_<grandfathered, privateuse, langtag>> {
        template <typename... Ts>
        Language_Tag(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

    }
  }
}

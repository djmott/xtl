/** @file
 * RFC 3986 Uniform Resource Identifier (URI) generic syntax.
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 *
 * https://tools.ietf.org/html/rfc3986
 *
 * PEG notes (ordered choice is not pure ABNF alternation):
 * - dec_octet alternatives are listed most-specific first so 255 is not swallowed as Digit.
 * - IPv6address lists the nine §3.2.2 forms with longer/more-constrained tails before bare "::".
 * - ls32 tries IPv4address before (h16 ":" h16) so dotted tails are not hex-eaten.
 * - path_empty is last among hier_part / relative_part alternatives.
 */

#pragma once

#include "xtd/xtd.hpp"
#include "xtd/parse.hpp"

namespace xtd {
  namespace Grammars {
    namespace RFC3986 {

      namespace P = xtd::parse;

#pragma region("terminals")
      CHARACTERS_(AlphaLower, 'a', 'z');
      CHARACTERS_(AlphaUpper, 'A', 'Z');
      CHARACTERS_(Digit, '0', '9');
      CHARACTERS_(HexLower, 'a', 'f');
      CHARACTERS_(HexUpper, 'A', 'F');

      CHARACTER_(Colon, ':');
      CHARACTER_(ForwardSlash, '/');
      CHARACTER_(Question, '?');
      CHARACTER_(Hash, '#');
      CHARACTER_(OpenBracket, '[');
      CHARACTER_(CloseBracket, ']');
      CHARACTER_(At, '@');
      CHARACTER_(Exclaimation, '!');
      CHARACTER_(Dollar, '$');
      CHARACTER_(Ampersand, '&');
      CHARACTER_(SingleQuote, '\'');
      CHARACTER_(OpenParan, '(');
      CHARACTER_(CloseParan, ')');
      CHARACTER_(Asterisk, '*');
      CHARACTER_(Plus, '+');
      CHARACTER_(Comma, ',');
      CHARACTER_(SemiColon, ';');
      CHARACTER_(Equal, '=');
      CHARACTER_(Hyphen, '-');
      CHARACTER_(Period, '.');
      CHARACTER_(UnderScore, '_');
      CHARACTER_(Tilde, '~');
      CHARACTER_(Percent, '%');
      CHARACTER_(v_char, 'v');

      CHARACTER_(_0, '0');
      CHARACTER_(_1, '1');
      CHARACTER_(_2, '2');
      CHARACTER_(_3, '3');
      CHARACTER_(_4, '4');
      CHARACTER_(_5, '5');
      CHARACTER_(_6, '6');
      CHARACTER_(_7, '7');
      CHARACTER_(_8, '8');
      CHARACTER_(_9, '9');

      STRING(DoubleForwardSlash, "//");
      STRING(ColonColon, "::");
      STRING(_25, "25");

      using Alpha = P::or_<AlphaLower, AlphaUpper>;
      using AlphaNumeric = P::or_<Alpha, Digit>;
      using HexDigit = P::or_<Digit, HexLower, HexUpper>;
#pragma endregion

#pragma region("character classes")
      using gen_delims = P::or_<Colon, ForwardSlash, Question, Hash, OpenBracket, CloseBracket, At>;

      using sub_delims = P::or_<Exclaimation, Dollar, Ampersand, SingleQuote, OpenParan, CloseParan,
        Asterisk, Plus, Comma, SemiColon, Equal>;

      using unreserved = P::or_<AlphaNumeric, Hyphen, Period, UnderScore, Tilde>;

      using reserved = P::or_<gen_delims, sub_delims>;
#pragma endregion

#pragma region("forward declarations")
      struct URI;
      struct hier_part;
      struct URI_reference;
      struct absolute_URI;
      struct relative_ref;
      struct relative_part;
      struct scheme;
      struct authority;
      struct userinfo;
      struct host;
      struct port;
      struct IP_literal;
      struct IPvFuture;
      struct IPv6address;
      struct ls32;
      struct IPv4address;
      struct dec_octet;
      struct reg_name;
      struct path;
      struct path_abempty;
      struct path_absolute;
      struct path_noscheme;
      struct path_rootless;
      struct path_empty;
      struct segment;
      struct segment_nz;
      struct segment_nz_nc;
      struct pchar;
      struct query;
      struct fragment;
      struct pct_encoded;
#pragma endregion

      // h16 = 1*4HEXDIG (longest-first; no Repeat<> in xtd::parse)
      using h16 = P::or_<
        P::and_<HexDigit, HexDigit, HexDigit, HexDigit>,
        P::and_<HexDigit, HexDigit, HexDigit>,
        P::and_<HexDigit, HexDigit>,
        HexDigit>;

      struct pct_encoded : P::rule<pct_encoded, P::and_<Percent, HexDigit, HexDigit>> {
        template <typename... Ts>
        pct_encoded(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct scheme : P::rule<scheme, P::and_<Alpha, P::zero_or_more_<P::or_<AlphaNumeric, Plus, Hyphen, Period>>>> {
        template <typename... Ts>
        scheme(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct userinfo : P::rule<userinfo, P::zero_or_more_<P::or_<unreserved, pct_encoded, sub_delims, Colon>>> {
        template <typename... Ts>
        userinfo(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct port : P::rule<port, P::zero_or_more_<Digit>> {
        template <typename... Ts>
        port(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // dec_octet: most-specific first (PEG)
      struct dec_octet : P::rule<dec_octet, P::or_<
        P::and_<_25, P::or_<_0, _1, _2, _3, _4, _5>>,
        P::and_<_2, P::or_<_0, _1, _2, _3, _4>, Digit>,
        P::and_<_1, Digit, Digit>,
        P::and_<P::or_<_1, _2, _3, _4, _5, _6, _7, _8, _9>, Digit>,
        Digit
      >> {
        template <typename... Ts>
        dec_octet(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct IPv4address : P::rule<IPv4address, P::and_<dec_octet, Period, dec_octet, Period, dec_octet, Period, dec_octet>> {
        template <typename... Ts>
        IPv4address(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct ls32 : P::rule<ls32, P::or_<IPv4address, P::and_<h16, Colon, h16>>> {
        template <typename... Ts>
        ls32(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Optional prefixes for compressed IPv6 forms: [ *n( h16 ":" ) h16 ]
      using ipv6_pref1 = P::zero_or_one_<P::or_<P::and_<h16, Colon, h16>, h16>>;
      using ipv6_pref2 = P::zero_or_one_<P::or_<
        P::and_<h16, Colon, h16, Colon, h16>,
        P::and_<h16, Colon, h16>,
        h16>>;
      using ipv6_pref3 = P::zero_or_one_<P::or_<
        P::and_<h16, Colon, h16, Colon, h16, Colon, h16>,
        P::and_<h16, Colon, h16, Colon, h16>,
        P::and_<h16, Colon, h16>,
        h16>>;
      using ipv6_pref4 = P::zero_or_one_<P::or_<
        P::and_<h16, Colon, h16, Colon, h16, Colon, h16, Colon, h16>,
        P::and_<h16, Colon, h16, Colon, h16, Colon, h16>,
        P::and_<h16, Colon, h16, Colon, h16>,
        P::and_<h16, Colon, h16>,
        h16>>;
      using ipv6_pref5 = P::zero_or_one_<P::or_<
        P::and_<h16, Colon, h16, Colon, h16, Colon, h16, Colon, h16, Colon, h16>,
        P::and_<h16, Colon, h16, Colon, h16, Colon, h16, Colon, h16>,
        P::and_<h16, Colon, h16, Colon, h16, Colon, h16>,
        P::and_<h16, Colon, h16, Colon, h16>,
        P::and_<h16, Colon, h16>,
        h16>>;
      using ipv6_pref6 = P::zero_or_one_<P::or_<
        P::and_<h16, Colon, h16, Colon, h16, Colon, h16, Colon, h16, Colon, h16, Colon, h16>,
        P::and_<h16, Colon, h16, Colon, h16, Colon, h16, Colon, h16, Colon, h16>,
        P::and_<h16, Colon, h16, Colon, h16, Colon, h16, Colon, h16>,
        P::and_<h16, Colon, h16, Colon, h16, Colon, h16>,
        P::and_<h16, Colon, h16, Colon, h16>,
        P::and_<h16, Colon, h16>,
        h16>>;

      // Nine IPv6address alternatives (§3.2.2), longest/most constrained before bare "::"
      using ipv6_form1 = P::and_<h16, Colon, h16, Colon, h16, Colon, h16, Colon, h16, Colon, h16, Colon, ls32>;
      using ipv6_form2 = P::and_<ColonColon, h16, Colon, h16, Colon, h16, Colon, h16, Colon, h16, Colon, ls32>;
      using ipv6_form3 = P::and_<P::zero_or_one_<h16>, ColonColon, h16, Colon, h16, Colon, h16, Colon, h16, Colon, ls32>;
      using ipv6_form4 = P::and_<ipv6_pref1, ColonColon, h16, Colon, h16, Colon, h16, Colon, ls32>;
      using ipv6_form5 = P::and_<ipv6_pref2, ColonColon, h16, Colon, h16, Colon, ls32>;
      using ipv6_form6 = P::and_<ipv6_pref3, ColonColon, h16, Colon, ls32>;
      using ipv6_form7 = P::and_<ipv6_pref4, ColonColon, ls32>;
      using ipv6_form8 = P::and_<ipv6_pref5, ColonColon, h16>;
      using ipv6_form9 = P::and_<ipv6_pref6, ColonColon>;

      struct IPv6address : P::rule<IPv6address, P::or_<
        ipv6_form1, ipv6_form2, ipv6_form3, ipv6_form4, ipv6_form5,
        ipv6_form6, ipv6_form7, ipv6_form8, ipv6_form9
      >> {
        template <typename... Ts>
        IPv6address(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct IPvFuture : P::rule<IPvFuture, P::and_<
        v_char, P::one_or_more_<HexDigit>, Period, P::one_or_more_<P::or_<unreserved, sub_delims, Colon>>
      >> {
        template <typename... Ts>
        IPvFuture(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct IP_literal : P::rule<IP_literal, P::and_<OpenBracket, P::or_<IPv6address, IPvFuture>, CloseBracket>> {
        template <typename... Ts>
        IP_literal(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct reg_name : P::rule<reg_name, P::zero_or_more_<P::or_<unreserved, pct_encoded, sub_delims>>> {
        template <typename... Ts>
        reg_name(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct host : P::rule<host, P::or_<IP_literal, IPv4address, reg_name>> {
        template <typename... Ts>
        host(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct authority : P::rule<authority, P::and_<
        P::zero_or_one_<P::and_<userinfo, At>>,
        host,
        P::zero_or_one_<P::and_<Colon, port>>
      >> {
        template <typename... Ts>
        authority(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct pchar : P::rule<pchar, P::or_<unreserved, pct_encoded, sub_delims, Colon, At>> {
        template <typename... Ts>
        pchar(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct segment : P::rule<segment, P::zero_or_more_<pchar>> {
        template <typename... Ts>
        segment(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct segment_nz : P::rule<segment_nz, P::one_or_more_<pchar>> {
        template <typename... Ts>
        segment_nz(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct segment_nz_nc : P::rule<segment_nz_nc, P::one_or_more_<P::or_<unreserved, pct_encoded, sub_delims, At>>> {
        template <typename... Ts>
        segment_nz_nc(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Empty match (decl == impl); must not use EOF
      struct path_empty : P::rule<path_empty> {
        template <typename... Ts>
        path_empty(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct path_abempty : P::rule<path_abempty, P::zero_or_more_<P::and_<ForwardSlash, segment>>> {
        template <typename... Ts>
        path_abempty(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct path_absolute : P::rule<path_absolute, P::and_<
        ForwardSlash,
        P::zero_or_one_<P::and_<segment_nz, P::zero_or_more_<P::and_<ForwardSlash, segment>>>>
      >> {
        template <typename... Ts>
        path_absolute(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct path_noscheme : P::rule<path_noscheme, P::and_<
        segment_nz_nc, P::zero_or_more_<P::and_<ForwardSlash, segment>>
      >> {
        template <typename... Ts>
        path_noscheme(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct path_rootless : P::rule<path_rootless, P::and_<
        segment_nz, P::zero_or_more_<P::and_<ForwardSlash, segment>>
      >> {
        template <typename... Ts>
        path_rootless(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct path : P::rule<path, P::or_<path_abempty, path_absolute, path_noscheme, path_rootless, path_empty>> {
        template <typename... Ts>
        path(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct query : P::rule<query, P::zero_or_more_<P::or_<pchar, ForwardSlash, Question>>> {
        template <typename... Ts>
        query(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct fragment : P::rule<fragment, P::zero_or_more_<P::or_<pchar, ForwardSlash, Question>>> {
        template <typename... Ts>
        fragment(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct hier_part : P::rule<hier_part, P::or_<
        P::and_<DoubleForwardSlash, authority, path_abempty>,
        path_absolute,
        path_rootless,
        path_empty
      >> {
        template <typename... Ts>
        hier_part(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct relative_part : P::rule<relative_part, P::or_<
        P::and_<DoubleForwardSlash, authority, path_abempty>,
        path_absolute,
        path_noscheme,
        path_empty
      >> {
        template <typename... Ts>
        relative_part(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct URI : P::rule<URI, P::and_<
        scheme, Colon, hier_part,
        P::zero_or_one_<P::and_<Question, query>>,
        P::zero_or_one_<P::and_<Hash, fragment>>
      >> {
        template <typename... Ts>
        URI(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct absolute_URI : P::rule<absolute_URI, P::and_<
        scheme, Colon, hier_part,
        P::zero_or_one_<P::and_<Question, query>>
      >> {
        template <typename... Ts>
        absolute_URI(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct relative_ref : P::rule<relative_ref, P::and_<
        relative_part,
        P::zero_or_one_<P::and_<Question, query>>,
        P::zero_or_one_<P::and_<Hash, fragment>>
      >> {
        template <typename... Ts>
        relative_ref(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct URI_reference : P::rule<URI_reference, P::or_<URI, relative_ref>> {
        template <typename... Ts>
        URI_reference(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

    }
  }
}

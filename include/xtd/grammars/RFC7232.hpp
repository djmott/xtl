/** @file
 * RFC 7232 Hypertext Transfer Protocol (HTTP/1.1): Conditional Requests.
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 *
 * https://tools.ietf.org/html/rfc7232
 *
 * PEG notes:
 * - Header rules match header values only (no field-name / ":" prefix).
 * - Empty whitespace-skip context required (same as RFC7230/7231):
 *   xtd::parser with Rule, false, and empty whitespace<>.
 * - `weak` is case-sensitive "W/" (%x57.2F).
 * - `If_Match` / `If_None_Match` try "*" before 1#entity-tag.
 * - opaque-tag is not quoted-string: etagc has no quoted-pair / escapes.
 */

#pragma once

#include "xtd/xtd.hpp"
#include "xtd/parse.hpp"
#include "xtd/grammars/RFC7230.hpp"
#include "xtd/grammars/RFC7231.hpp"

namespace xtd {
  namespace Grammars {
    namespace RFC7232 {

      namespace P = xtd::parse;

#pragma region("imports")
      using OWS = RFC7230::OWS;
      using obs_text = RFC7230::obs_text;
      using DQUOTE = RFC7230::DQUOTE;
      using HTTP_date = RFC7231::HTTP_date;
#pragma endregion

#pragma region("punctuation terminals")
      CHARACTER_(Comma, ',');
      CHARACTER_(Asterisk, '*');
      CHARACTER_(Exclamation, '!');
      CHARACTERS_(etagc_vchar, '\x23', '\x7E');
#pragma endregion

#pragma region("string terminals")
      STRING(weak, "W/");
#pragma endregion

#pragma region("forward declarations")
      struct etagc;
      struct opaque_tag;
      struct entity_tag;
      struct ETag;
      struct If_Match;
      struct If_None_Match;
      struct If_Modified_Since;
      struct If_Unmodified_Since;
      struct Last_Modified;
#pragma endregion

#pragma region("rules")
      //- etagc = "!" / %x23-7E / obs-text
      struct etagc : P::rule<etagc, P::or_<Exclamation, etagc_vchar, obs_text>> {
        template <typename... Ts>
        etagc(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- opaque-tag = DQUOTE *etagc DQUOTE
      struct opaque_tag : P::rule<opaque_tag, P::and_<
        DQUOTE, P::zero_or_more_<etagc>, DQUOTE
      >> {
        template <typename... Ts>
        opaque_tag(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- entity-tag = [ weak ] opaque-tag
      struct entity_tag : P::rule<entity_tag, P::and_<
        P::zero_or_one_<weak>, opaque_tag
      >> {
        template <typename... Ts>
        entity_tag(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- ETag = entity-tag
      struct ETag : P::rule<ETag, entity_tag> {
        template <typename... Ts>
        ETag(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- If-Match = "*" / 1#entity-tag
      struct If_Match : P::rule<If_Match, P::or_<
        Asterisk,
        P::and_<
          P::zero_or_more_<P::and_<Comma, OWS>>,
          entity_tag,
          P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<OWS, entity_tag>>>>
        >
      >> {
        template <typename... Ts>
        If_Match(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- If-None-Match = "*" / 1#entity-tag
      struct If_None_Match : P::rule<If_None_Match, P::or_<
        Asterisk,
        P::and_<
          P::zero_or_more_<P::and_<Comma, OWS>>,
          entity_tag,
          P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<OWS, entity_tag>>>>
        >
      >> {
        template <typename... Ts>
        If_None_Match(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- If-Modified-Since = HTTP-date
      struct If_Modified_Since : P::rule<If_Modified_Since, HTTP_date> {
        template <typename... Ts>
        If_Modified_Since(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- If-Unmodified-Since = HTTP-date
      struct If_Unmodified_Since : P::rule<If_Unmodified_Since, HTTP_date> {
        template <typename... Ts>
        If_Unmodified_Since(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- Last-Modified = HTTP-date
      struct Last_Modified : P::rule<Last_Modified, HTTP_date> {
        template <typename... Ts>
        Last_Modified(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

    }
  }
}

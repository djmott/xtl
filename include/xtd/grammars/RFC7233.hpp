/** @file
 * RFC 7233 Hypertext Transfer Protocol (HTTP/1.1): Range Requests.
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 *
 * https://tools.ietf.org/html/rfc7233
 *
 * PEG notes:
 * - Header rules match header values only (no field-name / ":" prefix).
 * - Empty whitespace-skip context required (same as RFC7230/7231/7232):
 *   xtd::parser with Rule, false, and empty whitespace<>.
 * - `bytes` and `none` are case-sensitive string terminals.
 * - Range / Content-Range try bytes-* forms before other-* (token would match "bytes").
 * - Accept-Ranges tries "none" before 1#range-unit (token would match "none").
 * - byte-range-set elements try suffix-byte-range-spec before byte-range-spec.
 * - byte-content-range body tries unsatisfied-range before byte-range-resp.
 * - If-Range tries entity-tag before HTTP-date.
 * - CHAR (%x01-7F) is local; RFC5234 does not yet export it.
 */

#pragma once

#include "xtd/xtd.hpp"
#include "xtd/parse.hpp"
#include "xtd/grammars/RFC7230.hpp"
#include "xtd/grammars/RFC7231.hpp"
#include "xtd/grammars/RFC7232.hpp"

namespace xtd {
  namespace Grammars {
    namespace RFC7233 {

      namespace P = xtd::parse;

#pragma region("imports")
      using DIGIT = RFC7230::DIGIT;
      using OWS = RFC7230::OWS;
      using SP = RFC7230::SP;
      using token = RFC7230::token;
      using VCHAR = RFC7230::VCHAR;
      using HTTP_date = RFC7231::HTTP_date;
      using entity_tag = RFC7232::entity_tag;
#pragma endregion

#pragma region("punctuation terminals")
      CHARACTER_(EqualSign, '=');
      CHARACTER_(Hyphen, '-');
      CHARACTER_(ForwardSlash, '/');
      CHARACTER_(Comma, ',');
      CHARACTER_(Asterisk, '*');
      CHARACTERS_(CHAR, '\x01', '\x7F');
#pragma endregion

#pragma region("string terminals")
      STRING(bytes, "bytes");
      STRING(none, "none");
#pragma endregion

#pragma region("forward declarations")
      struct bytes_unit;
      struct other_range_unit;
      struct range_unit;
      struct first_byte_pos;
      struct last_byte_pos;
      struct suffix_length;
      struct complete_length;
      struct byte_range_spec;
      struct suffix_byte_range_spec;
      struct byte_range_set;
      struct byte_ranges_specifier;
      struct other_range_set;
      struct other_ranges_specifier;
      struct Range;
      struct acceptable_ranges;
      struct Accept_Ranges;
      struct byte_range;
      struct byte_range_resp;
      struct unsatisfied_range;
      struct byte_content_range;
      struct other_range_resp;
      struct other_content_range;
      struct Content_Range;
      struct If_Range;
#pragma endregion

#pragma region("rules")
      //- bytes-unit = "bytes"
      struct bytes_unit : P::rule<bytes_unit, bytes> {
        template <typename... Ts>
        bytes_unit(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- other-range-unit = token
      struct other_range_unit : P::rule<other_range_unit, token> {
        template <typename... Ts>
        other_range_unit(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- range-unit = bytes-unit / other-range-unit
      struct range_unit : P::rule<range_unit, P::or_<bytes_unit, other_range_unit>> {
        template <typename... Ts>
        range_unit(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- first-byte-pos = 1*DIGIT
      struct first_byte_pos : P::rule<first_byte_pos, P::one_or_more_<DIGIT>> {
        template <typename... Ts>
        first_byte_pos(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- last-byte-pos = 1*DIGIT
      struct last_byte_pos : P::rule<last_byte_pos, P::one_or_more_<DIGIT>> {
        template <typename... Ts>
        last_byte_pos(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- suffix-length = 1*DIGIT
      struct suffix_length : P::rule<suffix_length, P::one_or_more_<DIGIT>> {
        template <typename... Ts>
        suffix_length(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- complete-length = 1*DIGIT
      struct complete_length : P::rule<complete_length, P::one_or_more_<DIGIT>> {
        template <typename... Ts>
        complete_length(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- byte-range-spec = first-byte-pos "-" [ last-byte-pos ]
      struct byte_range_spec : P::rule<byte_range_spec, P::and_<
        first_byte_pos, Hyphen, P::zero_or_one_<last_byte_pos>
      >> {
        template <typename... Ts>
        byte_range_spec(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- suffix-byte-range-spec = "-" suffix-length
      struct suffix_byte_range_spec : P::rule<suffix_byte_range_spec, P::and_<
        Hyphen, suffix_length
      >> {
        template <typename... Ts>
        suffix_byte_range_spec(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- byte-range-set = 1#( byte-range-spec / suffix-byte-range-spec )
      struct byte_range_set : P::rule<byte_range_set, P::and_<
        P::zero_or_more_<P::and_<Comma, OWS>>,
        P::or_<suffix_byte_range_spec, byte_range_spec>,
        P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<OWS, P::or_<suffix_byte_range_spec, byte_range_spec>>>>>
      >> {
        template <typename... Ts>
        byte_range_set(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- byte-ranges-specifier = bytes-unit "=" byte-range-set
      struct byte_ranges_specifier : P::rule<byte_ranges_specifier, P::and_<
        bytes_unit, EqualSign, byte_range_set
      >> {
        template <typename... Ts>
        byte_ranges_specifier(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- other-range-set = 1*VCHAR
      struct other_range_set : P::rule<other_range_set, P::one_or_more_<VCHAR>> {
        template <typename... Ts>
        other_range_set(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- other-ranges-specifier = other-range-unit "=" other-range-set
      struct other_ranges_specifier : P::rule<other_ranges_specifier, P::and_<
        other_range_unit, EqualSign, other_range_set
      >> {
        template <typename... Ts>
        other_ranges_specifier(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- Range = byte-ranges-specifier / other-ranges-specifier
      struct Range : P::rule<Range, P::or_<byte_ranges_specifier, other_ranges_specifier>> {
        template <typename... Ts>
        Range(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- acceptable-ranges = 1#range-unit / "none"
      struct acceptable_ranges : P::rule<acceptable_ranges, P::or_<
        none,
        P::and_<
          P::zero_or_more_<P::and_<Comma, OWS>>,
          range_unit,
          P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<OWS, range_unit>>>>
        >
      >> {
        template <typename... Ts>
        acceptable_ranges(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- Accept-Ranges = acceptable-ranges
      struct Accept_Ranges : P::rule<Accept_Ranges, acceptable_ranges> {
        template <typename... Ts>
        Accept_Ranges(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- byte-range = first-byte-pos "-" last-byte-pos
      struct byte_range : P::rule<byte_range, P::and_<
        first_byte_pos, Hyphen, last_byte_pos
      >> {
        template <typename... Ts>
        byte_range(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- byte-range-resp = byte-range "/" ( complete-length / "*" )
      struct byte_range_resp : P::rule<byte_range_resp, P::and_<
        byte_range, ForwardSlash, P::or_<complete_length, Asterisk>
      >> {
        template <typename... Ts>
        byte_range_resp(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- unsatisfied-range = "*/" complete-length
      struct unsatisfied_range : P::rule<unsatisfied_range, P::and_<
        Asterisk, ForwardSlash, complete_length
      >> {
        template <typename... Ts>
        unsatisfied_range(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- byte-content-range = bytes-unit SP ( byte-range-resp / unsatisfied-range )
      struct byte_content_range : P::rule<byte_content_range, P::and_<
        bytes_unit, SP, P::or_<unsatisfied_range, byte_range_resp>
      >> {
        template <typename... Ts>
        byte_content_range(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- other-range-resp = *CHAR
      struct other_range_resp : P::rule<other_range_resp, P::zero_or_more_<CHAR>> {
        template <typename... Ts>
        other_range_resp(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- other-content-range = other-range-unit SP other-range-resp
      struct other_content_range : P::rule<other_content_range, P::and_<
        other_range_unit, SP, other_range_resp
      >> {
        template <typename... Ts>
        other_content_range(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- Content-Range = byte-content-range / other-content-range
      struct Content_Range : P::rule<Content_Range, P::or_<byte_content_range, other_content_range>> {
        template <typename... Ts>
        Content_Range(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- If-Range = entity-tag / HTTP-date
      struct If_Range : P::rule<If_Range, P::or_<entity_tag, HTTP_date>> {
        template <typename... Ts>
        If_Range(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

    }
  }
}

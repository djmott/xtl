/** @file
 * RFC 7231 Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content.
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 *
 * https://tools.ietf.org/html/rfc7231
 *
 * PEG notes:
 * - Header rules match header values only (no field-name / ":" prefix).
 * - Empty whitespace-skip context required (same as RFC7230):
 *   xtd::parser with Rule, false, and empty whitespace<>.
 * - No Repeat; bounded counts (2DIGIT, 4DIGIT, star-3DIGIT) are hand-expanded.
 * - media_range tries star/star then type/star before type/subtype.
 * - codings prefers identity and star literals before content-coding token.
 * - HTTP_date tries IMF-fixdate before obs-date.
 */

#pragma once

#include "xtd/xtd.hpp"
#include "xtd/parse.hpp"
#include "xtd/grammars/RFC7230.hpp"
#include "xtd/grammars/RFC5322.hpp"
#include "xtd/grammars/RFC4647.hpp"
#include "xtd/grammars/RFC5646.hpp"

namespace xtd {
  namespace Grammars {
    namespace RFC7231 {

      namespace P = xtd::parse;

#pragma region("imports")
      using language_range = RFC4647::language_range;
      using language_tag = RFC5646::Language_Tag;
      using mailbox = RFC5322::mailbox;

      using BWS = RFC7230::BWS;
      using OWS = RFC7230::OWS;
      using RWS = RFC7230::RWS;
      using URI_reference = RFC7230::URI_reference;
      using absolute_URI = RFC7230::absolute_URI;
      using comment = RFC7230::comment;
      using field_name = RFC7230::field_name;
      using partial_URI = RFC7230::partial_URI;
      using quoted_string = RFC7230::quoted_string;
      using token = RFC7230::token;

      using DIGIT = RFC7230::DIGIT;
      using SP = RFC7230::SP;
#pragma endregion

#pragma region("punctuation terminals")
      CHARACTER_(Colon, ':');
      CHARACTER_(ForwardSlash, '/');
      CHARACTER_(Asterisk, '*');
      CHARACTER_(Comma, ',');
      CHARACTER_(SemiColon, ';');
      CHARACTER_(Equal, '=');
      CHARACTER_(Hyphen, '-');
      CHARACTER_(Period, '.');
      CHARACTER_(_0, '0');
      CHARACTER_(_1, '1');
      CHARACTER_(_q, 'q');
#pragma endregion

#pragma region("string terminals")
      STRING(_100_continue, "100-continue");
      STRING(GMT, "GMT");
      STRING(identity, "identity");
      STRING(Mon, "Mon");
      STRING(Tue, "Tue");
      STRING(Wed, "Wed");
      STRING(Thu, "Thu");
      STRING(Fri, "Fri");
      STRING(Sat, "Sat");
      STRING(Sun, "Sun");
      STRING(Monday, "Monday");
      STRING(Tuesday, "Tuesday");
      STRING(Wednesday, "Wednesday");
      STRING(Thursday, "Thursday");
      STRING(Friday, "Friday");
      STRING(Saturday, "Saturday");
      STRING(Sunday, "Sunday");
      STRING(media_range_all, "*/*");
      STRING(media_subrange_all, "/*");
      STRING(Jan, "Jan");
      STRING(Feb, "Feb");
      STRING(Mar, "Mar");
      STRING(Apr, "Apr");
      STRING(May, "May");
      STRING(Jun, "Jun");
      STRING(Jul, "Jul");
      STRING(Aug, "Aug");
      STRING(Sep, "Sep");
      STRING(Oct, "Oct");
      STRING(Nov, "Nov");
      STRING(Dec, "Dec");
#pragma endregion

#pragma region("forward declarations")
      struct accept;
      struct Accept_Charset;
      struct Accept_Encoding;
      struct Accept_Language;
      struct Allow;
      struct Content_Encoding;
      struct Content_Language;
      struct Content_Location;
      struct Content_Type;
      struct Date;
      struct Expect;
      struct From;
      struct HTTP_date;
      struct IMF_fixdate;
      struct Location;
      struct Max_Forwards;
      struct Referer;
      struct Retry_After;
      struct Server;
      struct User_Agent;
      struct Vary;
      struct accept_ext;
      struct accept_params;
      struct asctime_date;
      struct charset;
      struct codings;
      struct content_coding;
      struct date1;
      struct date2;
      struct date3;
      struct day;
      struct day_name;
      struct day_name_1;
      struct delay_seconds;
      struct hour;
      struct media_range;
      struct media_type;
      struct method;
      struct minute;
      struct month;
      struct obs_date;
      struct parameter;
      struct product;
      struct product_version;
      struct qvalue;
      struct rfc850_date;
      struct second;
      struct subtype;
      struct time_of_day;
      struct type;
      struct weight;
      struct year;
#pragma endregion

#pragma region("bounded helpers")
      using digit_2 = P::and_<DIGIT, DIGIT>;
      using digit_4 = P::and_<DIGIT, DIGIT, DIGIT, DIGIT>;
      using up_to_3DIGIT = P::zero_or_one_<P::and_<
        DIGIT, P::zero_or_one_<P::and_<DIGIT, P::zero_or_one_<DIGIT>>>
      >>;
      using up_to_3zero = P::zero_or_one_<P::and_<
        _0, P::zero_or_one_<P::and_<_0, P::zero_or_one_<_0>>>
      >>;
#pragma endregion

#pragma region("date / time atoms")
      struct day_name : P::rule<day_name, P::or_<Mon, Tue, Wed, Thu, Fri, Sat, Sun>> {
        template <typename... Ts>
        day_name(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct day_name_1 : P::rule<day_name_1, P::or_<
        Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday
      >> {
        template <typename... Ts>
        day_name_1(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct month : P::rule<month, P::or_<
        Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec
      >> {
        template <typename... Ts>
        month(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct day : P::rule<day, digit_2> {
        template <typename... Ts>
        day(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct hour : P::rule<hour, digit_2> {
        template <typename... Ts>
        hour(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct minute : P::rule<minute, digit_2> {
        template <typename... Ts>
        minute(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct second : P::rule<second, digit_2> {
        template <typename... Ts>
        second(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct year : P::rule<year, digit_4> {
        template <typename... Ts>
        year(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // date1 = day SP month SP year
      struct date1 : P::rule<date1, P::and_<day, SP, month, SP, year>> {
        template <typename... Ts>
        date1(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // date2 = day "-" month "-" 2DIGIT
      struct date2 : P::rule<date2, P::and_<day, Hyphen, month, Hyphen, digit_2>> {
        template <typename... Ts>
        date2(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // date3 = month SP ( 2DIGIT / ( SP DIGIT ) )
      struct date3 : P::rule<date3, P::and_<
        month, SP, P::or_<digit_2, P::and_<SP, DIGIT>>
      >> {
        template <typename... Ts>
        date3(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // time-of-day = hour ":" minute ":" second
      struct time_of_day : P::rule<time_of_day, P::and_<hour, Colon, minute, Colon, second>> {
        template <typename... Ts>
        time_of_day(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // IMF-fixdate = day-name "," SP date1 SP time-of-day SP GMT
      struct IMF_fixdate : P::rule<IMF_fixdate, P::and_<
        day_name, Comma, SP, date1, SP, time_of_day, SP, GMT
      >> {
        template <typename... Ts>
        IMF_fixdate(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // rfc850-date = day-name-l "," SP date2 SP time-of-day SP GMT
      struct rfc850_date : P::rule<rfc850_date, P::and_<
        day_name_1, Comma, SP, date2, SP, time_of_day, SP, GMT
      >> {
        template <typename... Ts>
        rfc850_date(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // asctime-date = day-name SP date3 SP time-of-day SP year
      struct asctime_date : P::rule<asctime_date, P::and_<
        day_name, SP, date3, SP, time_of_day, SP, year
      >> {
        template <typename... Ts>
        asctime_date(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // obs-date = rfc850-date / asctime-date
      struct obs_date : P::rule<obs_date, P::or_<rfc850_date, asctime_date>> {
        template <typename... Ts>
        obs_date(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // HTTP-date = IMF-fixdate / obs-date
      struct HTTP_date : P::rule<HTTP_date, P::or_<IMF_fixdate, obs_date>> {
        template <typename... Ts>
        HTTP_date(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("media / quality / product")
      // qvalue = ( "0" [ "." *3DIGIT ] ) / ( "1" [ "." *3"0" ] )
      struct qvalue : P::rule<qvalue, P::or_<
        P::and_<_0, P::zero_or_one_<P::and_<Period, up_to_3DIGIT>>>,
        P::and_<_1, P::zero_or_one_<P::and_<Period, up_to_3zero>>>
      >> {
        template <typename... Ts>
        qvalue(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // weight = OWS ";" OWS "q=" qvalue
      struct weight : P::rule<weight, P::and_<OWS, SemiColon, OWS, _q, Equal, qvalue>> {
        template <typename... Ts>
        weight(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // parameter = token "=" ( token / quoted-string )
      struct parameter : P::rule<parameter, P::and_<
        token, Equal, P::or_<token, quoted_string>
      >> {
        template <typename... Ts>
        parameter(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct type : P::rule<type, token> {
        template <typename... Ts>
        type(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct subtype : P::rule<subtype, token> {
        template <typename... Ts>
        subtype(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // media-type = type "/" subtype *( OWS ";" OWS parameter )
      struct media_type : P::rule<media_type, P::and_<
        type, ForwardSlash, subtype,
        P::zero_or_more_<P::and_<OWS, SemiColon, OWS, parameter>>
      >> {
        template <typename... Ts>
        media_type(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // media-range = ( "*/*" / ( type "/*" ) / ( type "/" subtype ) ) *( OWS ";" OWS parameter )
      struct media_range : P::rule<media_range, P::and_<
        P::or_<
          media_range_all,
          P::and_<type, media_subrange_all>,
          P::and_<type, ForwardSlash, subtype>
        >,
        P::zero_or_more_<P::and_<OWS, SemiColon, OWS, parameter>>
      >> {
        template <typename... Ts>
        media_range(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // accept-ext = OWS ";" OWS token [ "=" ( token / quoted-string ) ]
      struct accept_ext : P::rule<accept_ext, P::and_<
        OWS, SemiColon, OWS, token,
        P::zero_or_one_<P::and_<Equal, P::or_<token, quoted_string>>>
      >> {
        template <typename... Ts>
        accept_ext(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // accept-params = weight *accept-ext
      struct accept_params : P::rule<accept_params, P::and_<weight, P::zero_or_more_<accept_ext>>> {
        template <typename... Ts>
        accept_params(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct charset : P::rule<charset, token> {
        template <typename... Ts>
        charset(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct content_coding : P::rule<content_coding, token> {
        template <typename... Ts>
        content_coding(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // codings = content-coding / "identity" / "*"
      // Prefer literals before generic token so "*" / "identity" are distinct when used alone.
      struct codings : P::rule<codings, P::or_<identity, Asterisk, content_coding>> {
        template <typename... Ts>
        codings(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct method : P::rule<method, token> {
        template <typename... Ts>
        method(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      struct product_version : P::rule<product_version, token> {
        template <typename... Ts>
        product_version(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // product = token [ "/" product-version ]
      struct product : P::rule<product, P::and_<
        token, P::zero_or_one_<P::and_<ForwardSlash, product_version>>
      >> {
        template <typename... Ts>
        product(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // delay-seconds = 1*DIGIT
      struct delay_seconds : P::rule<delay_seconds, P::one_or_more_<DIGIT>> {
        template <typename... Ts>
        delay_seconds(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("header values")
      // Accept = [ ( "," / ( media-range [ accept-params ] ) )
      //           *( OWS "," [ OWS ( media-range [ accept-params ] ) ] ) ]
      struct accept : P::rule<accept, P::zero_or_one_<P::and_<
        P::or_<Comma, P::and_<media_range, P::zero_or_one_<accept_params>>>,
        P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<
          OWS, media_range, P::zero_or_one_<accept_params>
        >>>>
      >>> {
        template <typename... Ts>
        accept(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Accept-Charset = *( "," OWS ) ( ( charset / "*" ) [ weight ] )
      //                 *( OWS "," [ OWS ( ( charset / "*" ) [ weight ] ) ] )
      struct Accept_Charset : P::rule<Accept_Charset, P::and_<
        P::zero_or_more_<P::and_<Comma, OWS>>,
        P::or_<charset, Asterisk>, P::zero_or_one_<weight>,
        P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<
          OWS, P::or_<charset, Asterisk>, P::zero_or_one_<weight>
        >>>>
      >> {
        template <typename... Ts>
        Accept_Charset(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Accept-Encoding = [ ( "," / ( codings [ weight ] ) )
      //                    *( OWS "," [ OWS ( codings [ weight ] ) ] ) ]
      struct Accept_Encoding : P::rule<Accept_Encoding, P::zero_or_one_<P::and_<
        P::or_<Comma, P::and_<codings, P::zero_or_one_<weight>>>,
        P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<
          OWS, codings, P::zero_or_one_<weight>
        >>>>
      >>> {
        template <typename... Ts>
        Accept_Encoding(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Accept-Language = *( "," OWS ) ( language-range [ weight ] )
      //                  *( OWS "," [ OWS ( language-range [ weight ] ) ] )
      struct Accept_Language : P::rule<Accept_Language, P::and_<
        P::zero_or_more_<P::and_<Comma, OWS>>,
        language_range, P::zero_or_one_<weight>,
        P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<
          OWS, language_range, P::zero_or_one_<weight>
        >>>>
      >> {
        template <typename... Ts>
        Accept_Language(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Allow = [ ( "," / method ) *( OWS "," [ OWS method ] ) ]
      struct Allow : P::rule<Allow, P::zero_or_one_<P::and_<
        P::or_<Comma, method>,
        P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<OWS, method>>>>
      >>> {
        template <typename... Ts>
        Allow(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Content-Encoding = *( "," OWS ) content-coding *( OWS "," [ OWS content-coding ] )
      struct Content_Encoding : P::rule<Content_Encoding, P::and_<
        P::zero_or_more_<P::and_<Comma, OWS>>,
        content_coding,
        P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<OWS, content_coding>>>>
      >> {
        template <typename... Ts>
        Content_Encoding(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Content-Language = *( "," OWS ) language-tag *( OWS "," [ OWS language-tag ] )
      struct Content_Language : P::rule<Content_Language, P::and_<
        P::zero_or_more_<P::and_<Comma, OWS>>,
        language_tag,
        P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<OWS, language_tag>>>>
      >> {
        template <typename... Ts>
        Content_Language(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Content-Location = absolute-URI / partial-URI
      struct Content_Location : P::rule<Content_Location, P::or_<absolute_URI, partial_URI>> {
        template <typename... Ts>
        Content_Location(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Content-Type = media-type
      struct Content_Type : P::rule<Content_Type, media_type> {
        template <typename... Ts>
        Content_Type(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Date = HTTP-date
      struct Date : P::rule<Date, HTTP_date> {
        template <typename... Ts>
        Date(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Expect = "100-continue"
      struct Expect : P::rule<Expect, _100_continue> {
        template <typename... Ts>
        Expect(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // From = mailbox
      struct From : P::rule<From, mailbox> {
        template <typename... Ts>
        From(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Location = URI-reference
      struct Location : P::rule<Location, URI_reference> {
        template <typename... Ts>
        Location(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Max-Forwards = 1*DIGIT
      struct Max_Forwards : P::rule<Max_Forwards, P::one_or_more_<DIGIT>> {
        template <typename... Ts>
        Max_Forwards(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Referer = absolute-URI / partial-URI
      struct Referer : P::rule<Referer, P::or_<absolute_URI, partial_URI>> {
        template <typename... Ts>
        Referer(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Retry-After = HTTP-date / delay-seconds
      struct Retry_After : P::rule<Retry_After, P::or_<HTTP_date, delay_seconds>> {
        template <typename... Ts>
        Retry_After(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Server = product *( RWS ( product / comment ) )
      struct Server : P::rule<Server, P::and_<
        product, P::zero_or_more_<P::and_<RWS, P::or_<product, comment>>>
      >> {
        template <typename... Ts>
        Server(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // User-Agent = product *( RWS ( product / comment ) )
      struct User_Agent : P::rule<User_Agent, P::and_<
        product, P::zero_or_more_<P::and_<RWS, P::or_<product, comment>>>
      >> {
        template <typename... Ts>
        User_Agent(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // Vary = "*" / ( *( "," OWS ) field-name *( OWS "," [ OWS field-name ] ) )
      struct Vary : P::rule<Vary, P::or_<
        Asterisk,
        P::and_<
          P::zero_or_more_<P::and_<Comma, OWS>>,
          field_name,
          P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<OWS, field_name>>>>
        >
      >> {
        template <typename... Ts>
        Vary(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

    }
  }
}

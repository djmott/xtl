/** @file
 * RFC 7230 Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing.
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 *
 * https://tools.ietf.org/html/rfc7230
 *
 * PEG notes (ordered choice is not pure ABNF alternation):
 * - `request_target` tries origin-form, absolute-form, authority-form, then asterisk-form
 *   (ABNF order). origin-form always starts with "/"; absolute-form requires a scheme;
 *   the remaining alternatives do not share those prefixes, so ordered choice is safe.
 * - `transfer_coding` lists the named codings before `transfer_extension` so "chunked" /
 *   "gzip" / etc. are not absorbed as a bare token extension.
 * - There is no `Repeat<>` combinator in xtd::parse. Bounded counts (3DIGIT, *3DIGIT,
 *   *3"0") are hand-expanded with `and_` / `or_` / `zero_or_one_` as documented at each site.
 * - `message_body` (*OCTET) and `chunk_data` (1*OCTET) are unbounded. Content-Length /
 *   chunk-size binding is an application concern: greedy octet runs cannot leave a
 *   trailing CRLF for a later sequence element, so non-empty `chunk` framing is not
 *   reliably parseable without a length-aware layer. `last_chunk` / empty chunked bodies
 *   still match.
 * - Full-range `OCTET` is spelled `or_<OctetAscii, ObsText>` (`%x00-7F` / `%x80-FF`) because
 *   `CHARACTERS_('\x00','\xff')` is not portable when `char` is signed.
 * - `comment`/`ctext` are mutually recursive; all rule structs are forward-declared before
 *   definitions (same pattern as RFC 5322).
 * - Header rules (`Connection`, `Host`, `Transfer_Encoding`, …) match header *values* only;
 *   they do not include the field-name / ":" prefix.
 * - This grammar assumes a parser context with an EMPTY whitespace-skip set (e.g.
 *   `xtd::parser<HTTP_message, false, xtd::parse::whitespace<>>`). OWS/RWS/BWS and CRLF
 *   are matched explicitly; the default skip set of `" \t\n\r"` would break framing.
 */

#pragma once

#include "xtd/xtd.hpp"
#include "xtd/parse.hpp"
#include "xtd/grammars/RFC5234.hpp"
#include "xtd/grammars/RFC3986.hpp"

namespace xtd {
  namespace Grammars {
    namespace RFC7230 {

      namespace P = xtd::parse;

#pragma region("imports from RFC5234")
      using CR = RFC5234::CR;
      using LF = RFC5234::LF;
      using CRLF = RFC5234::CRLF;
      using SP = RFC5234::SP;
      using HTAB = RFC5234::HTAB;
      using DIGIT = RFC5234::DIGIT;
      using ALPHA = RFC5234::ALPHA;
      using VCHAR = RFC5234::VCHAR;
      using HEXDIG = RFC5234::HEXDIG;
      using DQUOTE = RFC5234::DQUOTE;
#pragma endregion

#pragma region("imports from RFC3986")
      using absolute_URI = RFC3986::absolute_URI;
      using authority = RFC3986::authority;
      using fragment = RFC3986::fragment;
      using path_abempty = RFC3986::path_abempty;
      using port = RFC3986::port;
      using query = RFC3986::query;
      using relative_part = RFC3986::relative_part;
      using scheme = RFC3986::scheme;
      using segment = RFC3986::segment;
      using uri_host = RFC3986::host;
      using URI_reference = RFC3986::URI_reference;
#pragma endregion

#pragma region("punctuation terminals")
      CHARACTER_(Colon, ':');
      CHARACTER_(ForwardSlash, '/');
      CHARACTER_(Question, '?');
      CHARACTER_(Hash, '#');
      CHARACTER_(Asterisk, '*');
      CHARACTER_(Comma, ',');
      CHARACTER_(SemiColon, ';');
      CHARACTER_(Equal, '=');
      CHARACTER_(BackSlash, '\\');
      CHARACTER_(OpenParan, '(');
      CHARACTER_(CloseParan, ')');
      CHARACTER_(Exclaimation, '!');
      CHARACTER_(Dollar, '$');
      CHARACTER_(Percent, '%');
      CHARACTER_(Ampersand, '&');
      CHARACTER_(SingleQuote, '\'');
      CHARACTER_(Plus, '+');
      CHARACTER_(Hyphen, '-');
      CHARACTER_(Period, '.');
      CHARACTER_(Caret, '^');
      CHARACTER_(UnderScore, '_');
      CHARACTER_(Grave, '`');
      CHARACTER_(Pipe, '|');
      CHARACTER_(Tilde, '~');
      CHARACTER_(_0, '0');
      CHARACTER_(_1, '1');
      CHARACTER_(_q, 'q');

      CHARACTERS_(CtextLow, '\x21', '\x27');
      CHARACTERS_(CtextMid, '\x2a', '\x5b');
      CHARACTERS_(CtextHigh, '\x5d', '\x7e');
      CHARACTER_(QdtextBang, '\x21');
      CHARACTERS_(QdtextMid, '\x23', '\x5b');
      CHARACTERS_(QdtextHigh, '\x5d', '\x7e');
      CHARACTERS_(ObsText, '\x80', '\xff');
      // Portable *OCTET: split so signed-char CHARACTERS_ ranges stay valid.
      CHARACTERS_(OctetAscii, '\x00', '\x7f');
      using any_octet = P::or_<OctetAscii, ObsText>;

      using AlphaNumeric = P::or_<ALPHA, DIGIT>;
#pragma endregion

#pragma region("string terminals")
      STRING(chunked, "chunked");
      STRING(compress, "compress");
      STRING(deflate, "deflate");
      STRING(gzip, "gzip");
      STRING(HTTP_name, "HTTP");
      STRING(http_URI_prefix, "http://");
      STRING(https_URI_prefix, "https://");
      STRING(trailers, "trailers");
#pragma endregion

#pragma region("forward declarations")
      struct Connection;
      struct Content_Length;
      struct HTTP_message;
      struct HTTP_version;
      struct Host;
      struct TE;
      struct Trailer;
      struct Transfer_Encoding;
      struct Upgrade;
      struct Via;
      struct absolute_form;
      struct absolute_path;
      struct asterisk_form;
      struct authority_form;
      struct chunk;
      struct chunk_data;
      struct chunk_ext_name;
      struct chunk_ext_val;
      struct chunk_ext;
      struct chunk_size;
      struct chunked_body;
      struct comment;
      struct connection_option;
      struct ctext;
      struct field_content;
      struct field_name;
      struct field_value;
      struct field_vchar;
      struct header_field;
      struct http_URI;
      struct https_URI;
      struct last_chunk;
      struct message_body;
      struct method;
      struct obs_fold;
      struct obs_text;
      struct origin_form;
      struct partial_URI;
      struct protocol;
      struct protocol_name;
      struct protocol_version;
      struct pseudonym;
      struct qdtext;
      struct quoted_pair;
      struct quoted_string;
      struct rank;
      struct reason_phrase;
      struct received_by;
      struct received_protocol;
      struct request_line;
      struct request_target;
      struct start_line;
      struct status_code;
      struct status_line;
      struct t_codings;
      struct t_ranking;
      struct tchar;
      struct token;
      struct trailer_part;
      struct transfer_coding;
      struct transfer_extension;
      struct transfer_parameter;
#pragma endregion

#pragma region("2.1 / 3.2.3 Whitespace")
      //- OWS = *( SP / HTAB )
      using OWS = P::zero_or_more_<P::or_<SP, HTAB>>;
      //- RWS = 1*( SP / HTAB )
      using RWS = P::one_or_more_<P::or_<SP, HTAB>>;
      //- BWS = OWS
      using BWS = OWS;
#pragma endregion

#pragma region("3.2.6.  Field Value Components")
      //- tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." /
      //          "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA
      struct tchar : P::rule<tchar, P::or_<
        Exclaimation, Hash, Dollar, Percent, Ampersand, SingleQuote, Asterisk, Plus,
        Hyphen, Period, Caret, UnderScore, Grave, Pipe, Tilde, AlphaNumeric
      >> {
        template <typename... Ts>
        tchar(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- token = 1*tchar
      struct token : P::rule<token, P::one_or_more_<tchar>> {
        template <typename... Ts>
        token(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-text = %x80-FF
      struct obs_text : P::rule<obs_text, ObsText> {
        template <typename... Ts>
        obs_text(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- qdtext = HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text
      struct qdtext : P::rule<qdtext, P::or_<
        HTAB, SP, QdtextBang, QdtextMid, QdtextHigh, obs_text
      >> {
        template <typename... Ts>
        qdtext(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- quoted-pair = "\" ( HTAB / SP / VCHAR / obs-text )
      struct quoted_pair : P::rule<quoted_pair, P::and_<
        BackSlash, P::or_<HTAB, SP, VCHAR, obs_text>
      >> {
        template <typename... Ts>
        quoted_pair(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- quoted-string = DQUOTE *( qdtext / quoted-pair ) DQUOTE
      struct quoted_string : P::rule<quoted_string, P::and_<
        DQUOTE, P::zero_or_more_<P::or_<qdtext, quoted_pair>>, DQUOTE
      >> {
        template <typename... Ts>
        quoted_string(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- ctext = HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text
      struct ctext : P::rule<ctext, P::or_<
        HTAB, SP, CtextLow, CtextMid, CtextHigh, obs_text
      >> {
        template <typename... Ts>
        ctext(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- comment = "(" *( ctext / quoted-pair / comment ) ")"
      struct comment : P::rule<comment, P::and_<
        OpenParan, P::zero_or_more_<P::or_<ctext, quoted_pair, comment>>, CloseParan
      >> {
        template <typename... Ts>
        comment(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("2.6.  Protocol Versioning")
      //- HTTP-version = HTTP-name "/" DIGIT "." DIGIT
      //- HTTP-name = %x48.54.54.50 ; "HTTP"
      struct HTTP_version : P::rule<HTTP_version, P::and_<
        HTTP_name, ForwardSlash, DIGIT, Period, DIGIT
      >> {
        template <typename... Ts>
        HTTP_version(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("2.7.  URI")
      //- absolute-path = 1*( "/" segment )
      struct absolute_path : P::rule<absolute_path, P::one_or_more_<P::and_<ForwardSlash, segment>>> {
        template <typename... Ts>
        absolute_path(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- partial-URI = relative-part [ "?" query ]
      struct partial_URI : P::rule<partial_URI, P::and_<
        relative_part, P::zero_or_one_<P::and_<Question, query>>
      >> {
        template <typename... Ts>
        partial_URI(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- http-URI = "http://" authority path-abempty [ "?" query ] [ "#" fragment ]
      struct http_URI : P::rule<http_URI, P::and_<
        http_URI_prefix, authority, path_abempty,
        P::zero_or_one_<P::and_<Question, query>>,
        P::zero_or_one_<P::and_<Hash, fragment>>
      >> {
        template <typename... Ts>
        http_URI(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- https-URI = "https://" authority path-abempty [ "?" query ] [ "#" fragment ]
      struct https_URI : P::rule<https_URI, P::and_<
        https_URI_prefix, authority, path_abempty,
        P::zero_or_one_<P::and_<Question, query>>,
        P::zero_or_one_<P::and_<Hash, fragment>>
      >> {
        template <typename... Ts>
        https_URI(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("3.1.  Start Line")
      //- method = token
      struct method : P::rule<method, token> {
        template <typename... Ts>
        method(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- origin-form = absolute-path [ "?" query ]
      struct origin_form : P::rule<origin_form, P::and_<
        absolute_path, P::zero_or_one_<P::and_<Question, query>>
      >> {
        template <typename... Ts>
        origin_form(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- absolute-form = absolute-URI
      struct absolute_form : P::rule<absolute_form, absolute_URI> {
        template <typename... Ts>
        absolute_form(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- authority-form = authority
      struct authority_form : P::rule<authority_form, authority> {
        template <typename... Ts>
        authority_form(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- asterisk-form = "*"
      struct asterisk_form : P::rule<asterisk_form, Asterisk> {
        template <typename... Ts>
        asterisk_form(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- request-target = origin-form / absolute-form / authority-form / asterisk-form
      struct request_target : P::rule<request_target, P::or_<
        origin_form, absolute_form, authority_form, asterisk_form
      >> {
        template <typename... Ts>
        request_target(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- request-line = method SP request-target SP HTTP-version CRLF
      struct request_line : P::rule<request_line, P::and_<
        method, SP, request_target, SP, HTTP_version, CRLF
      >> {
        template <typename... Ts>
        request_line(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- status-code = 3DIGIT
      struct status_code : P::rule<status_code, P::and_<DIGIT, DIGIT, DIGIT>> {
        template <typename... Ts>
        status_code(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- reason-phrase = *( HTAB / SP / VCHAR / obs-text )
      struct reason_phrase : P::rule<reason_phrase, P::zero_or_more_<P::or_<HTAB, SP, VCHAR, obs_text>>> {
        template <typename... Ts>
        reason_phrase(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- status-line = HTTP-version SP status-code SP reason-phrase CRLF
      struct status_line : P::rule<status_line, P::and_<
        HTTP_version, SP, status_code, SP, reason_phrase, CRLF
      >> {
        template <typename... Ts>
        status_line(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- start-line = request-line / status-line
      struct start_line : P::rule<start_line, P::or_<request_line, status_line>> {
        template <typename... Ts>
        start_line(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("3.2.  Header Fields")
      //- field-name = token
      struct field_name : P::rule<field_name, token> {
        template <typename... Ts>
        field_name(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- field-vchar = VCHAR / obs-text
      struct field_vchar : P::rule<field_vchar, P::or_<VCHAR, obs_text>> {
        template <typename... Ts>
        field_vchar(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- field-content = field-vchar [ 1*( SP / HTAB ) field-vchar ]
      struct field_content : P::rule<field_content, P::and_<
        field_vchar,
        P::zero_or_one_<P::and_<P::one_or_more_<P::or_<SP, HTAB>>, field_vchar>>
      >> {
        template <typename... Ts>
        field_content(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-fold = CRLF 1*( SP / HTAB )
      struct obs_fold : P::rule<obs_fold, P::and_<CRLF, P::one_or_more_<P::or_<SP, HTAB>>>> {
        template <typename... Ts>
        obs_fold(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- field-value = *( field-content / obs-fold )
      struct field_value : P::rule<field_value, P::zero_or_more_<P::or_<field_content, obs_fold>>> {
        template <typename... Ts>
        field_value(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- header-field = field-name ":" OWS field-value OWS
      struct header_field : P::rule<header_field, P::and_<
        field_name, Colon, OWS, field_value, OWS
      >> {
        template <typename... Ts>
        header_field(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("3.3.  Message Body")
      //- message-body = *OCTET
      struct message_body : P::rule<message_body, P::zero_or_more_<any_octet>> {
        template <typename... Ts>
        message_body(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- HTTP-message = start-line *( header-field CRLF ) CRLF [ message-body ]
      struct HTTP_message : P::rule<HTTP_message, P::and_<
        start_line,
        P::zero_or_more_<P::and_<header_field, CRLF>>,
        CRLF,
        P::zero_or_one_<message_body>
      >> {
        template <typename... Ts>
        HTTP_message(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("4.  Transfer Codings")
      //- transfer-parameter = token BWS "=" BWS ( token / quoted-string )
      struct transfer_parameter : P::rule<transfer_parameter, P::and_<
        token, BWS, Equal, BWS, P::or_<token, quoted_string>
      >> {
        template <typename... Ts>
        transfer_parameter(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- transfer-extension = token *( OWS ";" OWS transfer-parameter )
      struct transfer_extension : P::rule<transfer_extension, P::and_<
        token, P::zero_or_more_<P::and_<OWS, SemiColon, OWS, transfer_parameter>>
      >> {
        template <typename... Ts>
        transfer_extension(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- transfer-coding = "chunked" / "compress" / "deflate" / "gzip" / transfer-extension
      struct transfer_coding : P::rule<transfer_coding, P::or_<
        chunked, compress, deflate, gzip, transfer_extension
      >> {
        template <typename... Ts>
        transfer_coding(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      // rank = ( "0" [ "." *3DIGIT ] ) / ( "1" [ "." *3"0" ] )
      using up_to_3DIGIT = P::zero_or_one_<P::and_<
        DIGIT, P::zero_or_one_<P::and_<DIGIT, P::zero_or_one_<DIGIT>>>
      >>;
      using up_to_3zero = P::zero_or_one_<P::and_<
        _0, P::zero_or_one_<P::and_<_0, P::zero_or_one_<_0>>>
      >>;
      struct rank : P::rule<rank, P::or_<
        P::and_<_0, P::zero_or_one_<P::and_<Period, up_to_3DIGIT>>>,
        P::and_<_1, P::zero_or_one_<P::and_<Period, up_to_3zero>>>
      >> {
        template <typename... Ts>
        rank(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- t-ranking = OWS ";" OWS "q=" rank
      struct t_ranking : P::rule<t_ranking, P::and_<OWS, SemiColon, OWS, _q, Equal, rank>> {
        template <typename... Ts>
        t_ranking(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- t-codings = "trailers" / ( transfer-coding [ t-ranking ] )
      struct t_codings : P::rule<t_codings, P::or_<
        trailers, P::and_<transfer_coding, P::zero_or_one_<t_ranking>>
      >> {
        template <typename... Ts>
        t_codings(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("4.1.  Chunked Transfer Coding")
      //- chunk-size = 1*HEXDIG
      struct chunk_size : P::rule<chunk_size, P::one_or_more_<HEXDIG>> {
        template <typename... Ts>
        chunk_size(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- chunk-ext-name = token
      struct chunk_ext_name : P::rule<chunk_ext_name, token> {
        template <typename... Ts>
        chunk_ext_name(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- chunk-ext-val = token / quoted-string
      struct chunk_ext_val : P::rule<chunk_ext_val, P::or_<token, quoted_string>> {
        template <typename... Ts>
        chunk_ext_val(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- chunk-ext = *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
      struct chunk_ext : P::rule<chunk_ext, P::zero_or_more_<P::and_<
        SemiColon, chunk_ext_name, P::zero_or_one_<P::and_<Equal, chunk_ext_val>>
      >>> {
        template <typename... Ts>
        chunk_ext(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- chunk-data = 1*OCTET
      struct chunk_data : P::rule<chunk_data, P::one_or_more_<any_octet>> {
        template <typename... Ts>
        chunk_data(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- chunk = chunk-size [ chunk-ext ] CRLF chunk-data CRLF
      struct chunk : P::rule<chunk, P::and_<
        chunk_size, P::zero_or_one_<chunk_ext>, CRLF, chunk_data, CRLF
      >> {
        template <typename... Ts>
        chunk(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- last-chunk = 1*("0") [ chunk-ext ] CRLF
      struct last_chunk : P::rule<last_chunk, P::and_<
        P::one_or_more_<_0>, P::zero_or_one_<chunk_ext>, CRLF
      >> {
        template <typename... Ts>
        last_chunk(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- trailer-part = *( header-field CRLF )
      struct trailer_part : P::rule<trailer_part, P::zero_or_more_<P::and_<header_field, CRLF>>> {
        template <typename... Ts>
        trailer_part(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- chunked-body = *chunk last-chunk trailer-part CRLF
      struct chunked_body : P::rule<chunked_body, P::and_<
        P::zero_or_more_<chunk>, last_chunk, trailer_part, CRLF
      >> {
        template <typename... Ts>
        chunked_body(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("5.4 / 6.1 / Appendix A — header values")
      //- Host = uri-host [ ":" port ]
      struct Host : P::rule<Host, P::and_<uri_host, P::zero_or_one_<P::and_<Colon, port>>>> {
        template <typename... Ts>
        Host(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- connection-option = token
      struct connection_option : P::rule<connection_option, token> {
        template <typename... Ts>
        connection_option(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- Connection = *( "," OWS ) connection-option *( OWS "," [ OWS connection-option ] )
      struct Connection : P::rule<Connection, P::and_<
        P::zero_or_more_<P::and_<Comma, OWS>>,
        connection_option,
        P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<OWS, connection_option>>>>
      >> {
        template <typename... Ts>
        Connection(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- Content-Length = 1*DIGIT
      struct Content_Length : P::rule<Content_Length, P::one_or_more_<DIGIT>> {
        template <typename... Ts>
        Content_Length(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- TE = [ ( "," / t-codings ) *( OWS "," [ OWS t-codings ] ) ]
      struct TE : P::rule<TE, P::zero_or_one_<P::and_<
        P::or_<Comma, t_codings>,
        P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<OWS, t_codings>>>>
      >>> {
        template <typename... Ts>
        TE(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- Trailer = *( "," OWS ) field-name *( OWS "," [ OWS field-name ] )
      struct Trailer : P::rule<Trailer, P::and_<
        P::zero_or_more_<P::and_<Comma, OWS>>,
        field_name,
        P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<OWS, field_name>>>>
      >> {
        template <typename... Ts>
        Trailer(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- Transfer-Encoding = *( "," OWS ) transfer-coding *( OWS "," [ OWS transfer-coding ] )
      struct Transfer_Encoding : P::rule<Transfer_Encoding, P::and_<
        P::zero_or_more_<P::and_<Comma, OWS>>,
        transfer_coding,
        P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<OWS, transfer_coding>>>>
      >> {
        template <typename... Ts>
        Transfer_Encoding(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- protocol-name = token
      struct protocol_name : P::rule<protocol_name, token> {
        template <typename... Ts>
        protocol_name(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- protocol-version = token
      struct protocol_version : P::rule<protocol_version, token> {
        template <typename... Ts>
        protocol_version(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- protocol = protocol-name [ "/" protocol-version ]
      struct protocol : P::rule<protocol, P::and_<
        protocol_name, P::zero_or_one_<P::and_<ForwardSlash, protocol_version>>
      >> {
        template <typename... Ts>
        protocol(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- Upgrade = *( "," OWS ) protocol *( OWS "," [ OWS protocol ] )
      struct Upgrade : P::rule<Upgrade, P::and_<
        P::zero_or_more_<P::and_<Comma, OWS>>,
        protocol,
        P::zero_or_more_<P::and_<OWS, Comma, P::zero_or_one_<P::and_<OWS, protocol>>>>
      >> {
        template <typename... Ts>
        Upgrade(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- pseudonym = token
      struct pseudonym : P::rule<pseudonym, token> {
        template <typename... Ts>
        pseudonym(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- received-protocol = [ protocol-name "/" ] protocol-version
      struct received_protocol : P::rule<received_protocol, P::and_<
        P::zero_or_one_<P::and_<protocol_name, ForwardSlash>>, protocol_version
      >> {
        template <typename... Ts>
        received_protocol(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- received-by = ( uri-host [ ":" port ] ) / pseudonym
      struct received_by : P::rule<received_by, P::or_<
        P::and_<uri_host, P::zero_or_one_<P::and_<Colon, port>>>,
        pseudonym
      >> {
        template <typename... Ts>
        received_by(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- Via = *( "," OWS ) received-protocol RWS received-by [ RWS comment ]
      //       *( OWS "," [ OWS received-protocol RWS received-by [ RWS comment ] ] )
      struct Via : P::rule<Via, P::and_<
        P::zero_or_more_<P::and_<Comma, OWS>>,
        received_protocol, RWS, received_by, P::zero_or_one_<P::and_<RWS, comment>>,
        P::zero_or_more_<P::and_<
          OWS, Comma,
          P::zero_or_one_<P::and_<
            OWS, received_protocol, RWS, received_by, P::zero_or_one_<P::and_<RWS, comment>>
          >>
        >>
      >> {
        template <typename... Ts>
        Via(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

    }
  }
}

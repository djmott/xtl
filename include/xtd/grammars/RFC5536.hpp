/** @file
 * RFC 5536 Netnews Article Format.
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 *
 * https://tools.ietf.org/html/rfc5536
 *
 * PEG notes (ordered choice is not pure ABNF alternation):
 * - `path_diagnostic = diag-match / diag-other / diag-deprecated` is reordered to
 *   `or_<diag_other, diag_deprecated, diag_match>`: `diag-match` is the bare literal "!", a strict
 *   prefix of both `diag-other` ("!." ...) and `diag-deprecated` ("!" IPv4address ...). Trying
 *   `diag_match` first would consume the lone "!" and leave the following "." or IPv4address
 *   unconsumed, then fail the mandatory trailing "!" required by `path-list`'s enclosing `and_<>`
 *   (which backtracks the whole item, not just the diagnostic choice). Longest/most-constrained
 *   alternatives are therefore tried first, matching the general PEG convention used throughout
 *   this library's grammars.
 * - `toplabel`'s three ABNF alternatives are tried in their original textual order. The first two
 *   alternatives both begin with an optional `label` run, so a `toplabel` consisting of exactly two
 *   labels joined by 1*"-" is representable by alternative 1 or 2 as well as alternative 3; ordered
 *   choice picks whichever succeeds first, which is always a valid parse per the ABNF (the
 *   alternatives are not disjoint, only their union matters for acceptance).
 * - `value = token / quoted-string` (RFC 2045 §5.1, as referenced by `parameter`) has no prefix
 *   ambiguity: `token` cannot start with DQUOTE, so ordered choice here is equivalent to alternation.
 * - `id-right = dot-atom-text / no-fold-literal` has no prefix ambiguity: `no-fold-literal` starts
 *   with "[", which is not a valid `atext`, so `dot-atom-text` cannot partially consume it.
 * - This grammar assumes a parser context with an EMPTY whitespace-skip set (e.g.
 *   `xtd::parser<article, false, xtd::parse::whitespace<>>`), for the same reason documented in
 *   RFC5322.hpp: all folding/whitespace is matched explicitly via WSP/FWS/CFWS, and CRLF is
 *   semantically significant, so the default `xtd::parser<>` whitespace set would silently swallow
 *   it ahead of every terminal match.
 *
 * Section 2.2 requirements (restrictions this grammar enforces relative to RFC 5322) that shape the
 * rules below:
 * - Every header field body defined or referenced by this document MUST contain at least one
 *   non-whitespace character (`unstructured` is redefined here as `*WSP VCHAR *([FWS] VCHAR) *WSP`,
 *   disallowing the empty match that RFC 5322's `unstructured` permits).
 * - A SP is REQUIRED immediately after the ':' separating a header field name from its body. Rather
 *   than baking the space into the STRING prefix terminal (as RFC5322.hpp does for its own,
 *   non-space-mandating prefixes), each `*_prefix` STRING here covers only up to and including the
 *   ':', with an explicit `SP` combinator immediately following it in every rule, per the ABNF
 *   given in RFC 5536 (e.g. `orig-date = "Date:" SP date-time CRLF`).
 * - `msg-id` is redefined (Section 3.1.3) to disallow CFWS/comments and to bound `id-right` to
 *   `dot-atom-text / no-fold-literal` (dropping RFC 5322's obsolete forms); this stricter `msg-id`
 *   also applies wherever message identifiers are referenced below (References, Supersedes).
 * - The `References` header field requires CFWS (not `[CFWS]`) between message identifiers.
 * - `Control`, `Distribution`, `Followup-To`, `Lines`, `Newsgroups`, `Path`, `Supersedes`, `Xref`,
 *   and `Message-ID` disallow comments entirely, using FWS/WSP rather than CFWS for folding.
 * - `archive-param` and the parameters of `Injection-Info` are simplified here to a generic
 *   `attribute "=" value` (`value = token / quoted-string`) rather than the full RFC 2045/2231
 *   parameter grammar (continuations, charset/language extended values), since this grammar does
 *   not otherwise model MIME parameter encoding.
 */

#pragma once

#include "xtd/xtd.hpp"
#include "xtd/parse.hpp"
#include "xtd/grammars/RFC5322.hpp"
#include "xtd/grammars/RFC3986.hpp"

namespace xtd {
  namespace Grammars {
    namespace RFC5536 {

      namespace P = xtd::parse;

#pragma region("imports from RFC5322 (and transitively RFC5234)")
      using CRLF = RFC5322::CRLF;
      using SP = RFC5322::SP;
      using WSP = RFC5322::WSP;
      using FWS = RFC5322::FWS;
      using CFWS = RFC5322::CFWS;
      using DIGIT = RFC5322::DIGIT;
      using ALPHA = RFC5322::ALPHA;
      using VCHAR = RFC5322::VCHAR;
      using quoted_string = RFC5322::quoted_string;
      using dot_atom_text = RFC5322::dot_atom_text;
      using phrase = RFC5322::phrase;
      using date_time = RFC5322::date_time;
      using mailbox = RFC5322::mailbox;
      using mailbox_list = RFC5322::mailbox_list;
      using address_list = RFC5322::address_list;
      using body = RFC5322::body;
#pragma endregion

#pragma region("imports from RFC3986 (for Path diagnostics)")
      using IPv4address = RFC3986::IPv4address;
      using IPv6address = RFC3986::IPv6address;
#pragma endregion

#pragma region("punctuation terminals")
      CHARACTER_(Colon, ':');
      CHARACTER_(SemiColon, ';');
      CHARACTER_(Exclaimation, '!');
      CHARACTER_(Period, '.');
      CHARACTER_(Hyphen, '-');
      CHARACTER_(UnderScore, '_');
      CHARACTER_(Plus, '+');
      CHARACTER_(OpenBracket, '[');
      CHARACTER_(CloseBracket, ']');
      CHARACTER_(OpenBrace, '{');
      CHARACTER_(CloseBrace, '}');
      CHARACTER_(LessThan, '<');
      CHARACTER_(GreaterThan, '>');
      CHARACTER_(At, '@');
      CHARACTER_(Comma, ',');
      CHARACTER_(Equal, '=');
      CHARACTER_(ForwardSlash, '/');
      CHARACTER_(Hash, '#');
      CHARACTER_(Dollar, '$');
      CHARACTER_(Percent, '%');
      CHARACTER_(Ampersand, '&');
      CHARACTER_(SingleQuote, '\'');
      CHARACTER_(Asterisk, '*');
      CHARACTER_(Caret, '^');
      CHARACTER_(Backtick, '`');
      CHARACTER_(Pipe, '|');
      CHARACTER_(Tilde, '~');
#pragma endregion

#pragma region("character-class terminals")
      // mdtext = %d33-61 / %d63-90 / %d94-126 (i.e. printable US-ASCII excluding '>', '[', ']', '\')
      CHARACTERS_(MdtextLow, '\x21', '\x3d');
      CHARACTERS_(MdtextMid, '\x3f', '\x5a');
      CHARACTERS_(MdtextHigh, '\x5e', '\x7e');

      // article-locator = 1*( %x21-27 / %x29-3A / %x3C-7E ) (printable US-ASCII excluding '(' and ';')
      CHARACTERS_(LocatorLow, '\x21', '\x27');
      CHARACTERS_(LocatorMid, '\x29', '\x3a');
      CHARACTERS_(LocatorHigh, '\x3c', '\x7e');
#pragma endregion

#pragma region("header field name prefixes, section 3 (colon only; callers add the mandatory SP)")
      STRING(orig_date_prefix, "Date:");
      STRING(from_prefix, "From:");
      STRING(message_id_prefix, "Message-ID:");
      STRING(newsgroups_prefix, "Newsgroups:");
      STRING(path_prefix, "Path:");
      STRING(subject_prefix, "Subject:");

      STRING(approved_prefix, "Approved:");
      STRING(archive_prefix, "Archive:");
      STRING(control_prefix, "Control:");
      STRING(distribution_prefix, "Distribution:");
      STRING(expires_prefix, "Expires:");
      STRING(followup_to_prefix, "Followup-To:");
      STRING(injection_date_prefix, "Injection-Date:");
      STRING(injection_info_prefix, "Injection-Info:");
      STRING(lines_prefix, "Lines:");
      STRING(organization_prefix, "Organization:");
      STRING(references_prefix, "References:");
      STRING(summary_prefix, "Summary:");
      STRING(supersedes_prefix, "Supersedes:");
      STRING(user_agent_prefix, "User-Agent:");
      STRING(xref_prefix, "Xref:");

      STRING(comments_prefix, "Comments:");
      STRING(keywords_prefix, "Keywords:");
      STRING(reply_to_prefix, "Reply-To:");
      STRING(sender_prefix, "Sender:");
#pragma endregion

#pragma region("miscellaneous string literals")
      STRING(archive_no, "no");
      STRING(archive_yes, "yes");
      STRING(poster_literal, "poster"); // MUST be generated lowercase; see Followup-To
#pragma endregion

#pragma region("2.2 Restricted unstructured (stricter than RFC5322: forbids the empty match)")
      //- unstructured = *WSP VCHAR *( [FWS] VCHAR ) *WSP
      struct unstructured : P::rule<unstructured, P::and_<
        P::zero_or_more_<WSP>, VCHAR,
        P::zero_or_more_<P::and_<P::zero_or_one_<FWS>, VCHAR>>,
        P::zero_or_more_<WSP>
      >> {
        template <typename... Ts>
        unstructured(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("generic token / parameter grammar (shared by Control, User-Agent, Archive, Injection-Info)")
      // tchar: a pragmatic 5536 "token" character - ALPHA/DIGIT plus specials, including "{" and "}"
      // (unlike HTTP's token, per the note in Section 3.2.13), but excluding "/" and "=" since those
      // are used as structural separators in `product` and `parameter` respectively.
      using tchar = P::or_<ALPHA, DIGIT, Exclaimation, Hash, Dollar, Percent, Ampersand, SingleQuote,
        Asterisk, Plus, Hyphen, Period, Caret, UnderScore, Backtick, OpenBrace, CloseBrace, Pipe, Tilde>;

      //- token = 1*tchar
      struct token : P::rule<token, P::one_or_more_<tchar>> {
        template <typename... Ts>
        token(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- attribute = token
      struct attribute : P::rule<attribute, token> {
        template <typename... Ts>
        attribute(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- value = token / quoted-string  (RFC 2045 5.1, simplified; no prefix ambiguity, see file header)
      struct value : P::rule<value, P::or_<token, quoted_string>> {
        template <typename... Ts>
        value(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- parameter = attribute [CFWS] "=" [CFWS] value  (simplified generic RFC 2045/2231 parameter)
      struct parameter : P::rule<parameter, P::and_<
        attribute, P::zero_or_one_<CFWS>, Equal, P::zero_or_one_<CFWS>, value
      >> {
        template <typename... Ts>
        parameter(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("3.1 Mandatory Header Fields")
      //+ 3.1.3.  Message-ID (message-id syntax, stricter than RFC5322 msg-id)
      //- mdtext = %d33-61 / %d63-90 / %d94-126
      using mdtext = P::or_<MdtextLow, MdtextMid, MdtextHigh>;

      //- no-fold-literal = "[" *mdtext "]"
      struct no_fold_literal : P::rule<no_fold_literal, P::and_<
        OpenBracket, P::zero_or_more_<mdtext>, CloseBracket
      >> {
        template <typename... Ts>
        no_fold_literal(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- id-left = dot-atom-text
      struct id_left : P::rule<id_left, dot_atom_text> {
        template <typename... Ts>
        id_left(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- id-right = dot-atom-text / no-fold-literal
      struct id_right : P::rule<id_right, P::or_<dot_atom_text, no_fold_literal>> {
        template <typename... Ts>
        id_right(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- msg-id-core = id-left "@" id-right
      struct msg_id_core : P::rule<msg_id_core, P::and_<id_left, At, id_right>> {
        template <typename... Ts>
        msg_id_core(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- msg-id = "<" msg-id-core ">"  (NO CFWS; max length 250 octets, not enforced - see file header)
      struct msg_id : P::rule<msg_id, P::and_<LessThan, msg_id_core, GreaterThan>> {
        template <typename... Ts>
        msg_id(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- message-id = "Message-ID:" SP *WSP msg-id *WSP CRLF
      struct message_id : P::rule<message_id, P::and_<
        message_id_prefix, SP, P::zero_or_more_<WSP>, msg_id, P::zero_or_more_<WSP>, CRLF
      >> {
        template <typename... Ts>
        message_id(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.1.1.  Date
      //- orig-date = "Date:" SP date-time CRLF
      //  date-time is RFC5322's; its zone alternative already accepts obsolete "GMT" via obs-zone,
      //  which Section 2.1 of this document requires agents to accept.
      struct orig_date : P::rule<orig_date, P::and_<orig_date_prefix, SP, date_time, CRLF>> {
        template <typename... Ts>
        orig_date(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.1.2.  From
      //- from = "From:" SP mailbox-list CRLF
      struct from : P::rule<from, P::and_<from_prefix, SP, mailbox_list, CRLF>> {
        template <typename... Ts>
        from(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.1.4.  Newsgroups
      //- component-char = ALPHA / DIGIT / "+" / "-" / "_"
      using component_char = P::or_<ALPHA, DIGIT, Plus, Hyphen, UnderScore>;

      //- component = 1*component-char
      struct component : P::rule<component, P::one_or_more_<component_char>> {
        template <typename... Ts>
        component(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- newsgroup-name = component *( "." component )
      struct newsgroup_name : P::rule<newsgroup_name, P::and_<
        component, P::zero_or_more_<P::and_<Period, component>>
      >> {
        template <typename... Ts>
        newsgroup_name(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- newsgroup-list = *WSP newsgroup-name *( [FWS] "," [FWS] newsgroup-name ) *WSP
      struct newsgroup_list : P::rule<newsgroup_list, P::and_<
        P::zero_or_more_<WSP>, newsgroup_name,
        P::zero_or_more_<P::and_<P::zero_or_one_<FWS>, Comma, P::zero_or_one_<FWS>, newsgroup_name>>,
        P::zero_or_more_<WSP>
      >> {
        template <typename... Ts>
        newsgroup_list(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- newsgroups = "Newsgroups:" SP newsgroup-list CRLF
      struct newsgroups : P::rule<newsgroups, P::and_<newsgroups_prefix, SP, newsgroup_list, CRLF>> {
        template <typename... Ts>
        newsgroups(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.1.5.  Path
      //- alphanum = ALPHA / DIGIT
      using alphanum = P::or_<ALPHA, DIGIT>;

      //- path-nodot = 1*( alphanum / "-" / "_" )
      struct path_nodot : P::rule<path_nodot, P::one_or_more_<P::or_<alphanum, Hyphen, UnderScore>>> {
        template <typename... Ts>
        path_nodot(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- label = alphanum [ *( alphanum / "-" ) alphanum ]
      //  PEG-friendly (same approach as RFC1035.hpp): greedy *(alphanum/-) before a trailing
      //  alphanum would leave nothing for that trailing atom, so the optional group always fails
      //  and label would match only one character. Accept "alphanum *(alphanum / -)" instead
      //  (also allows a trailing hyphen, which strict RFC 5536 forbids).
      struct label : P::rule<label, P::and_<alphanum, P::zero_or_more_<P::or_<alphanum, Hyphen>>>> {
        template <typename... Ts>
        label(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- toplabel: must contain ALPHA (RFC 5536). PEG-safe form: ALPHA *(alphanum / "-")
      //  (leading digits-then-alpha forms are not generated in practice for path identities).
      struct toplabel : P::rule<toplabel, P::and_<ALPHA, P::zero_or_more_<P::or_<alphanum, Hyphen>>>> {
        template <typename... Ts>
        toplabel(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- path-identity = ( 1*( label "." ) toplabel ) / path-nodot
      struct path_identity : P::rule<path_identity, P::or_<
        P::and_<P::one_or_more_<P::and_<label, Period>>, toplabel>,
        path_nodot
      >> {
        template <typename... Ts>
        path_identity(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- diag-keyword = 1*ALPHA
      struct diag_keyword : P::rule<diag_keyword, P::one_or_more_<ALPHA>> {
        template <typename... Ts>
        diag_keyword(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- diag-identity = path-identity / IPv4address / IPv6address
      struct diag_identity : P::rule<diag_identity, P::or_<path_identity, IPv4address, IPv6address>> {
        template <typename... Ts>
        diag_identity(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- diag-match = "!"
      struct diag_match : P::rule<diag_match, Exclaimation> {
        template <typename... Ts>
        diag_match(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- diag-other = "!." diag-keyword [ "." diag-identity ] [FWS]
      struct diag_other : P::rule<diag_other, P::and_<
        Exclaimation, Period, diag_keyword,
        P::zero_or_one_<P::and_<Period, diag_identity>>, P::zero_or_one_<FWS>
      >> {
        template <typename... Ts>
        diag_other(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- diag-deprecated = "!" IPv4address [FWS]
      struct diag_deprecated : P::rule<diag_deprecated, P::and_<
        Exclaimation, IPv4address, P::zero_or_one_<FWS>
      >> {
        template <typename... Ts>
        diag_deprecated(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- path-diagnostic = diag-match / diag-other / diag-deprecated  (reordered - see file header)
      struct path_diagnostic : P::rule<path_diagnostic, P::or_<diag_other, diag_deprecated, diag_match>> {
        template <typename... Ts>
        path_diagnostic(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- path-list = *( path-identity [FWS] [path-diagnostic] "!" )
      //  PEG: bare `diag-match` ("!") must not steal the mandatory trailing "!".
      //  Encode as: identity [FWS] ( diag-other "!" / diag-deprecated "!" / "!!" / "!" )
      using path_list_item = P::and_<
        path_identity,
        P::zero_or_one_<FWS>,
        P::or_<
          P::and_<diag_other, Exclaimation>,
          P::and_<diag_deprecated, Exclaimation>,
          P::and_<Exclaimation, Exclaimation>,
          Exclaimation
        >
      >;
      struct path_list : P::rule<path_list, P::zero_or_more_<path_list_item>> {
        template <typename... Ts>
        path_list(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- tail-entry = path-nodot  (may be the literal string "not-for-mail")
      struct tail_entry : P::rule<tail_entry, path_nodot> {
        template <typename... Ts>
        tail_entry(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- path = "Path:" SP *WSP path-list tail-entry *WSP CRLF
      struct path : P::rule<path, P::and_<
        path_prefix, SP, P::zero_or_more_<WSP>, path_list, tail_entry, P::zero_or_more_<WSP>, CRLF
      >> {
        template <typename... Ts>
        path(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.1.6.  Subject
      //- subject = "Subject:" SP unstructured CRLF
      struct subject : P::rule<subject, P::and_<subject_prefix, SP, unstructured, CRLF>> {
        template <typename... Ts>
        subject(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("3.2 Optional Header Fields")
      //+ 3.2.1.  Approved
      //- approved = "Approved:" SP mailbox-list CRLF
      struct approved : P::rule<approved, P::and_<approved_prefix, SP, mailbox_list, CRLF>> {
        template <typename... Ts>
        approved(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.2.2.  Archive
      //- archive-param = parameter
      struct archive_param : P::rule<archive_param, parameter> {
        template <typename... Ts>
        archive_param(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- archive = "Archive:" SP [CFWS] ("no" / "yes") *( [CFWS] ";" [CFWS] archive-param ) [CFWS] CRLF
      struct archive : P::rule<archive, P::and_<
        archive_prefix, SP, P::zero_or_one_<CFWS>, P::or_<archive_no, archive_yes>,
        P::zero_or_more_<P::and_<
          P::zero_or_one_<CFWS>, SemiColon, P::zero_or_one_<CFWS>, archive_param
        >>,
        P::zero_or_one_<CFWS>, CRLF
      >> {
        template <typename... Ts>
        archive(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.2.3.  Control
      //- verb = token
      struct verb : P::rule<verb, token> {
        template <typename... Ts>
        verb(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- argument = 1*( %x21-7E )  (== 1*VCHAR)
      struct argument : P::rule<argument, P::one_or_more_<VCHAR>> {
        template <typename... Ts>
        argument(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- control-command = verb *( 1*WSP argument )
      struct control_command : P::rule<control_command, P::and_<
        verb, P::zero_or_more_<P::and_<P::one_or_more_<WSP>, argument>>
      >> {
        template <typename... Ts>
        control_command(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- control = "Control:" SP *WSP control-command *WSP CRLF
      struct control : P::rule<control, P::and_<
        control_prefix, SP, P::zero_or_more_<WSP>, control_command, P::zero_or_more_<WSP>, CRLF
      >> {
        template <typename... Ts>
        control(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.2.4.  Distribution
      //- dist-name = ALPHA / DIGIT *( ALPHA / DIGIT / "+" / "-" / "_" )  (first char excludes +/-/_)
      struct dist_name : P::rule<dist_name, P::and_<
        P::or_<ALPHA, DIGIT>, P::zero_or_more_<component_char>
      >> {
        template <typename... Ts>
        dist_name(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- dist-list = *WSP dist-name *( [FWS] "," [FWS] dist-name ) *WSP
      struct dist_list : P::rule<dist_list, P::and_<
        P::zero_or_more_<WSP>, dist_name,
        P::zero_or_more_<P::and_<P::zero_or_one_<FWS>, Comma, P::zero_or_one_<FWS>, dist_name>>,
        P::zero_or_more_<WSP>
      >> {
        template <typename... Ts>
        dist_list(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- distribution = "Distribution:" SP dist-list CRLF
      struct distribution : P::rule<distribution, P::and_<distribution_prefix, SP, dist_list, CRLF>> {
        template <typename... Ts>
        distribution(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.2.5.  Expires
      //- expires = "Expires:" SP date-time CRLF
      struct expires : P::rule<expires, P::and_<expires_prefix, SP, date_time, CRLF>> {
        template <typename... Ts>
        expires(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.2.6.  Followup-To
      //- poster-text = *WSP %d112.111.115.116.101.114 *WSP  ("poster", MUST be generated lowercase)
      struct poster_text : P::rule<poster_text, P::and_<
        P::zero_or_more_<WSP>, poster_literal, P::zero_or_more_<WSP>
      >> {
        template <typename... Ts>
        poster_text(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- followup-to = "Followup-To:" SP ( newsgroup-list / poster-text ) CRLF
      struct followup_to : P::rule<followup_to, P::and_<
        followup_to_prefix, SP, P::or_<newsgroup_list, poster_text>, CRLF
      >> {
        template <typename... Ts>
        followup_to(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.2.7.  Injection-Date
      //- injection-date = "Injection-Date:" SP date-time CRLF
      struct injection_date : P::rule<injection_date, P::and_<injection_date_prefix, SP, date_time, CRLF>> {
        template <typename... Ts>
        injection_date(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.2.8.  Injection-Info
      //- injection-info = "Injection-Info:" SP [CFWS] path-identity [CFWS]
      //                   *( ";" [CFWS] parameter ) [CFWS] CRLF
      //  Named parameters ("posting-host", "posting-account", "logging-data", "mail-complaints-to")
      //  are accepted generically via `parameter` rather than individually modeled.
      struct injection_info : P::rule<injection_info, P::and_<
        injection_info_prefix, SP, P::zero_or_one_<CFWS>, path_identity, P::zero_or_one_<CFWS>,
        P::zero_or_more_<P::and_<SemiColon, P::zero_or_one_<CFWS>, parameter>>,
        P::zero_or_one_<CFWS>, CRLF
      >> {
        template <typename... Ts>
        injection_info(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.2.9.  Organization
      //- organization = "Organization:" SP unstructured CRLF
      struct organization : P::rule<organization, P::and_<organization_prefix, SP, unstructured, CRLF>> {
        template <typename... Ts>
        organization(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.2.10.  References
      //- references = "References:" SP [CFWS] msg-id *(CFWS msg-id) [CFWS] CRLF
      //  Unlike RFC5322, CFWS between message identifiers is REQUIRED (not optional).
      struct references : P::rule<references, P::and_<
        references_prefix, SP, P::zero_or_one_<CFWS>, msg_id,
        P::zero_or_more_<P::and_<CFWS, msg_id>>, P::zero_or_one_<CFWS>, CRLF
      >> {
        template <typename... Ts>
        references(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.2.11.  Summary
      //- summary = "Summary:" SP unstructured CRLF
      struct summary : P::rule<summary, P::and_<summary_prefix, SP, unstructured, CRLF>> {
        template <typename... Ts>
        summary(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.2.12.  Supersedes
      //- supersedes = "Supersedes:" SP *WSP msg-id *WSP CRLF
      struct supersedes : P::rule<supersedes, P::and_<
        supersedes_prefix, SP, P::zero_or_more_<WSP>, msg_id, P::zero_or_more_<WSP>, CRLF
      >> {
        template <typename... Ts>
        supersedes(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.2.13.  User-Agent
      //- product-version = [CFWS] token
      struct product_version : P::rule<product_version, P::and_<P::zero_or_one_<CFWS>, token>> {
        template <typename... Ts>
        product_version(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- product = [CFWS] token [ [CFWS] "/" product-version ]
      struct product : P::rule<product, P::and_<
        P::zero_or_one_<CFWS>, token,
        P::zero_or_one_<P::and_<P::zero_or_one_<CFWS>, ForwardSlash, product_version>>
      >> {
        template <typename... Ts>
        product(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- user-agent = "User-Agent:" SP 1*product [CFWS] CRLF
      struct user_agent : P::rule<user_agent, P::and_<
        user_agent_prefix, SP, P::one_or_more_<product>, P::zero_or_one_<CFWS>, CRLF
      >> {
        template <typename... Ts>
        user_agent(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.2.14.  Xref
      //- server-name = path-identity
      struct server_name : P::rule<server_name, path_identity> {
        template <typename... Ts>
        server_name(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- article-locator = 1*( %x21-27 / %x29-3A / %x3C-7E )
      struct article_locator : P::rule<article_locator, P::one_or_more_<
        P::or_<LocatorLow, LocatorMid, LocatorHigh>
      >> {
        template <typename... Ts>
        article_locator(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- location = newsgroup-name ":" article-locator
      struct location : P::rule<location, P::and_<newsgroup_name, Colon, article_locator>> {
        template <typename... Ts>
        location(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- xref = "Xref:" SP *WSP server-name 1*( FWS location ) *WSP CRLF
      struct xref : P::rule<xref, P::and_<
        xref_prefix, SP, P::zero_or_more_<WSP>, server_name,
        P::one_or_more_<P::and_<FWS, location>>, P::zero_or_more_<WSP>, CRLF
      >> {
        template <typename... Ts>
        xref(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("3.3 Obsolete Header Fields")
      //+ 3.3.1.  Lines
      //- lines = "Lines:" SP *WSP 1*DIGIT *WSP CRLF
      struct lines : P::rule<lines, P::and_<
        lines_prefix, SP, P::zero_or_more_<WSP>, P::one_or_more_<DIGIT>, P::zero_or_more_<WSP>, CRLF
      >> {
        template <typename... Ts>
        lines(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("RFC5322-style optional headers, redefined here with the mandatory SP after colon")
      //- comments = "Comments:" SP unstructured CRLF
      struct comments : P::rule<comments, P::and_<comments_prefix, SP, unstructured, CRLF>> {
        template <typename... Ts>
        comments(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- keywords = "Keywords:" SP phrase *("," phrase) CRLF  (multiple occurrences not permitted)
      struct keywords : P::rule<keywords, P::and_<
        keywords_prefix, SP, phrase, P::zero_or_more_<P::and_<Comma, phrase>>, CRLF
      >> {
        template <typename... Ts>
        keywords(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- reply-to = "Reply-To:" SP address-list CRLF
      struct reply_to : P::rule<reply_to, P::and_<reply_to_prefix, SP, address_list, CRLF>> {
        template <typename... Ts>
        reply_to(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- sender = "Sender:" SP mailbox CRLF
      struct sender : P::rule<sender, P::and_<sender_prefix, SP, mailbox, CRLF>> {
        template <typename... Ts>
        sender(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("3. News Header Fields - fields extension, and the article as a whole")
      /*
      fields =/ *( approved / archive / control / distribution / expires / followup-to /
                   injection-date / injection-info / lines / newsgroups / organization / path /
                   summary / supersedes / user-agent / xref )
      This is the news-specific extension to [RFC5322]'s `fields`; it deliberately excludes Date,
      From, Message-ID, and Subject (redefined by name above, not additive) and Comments, Keywords,
      Reply-To, and Sender (reused from [RFC5322] with the added SP restriction above).
      */
      using fields_alt = P::or_<
        approved, archive, control, distribution, expires, followup_to, injection_date, injection_info,
        lines, newsgroups, organization, path, summary, supersedes, user_agent, xref
      >;

      struct fields : P::rule<fields, P::zero_or_more_<fields_alt>> {
        template <typename... Ts>
        fields(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      /*
      article_header = any mandatory or optional news header defined above, any of the four
      [RFC5322] mail header fields reused with the mandatory SP, or (as a fallback for headers this
      grammar does not model by name, e.g. MIME headers such as Content-Type) an [RFC5322]
      optional-field. The fallback is listed last so named productions are always preferred.
      */
      using article_header_alt = P::or_<
        orig_date, from, message_id, newsgroups, path, subject,
        fields_alt,
        comments, keywords, reply_to, sender,
        RFC5322::optional_field
      >;

      struct article_header : P::rule<article_header, article_header_alt> {
        template <typename... Ts>
        article_header(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- article = 1*article-header [CRLF body]
      struct article : P::rule<article, P::and_<
        P::one_or_more_<article_header>, P::zero_or_one_<P::and_<CRLF, body>>
      >> {
        template <typename... Ts>
        article(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

    }
  }
}

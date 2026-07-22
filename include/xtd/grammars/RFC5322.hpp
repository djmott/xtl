/** @file
 * RFC 5322 Internet Message Format.
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 *
 * https://tools.ietf.org/html/rfc5322
 *
 * PEG notes (ordered choice is not pure ABNF alternation):
 * - Every "modern / obs-*" alternation (FWS, quoted-pair, ctext, qtext, dtext, phrase, unstructured,
 *   angle-addr, mailbox-list, address-list, group-list, local-part, domain, message, body, id-left,
 *   id-right, day-of-week, day, year, hour, minute, second, zone) tries the modern (section 3) form
 *   first and falls back to the obsolete (section 4) form second, matching the ABNF text order. Since
 *   the modern forms are strict subsets of the liberal obsolete forms, ordered choice here is safe:
 *   whichever succeeds first is a valid parse, and generators (which MUST NOT emit obsolete forms)
 *   will always match the modern branch.
 * - fields = *(trace *optional-field / *(resent-date/.../resent-msg-id)) *(orig-date/.../optional-field)
 *   is encoded as `zero_or_more_<or_<and_<trace, zero_or_more_<optional_field>>, zero_or_more_<or_<resent-*>>>>`
 *   followed by `zero_or_more_<or_<orig-date/.../optional-field>>`; optional-field is intentionally the
 *   last alternative in both inner Or's so that named fields are preferred over the catch-all.
 * - obs-fields is a flat `zero_or_more_<or_<...23 obs-* alternatives...>>`; because every obs-* field
 *   rule starts by matching a distinct literal field-name string, ordered choice never mis-selects here.
 * - There is no `Repeat<>` combinator in xtd::parse. Bounded repetition counts (1*2DIGIT, 2DIGIT,
 *   2*DIGIT, 4*DIGIT) are hand-expanded with `and_`/`or_`/`zero_or_more_` as documented at each site;
 *   longest alternative is listed first in `or_<>` so e.g. 1*2DIGIT does not stop after a single digit
 *   when a second digit is available (greedy PEG matching approximates the ABNF repetition count).
 * - The 998-octet line-length limit on `body` (*998text CRLF / *998text) is NOT enforced; it is
 *   approximated as unbounded `zero_or_more_<text>` runs, since xtd::parse has no bounded-repeat
 *   combinator. This is called out again at the `body` rule below.
 * - `comment`/`ccontent` and `CFWS`/`FWS`/`comment` are mutually recursive. All rule structs are
 *   forward-declared before any definitions so each struct's `P::rule<Decl, Impl>` base can name
 *   not-yet-defined sibling rules as template arguments; template member functions (including the
 *   virtual `parse()` inherited from `P::rule<>`) are only instantiated when actually invoked, by
 *   which point every rule in this translation unit is a complete type. (Verified against this
 *   library's `rule<>`/`or_<>`/`and_<>` implementation with a standalone recursive comment/ccontent
 *   grammar compiled under g++ 15.)
 * - `Return` is capitalized (unlike the ABNF token "return") because "return" is a C++ keyword.
 * - `message` interleaves modern and `obs-*` header alternatives in one `zero_or_more_` so obsolete
 *   field spellings (e.g. "Subject : foo") parse at the top level. Standalone `fields` / `obs_fields`
 *   rules remain for direct testing. Resent blocks use `one_or_more_` (not `zero_or_more_`) so the
 *   outer repetition cannot spin on empty matches.
 * - This grammar assumes a parser context with an EMPTY whitespace-skip set (e.g.
 *   `xtd::parser<message, false, xtd::parse::whitespace<>>`). All insignificant whitespace in RFC 5322
 *   (folding, comments) is matched explicitly via `WSP`/`FWS`/`CFWS`, and CR/LF are semantically
 *   significant tokens; the default `xtd::parser<>` whitespace set of `" \t\n\r"` would cause the
 *   library's terminal-level `skip_ws` to silently swallow CR/LF/space ahead of every terminal match,
 *   breaking CRLF/WSP matching throughout this grammar.
 */

#pragma once

#include "xtd/xtd.hpp"
#include "xtd/parse.hpp"
#include "xtd/grammars/RFC5234.hpp"

namespace xtd {
  namespace Grammars {
    namespace RFC5322 {

      namespace P = xtd::parse;

#pragma region("imports from RFC5234")
      using CR = RFC5234::CR;
      using LF = RFC5234::LF;
      using CRLF = RFC5234::CRLF;
      using SP = RFC5234::SP;
      using HTAB = RFC5234::HTAB;
      using WSP = RFC5234::WSP;
      using DIGIT = RFC5234::DIGIT;
      using ALPHA = RFC5234::ALPHA;
      using VCHAR = RFC5234::VCHAR;
      using DQUOTE = RFC5234::DQUOTE;
      using HEXDIG = RFC5234::HEXDIG;
#pragma endregion

#pragma region("punctuation terminals")
      CHARACTER_(BackSlash, '\\');
      CHARACTER_(OpenParan, '(');
      CHARACTER_(CloseParan, ')');
      CHARACTER_(LessThan, '<');
      CHARACTER_(GreaterThan, '>');
      CHARACTER_(OpenBracket, '[');
      CHARACTER_(CloseBracket, ']');
      CHARACTER_(Colon, ':');
      CHARACTER_(SemiColon, ';');
      CHARACTER_(At, '@');
      CHARACTER_(Comma, ',');
      CHARACTER_(Period, '.');
      CHARACTER_(Exclaimation, '!');
      CHARACTER_(Hash, '#');
      CHARACTER_(Dollar, '$');
      CHARACTER_(Percent, '%');
      CHARACTER_(Ampersand, '&');
      CHARACTER_(SingleQuote, '\'');
      CHARACTER_(Asterisk, '*');
      CHARACTER_(Plus, '+');
      CHARACTER_(Hyphen, '-');
      CHARACTER_(ForwardSlash, '/');
      CHARACTER_(Equal, '=');
      CHARACTER_(Question, '?');
      CHARACTER_(Caret, '^');
      CHARACTER_(UnderScore, '_');
      CHARACTER_(Apostrophe, '`');
      CHARACTER_(OpenBrace, '{');
      CHARACTER_(CloseBrace, '}');
      CHARACTER_(Pipe, '|');
      CHARACTER_(Tilde, '~');
      CHARACTER_(Nul, '\x00');
#pragma endregion

#pragma region("control / range terminals for character classes")
      // obs-NO-WS-CTL = %d1-8 / %d11 / %d12 / %d14-31 / %d127
      CHARACTERS_(NoWsCtlLow, '\x01', '\x08');
      CHARACTER_(NoWsCtl11, '\x0b');
      CHARACTER_(NoWsCtl12, '\x0c');
      CHARACTERS_(NoWsCtlMid, '\x0e', '\x1f');
      CHARACTER_(NoWsCtlDel, '\x7f');

      // text = %d1-9 / %d11 / %d12 / %d14-127
      CHARACTERS_(TextLow, '\x01', '\x09');
      CHARACTERS_(TextHigh, '\x0e', '\x7f');

      // ctext = %d33-39 / %d42-91 / %d93-126 / obs-ctext ; qtext shares the %d93-126 tail
      CHARACTERS_(CtextLow, '\x21', '\x27');
      CHARACTERS_(CtextMid, '\x2a', '\x5b');
      CHARACTERS_(PrintHigh, '\x5d', '\x7e');

      // qtext = %d33 / %d35-91 / %d93-126 / obs-qtext
      CHARACTER_(QtextChar33, '\x21');
      CHARACTERS_(QtextMid, '\x23', '\x5b');

      // dtext = %d33-90 / %d94-126 / obs-dtext
      CHARACTERS_(DtextLow, '\x21', '\x5a');
      CHARACTERS_(DtextHigh, '\x5e', '\x7e');

      // ftext = %d33-57 / %d59-126
      CHARACTERS_(FtextLow, '\x21', '\x39');
      CHARACTERS_(FtextHigh, '\x3b', '\x7e');

      // obs-zone military single-letter zones: %d65-73 / %d75-90 / %d97-105 / %d107-122
      CHARACTERS_(ZoneMilUpperA, '\x41', '\x49');
      CHARACTERS_(ZoneMilUpperK, '\x4b', '\x5a');
      CHARACTERS_(ZoneMilLowerA, '\x61', '\x69');
      CHARACTERS_(ZoneMilLowerK, '\x6b', '\x7a');
#pragma endregion

#pragma region("string terminals - day/month/zone names")
      STRING(Mon, "Mon");
      STRING(Tue, "Tue");
      STRING(Wed, "Wed");
      STRING(Thu, "Thu");
      STRING(Fri, "Fri");
      STRING(Sat, "Sat");
      STRING(Sun, "Sun");

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

      STRING(UT, "UT");
      STRING(GMT, "GMT");
      STRING(EST, "EST");
      STRING(EDT, "EDT");
      STRING(CST, "CST");
      STRING(CDT, "CDT");
      STRING(MST, "MST");
      STRING(MDT, "MDT");
      STRING(PST, "PST");
      STRING(PDT, "PDT");
#pragma endregion

#pragma region("string terminals - field name prefixes (with colon), section 3")
      STRING(orig_date_prefix, "Date:");
      STRING(from_prefix, "From:");
      STRING(sender_prefix, "Sender:");
      STRING(reply_to_prefix, "Reply-To:");
      STRING(to_prefix, "To:");
      STRING(cc_prefix, "Cc:");
      STRING(bcc_prefix, "Bcc:");
      STRING(message_id_prefix, "Message-ID:");
      STRING(in_reply_to_prefix, "In-Reply-To:");
      STRING(references_prefix, "References:");
      STRING(subject_prefix, "Subject:");
      STRING(comments_prefix, "Comments:");
      STRING(keywords_prefix, "Keywords:");
      STRING(resent_date_prefix, "Resent-Date:");
      STRING(resent_from_prefix, "Resent-From:");
      STRING(resent_sender_prefix, "Resent-Sender:");
      STRING(resent_to_prefix, "Resent-To:");
      STRING(resent_cc_prefix, "Resent-Cc:");
      STRING(resent_bcc_prefix, "Resent-Bcc:");
      STRING(resent_msg_id_prefix, "Resent-Message-ID:");
      STRING(return_prefix, "Return-Path:");
      STRING(received_prefix, "Received:");
#pragma endregion

#pragma region("string terminals - bare field names (no colon), section 4.5 obs-*")
      STRING(Date_name, "Date");
      STRING(From_name, "From");
      STRING(Sender_name, "Sender");
      STRING(ReplyTo_name, "Reply-To");
      STRING(To_name, "To");
      STRING(Cc_name, "Cc");
      STRING(Bcc_name, "Bcc");
      STRING(MessageID_name, "Message-ID");
      STRING(InReplyTo_name, "In-Reply-To");
      STRING(References_name, "References");
      STRING(Subject_name, "Subject");
      STRING(Comments_name, "Comments");
      STRING(Keywords_name, "Keywords");
      STRING(ResentFrom_name, "Resent-From");
      STRING(ResentSender_name, "Resent-Sender");
      STRING(ResentDate_name, "Resent-Date");
      STRING(ResentTo_name, "Resent-To");
      STRING(ResentCc_name, "Resent-Cc");
      STRING(ResentBcc_name, "Resent-Bcc");
      STRING(ResentMessageID_name, "Resent-Message-ID");
      STRING(ResentReplyTo_name, "Resent-Reply-To");
      STRING(ReturnPath_name, "Return-Path");
      STRING(Received_name, "Received");
#pragma endregion

#pragma region("forward declarations")
      struct quoted_pair;
      struct FWS;
      struct comment;
      struct ccontent;
      struct CFWS;
      struct atom;
      struct dot_atom_text;
      struct dot_atom;
      struct qcontent;
      struct quoted_string;
      struct word;
      struct phrase;
      struct unstructured;

      struct date_time;
      struct day_of_week;
      struct day_name;
      struct date;
      struct day;
      struct month;
      struct year;
      struct time;
      struct time_of_day;
      struct hour;
      struct minute;
      struct second;
      struct zone;

      struct address;
      struct mailbox;
      struct name_addr;
      struct angle_addr;
      struct group;
      struct display_name;
      struct mailbox_list;
      struct address_list;
      struct group_list;
      struct addr_spec;
      struct local_part;
      struct domain;
      struct domain_literal;

      struct message;
      struct body;

      struct fields;
      struct orig_date;
      struct from;
      struct sender;
      struct reply_to;
      struct to;
      struct cc;
      struct bcc;
      struct message_id;
      struct in_reply_to;
      struct references;
      struct msg_id;
      struct id_left;
      struct id_right;
      struct no_fold_literal;
      struct subject;
      struct comments;
      struct keywords;
      struct resent_date;
      struct resent_from;
      struct resent_sender;
      struct resent_to;
      struct resent_cc;
      struct resent_bcc;
      struct resent_msg_id;
      struct trace;
      struct Return;
      struct path;
      struct received;
      struct received_token;
      struct optional_field;
      struct field_name;

      struct obs_qp;
      struct obs_body;
      struct obs_unstruct;
      struct obs_phrase;
      struct obs_phrase_list;
      struct obs_FWS;
      struct obs_day_of_week;
      struct obs_day;
      struct obs_year;
      struct obs_hour;
      struct obs_minute;
      struct obs_second;
      struct obs_zone;
      struct obs_angle_addr;
      struct obs_route;
      struct obs_domain_list;
      struct obs_mbox_list;
      struct obs_addr_list;
      struct obs_group_list;
      struct obs_local_part;
      struct obs_domain;
      struct obs_dtext;
      struct obs_fields;
      struct obs_return;
      struct obs_received;
      struct obs_orig_date;
      struct obs_from;
      struct obs_sender;
      struct obs_reply_to;
      struct obs_to;
      struct obs_cc;
      struct obs_bcc;
      struct obs_message_id;
      struct obs_in_reply_to;
      struct obs_references;
      struct obs_id_left;
      struct obs_id_right;
      struct obs_subject;
      struct obs_comments;
      struct obs_keywords;
      struct obs_resent_date;
      struct obs_resent_from;
      struct obs_resent_send;
      struct obs_resent_to;
      struct obs_resent_cc;
      struct obs_resent_bcc;
      struct obs_resent_mid;
      struct obs_resent_rply;
      struct obs_optional;
#pragma endregion

#pragma region("character classes (using aliases)")
      // obs-NO-WS-CTL = %d1-8 / %d11 / %d12 / %d14-31 / %d127
      using obs_NO_WS_CTL = P::or_<NoWsCtlLow, NoWsCtl11, NoWsCtl12, NoWsCtlMid, NoWsCtlDel>;

      // obs-ctext = obs-NO-WS-CTL ; obs-qtext = obs-NO-WS-CTL
      using obs_ctext = obs_NO_WS_CTL;
      using obs_qtext = obs_NO_WS_CTL;

      // obs-utext = %d0 / obs-NO-WS-CTL / VCHAR
      using obs_utext = P::or_<Nul, obs_NO_WS_CTL, VCHAR>;

      // text = %d1-9 / %d11 / %d12 / %d14-127
      using text = P::or_<TextLow, NoWsCtl11, NoWsCtl12, TextHigh>;

      // ctext = %d33-39 / %d42-91 / %d93-126 / obs-ctext
      using ctext = P::or_<CtextLow, CtextMid, PrintHigh, obs_ctext>;

      // qtext = %d33 / %d35-91 / %d93-126 / obs-qtext
      using qtext = P::or_<QtextChar33, QtextMid, PrintHigh, obs_qtext>;

      // dtext = %d33-90 / %d94-126 / obs-dtext
      using dtext = P::or_<DtextLow, DtextHigh, obs_dtext>;

      // ftext = %d33-57 / %d59-126
      using ftext = P::or_<FtextLow, FtextHigh>;

      // atext = ALPHA / DIGIT / "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "/" /
      //         "=" / "?" / "^" / "_" / "`" / "{" / "|" / "}" / "~"
      using atext = P::or_<ALPHA, DIGIT, Exclaimation, Hash, Dollar, Percent, Ampersand, SingleQuote,
        Asterisk, Plus, Hyphen, ForwardSlash, Equal, Question, Caret, UnderScore, Apostrophe,
        OpenBrace, CloseBrace, Pipe, Tilde>;

      // specials = "(" / ")" / "<" / ">" / "[" / "]" / ":" / ";" / "@" / "\" / "," / "." / DQUOTE
      using specials = P::or_<OpenParan, CloseParan, LessThan, GreaterThan, OpenBracket, CloseBracket,
        Colon, SemiColon, At, BackSlash, Comma, Period, DQUOTE>;
#pragma endregion

#pragma region("3.2 Quoted characters / Folding White Space and Comments")
      //- quoted-pair = ("\" (VCHAR / WSP)) / obs-qp
      struct quoted_pair : P::rule<quoted_pair, P::or_<P::and_<BackSlash, P::or_<VCHAR, WSP>>, obs_qp>> {
        template <typename... Ts>
        quoted_pair(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- FWS = ([*WSP CRLF] 1*WSP) / obs-FWS
      struct FWS : P::rule<FWS, P::or_<
        P::and_<P::zero_or_one_<P::and_<P::zero_or_more_<WSP>, CRLF>>, P::one_or_more_<WSP>>,
        obs_FWS
      >> {
        template <typename... Ts>
        FWS(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- ccontent = ctext / quoted-pair / comment
      struct ccontent : P::rule<ccontent, P::or_<ctext, quoted_pair, comment>> {
        template <typename... Ts>
        ccontent(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- comment = "(" *([FWS] ccontent) [FWS] ")"
      struct comment : P::rule<comment, P::and_<
        OpenParan, P::zero_or_more_<P::and_<P::zero_or_one_<FWS>, ccontent>>, P::zero_or_one_<FWS>, CloseParan
      >> {
        template <typename... Ts>
        comment(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- CFWS = (1*([FWS] comment) [FWS]) / FWS
      struct CFWS : P::rule<CFWS, P::or_<
        P::and_<P::one_or_more_<P::and_<P::zero_or_one_<FWS>, comment>>, P::zero_or_one_<FWS>>,
        FWS
      >> {
        template <typename... Ts>
        CFWS(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.2.3.  Atom
      //- atom = [CFWS] 1*atext [CFWS]
      struct atom : P::rule<atom, P::and_<P::zero_or_one_<CFWS>, P::one_or_more_<atext>, P::zero_or_one_<CFWS>>> {
        template <typename... Ts>
        atom(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- dot-atom-text = 1*atext *("." 1*atext)
      struct dot_atom_text : P::rule<dot_atom_text, P::and_<
        P::one_or_more_<atext>, P::zero_or_more_<P::and_<Period, P::one_or_more_<atext>>>
      >> {
        template <typename... Ts>
        dot_atom_text(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- dot-atom = [CFWS] dot-atom-text [CFWS]
      struct dot_atom : P::rule<dot_atom, P::and_<P::zero_or_one_<CFWS>, dot_atom_text, P::zero_or_one_<CFWS>>> {
        template <typename... Ts>
        dot_atom(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.2.4.  Quoted Strings
      //- qcontent = qtext / quoted-pair
      struct qcontent : P::rule<qcontent, P::or_<qtext, quoted_pair>> {
        template <typename... Ts>
        qcontent(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- quoted-string = [CFWS] DQUOTE *([FWS] qcontent) [FWS] DQUOTE [CFWS]
      struct quoted_string : P::rule<quoted_string, P::and_<
        P::zero_or_one_<CFWS>, DQUOTE,
        P::zero_or_more_<P::and_<P::zero_or_one_<FWS>, qcontent>>,
        P::zero_or_one_<FWS>, DQUOTE, P::zero_or_one_<CFWS>
      >> {
        template <typename... Ts>
        quoted_string(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.2.5.  Miscellaneous Tokens
      //- word = atom / quoted-string
      struct word : P::rule<word, P::or_<atom, quoted_string>> {
        template <typename... Ts>
        word(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- phrase = 1*word / obs-phrase
      struct phrase : P::rule<phrase, P::or_<P::one_or_more_<word>, obs_phrase>> {
        template <typename... Ts>
        phrase(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- unstructured = (*([FWS] VCHAR) *WSP) / obs-unstruct
      struct unstructured : P::rule<unstructured, P::or_<
        P::and_<P::zero_or_more_<P::and_<P::zero_or_one_<FWS>, VCHAR>>, P::zero_or_more_<WSP>>,
        obs_unstruct
      >> {
        template <typename... Ts>
        unstructured(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("3.3 Date and Time Specification")
      //- date-time = [ day-of-week "," ] date time [CFWS]
      struct date_time : P::rule<date_time, P::and_<
        P::zero_or_one_<P::and_<day_of_week, Comma>>, date, time, P::zero_or_one_<CFWS>
      >> {
        template <typename... Ts>
        date_time(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- day-of-week = ([FWS] day-name) / obs-day-of-week
      struct day_of_week : P::rule<day_of_week, P::or_<P::and_<P::zero_or_one_<FWS>, day_name>, obs_day_of_week>> {
        template <typename... Ts>
        day_of_week(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- day-name = "Mon" / "Tue" / "Wed" / "Thu" / "Fri" / "Sat" / "Sun"
      struct day_name : P::rule<day_name, P::or_<Mon, Tue, Wed, Thu, Fri, Sat, Sun>> {
        template <typename... Ts>
        day_name(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- date = day month year
      struct date : P::rule<date, P::and_<day, month, year>> {
        template <typename... Ts>
        date(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- day = ([FWS] 1*2DIGIT FWS) / obs-day ; 1*2DIGIT longest-first: 2DIGIT then DIGIT
      struct day : P::rule<day, P::or_<
        P::and_<P::zero_or_one_<FWS>, P::or_<P::and_<DIGIT, DIGIT>, DIGIT>, FWS>,
        obs_day
      >> {
        template <typename... Ts>
        day(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- month = "Jan" / "Feb" / "Mar" / "Apr" / "May" / "Jun" / "Jul" / "Aug" / "Sep" / "Oct" / "Nov" / "Dec"
      struct month : P::rule<month, P::or_<Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec>> {
        template <typename... Ts>
        month(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- year = (FWS 4*DIGIT FWS) / obs-year ; 4*DIGIT = DIGIT DIGIT DIGIT DIGIT *DIGIT
      struct year : P::rule<year, P::or_<
        P::and_<FWS, P::and_<DIGIT, DIGIT, DIGIT, DIGIT, P::zero_or_more_<DIGIT>>, FWS>,
        obs_year
      >> {
        template <typename... Ts>
        year(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- time = time-of-day zone
      struct time : P::rule<time, P::and_<time_of_day, zone>> {
        template <typename... Ts>
        time(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- time-of-day = hour ":" minute [ ":" second ]  (colon only accompanies second, if present)
      struct time_of_day : P::rule<time_of_day, P::and_<
        hour, Colon, minute, P::zero_or_one_<P::and_<Colon, second>>
      >> {
        template <typename... Ts>
        time_of_day(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- hour = 2DIGIT / obs-hour
      struct hour : P::rule<hour, P::or_<P::and_<DIGIT, DIGIT>, obs_hour>> {
        template <typename... Ts>
        hour(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- minute = 2DIGIT / obs-minute
      struct minute : P::rule<minute, P::or_<P::and_<DIGIT, DIGIT>, obs_minute>> {
        template <typename... Ts>
        minute(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- second = 2DIGIT / obs-second
      struct second : P::rule<second, P::or_<P::and_<DIGIT, DIGIT>, obs_second>> {
        template <typename... Ts>
        second(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- zone = (FWS ( "+" / "-" ) 4DIGIT) / obs-zone
      //- zone = (FWS ("+" / "-") 4DIGIT) / obs-zone
      //  Accept optional FWS before obs-zone so "16:00:00 GMT" parses (common Netnews/IMF form).
      struct zone : P::rule<zone, P::or_<
        P::and_<FWS, P::or_<Plus, Hyphen>, P::and_<DIGIT, DIGIT, DIGIT, DIGIT>>,
        P::and_<FWS, obs_zone>,
        obs_zone
      >> {
        template <typename... Ts>
        zone(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("3.4 Address Specification")
      //- address = mailbox / group
      struct address : P::rule<address, P::or_<mailbox, group>> {
        template <typename... Ts>
        address(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- mailbox = name-addr / addr-spec
      struct mailbox : P::rule<mailbox, P::or_<name_addr, addr_spec>> {
        template <typename... Ts>
        mailbox(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- name-addr = [display-name] angle-addr
      struct name_addr : P::rule<name_addr, P::and_<P::zero_or_one_<display_name>, angle_addr>> {
        template <typename... Ts>
        name_addr(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- angle-addr = ([CFWS] "<" addr-spec ">" [CFWS]) / obs-angle-addr
      struct angle_addr : P::rule<angle_addr, P::or_<
        P::and_<P::zero_or_one_<CFWS>, LessThan, addr_spec, GreaterThan, P::zero_or_one_<CFWS>>,
        obs_angle_addr
      >> {
        template <typename... Ts>
        angle_addr(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- group = display-name ":" [group-list] ";" [CFWS]
      struct group : P::rule<group, P::and_<
        display_name, Colon, P::zero_or_one_<group_list>, SemiColon, P::zero_or_one_<CFWS>
      >> {
        template <typename... Ts>
        group(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- display-name = phrase
      struct display_name : P::rule<display_name, phrase> {
        template <typename... Ts>
        display_name(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- mailbox-list = (mailbox *("," mailbox)) / obs-mbox-list
      struct mailbox_list : P::rule<mailbox_list, P::or_<
        P::and_<mailbox, P::zero_or_more_<P::and_<Comma, mailbox>>>,
        obs_mbox_list
      >> {
        template <typename... Ts>
        mailbox_list(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- address-list = (address *("," address)) / obs-addr-list
      struct address_list : P::rule<address_list, P::or_<
        P::and_<address, P::zero_or_more_<P::and_<Comma, address>>>,
        obs_addr_list
      >> {
        template <typename... Ts>
        address_list(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- group-list = mailbox-list / CFWS / obs-group-list
      struct group_list : P::rule<group_list, P::or_<mailbox_list, CFWS, obs_group_list>> {
        template <typename... Ts>
        group_list(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.4.1.  Addr-Spec Specification
      //- addr-spec = local-part "@" domain
      struct addr_spec : P::rule<addr_spec, P::and_<local_part, At, domain>> {
        template <typename... Ts>
        addr_spec(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- local-part = dot-atom / quoted-string / obs-local-part
      struct local_part : P::rule<local_part, P::or_<dot_atom, quoted_string, obs_local_part>> {
        template <typename... Ts>
        local_part(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- domain = dot-atom / domain-literal / obs-domain
      struct domain : P::rule<domain, P::or_<dot_atom, domain_literal, obs_domain>> {
        template <typename... Ts>
        domain(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- domain-literal = [CFWS] "[" *([FWS] dtext) [FWS] "]" [CFWS]
      struct domain_literal : P::rule<domain_literal, P::and_<
        P::zero_or_one_<CFWS>, OpenBracket,
        P::zero_or_more_<P::and_<P::zero_or_one_<FWS>, dtext>>,
        P::zero_or_one_<FWS>, CloseBracket, P::zero_or_one_<CFWS>
      >> {
        template <typename... Ts>
        domain_literal(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("3.5 Overall Message Syntax")
      //- body = (*(*998text CRLF) *998text) / obs-body
      //  PEG caveat: the 998-octet per-line cap is NOT enforced (no bounded-repeat combinator);
      //  each *998text is approximated as an unbounded zero_or_more_<text> run.
      struct body : P::rule<body, P::or_<
        P::and_<P::zero_or_more_<P::and_<P::zero_or_more_<text>, CRLF>>, P::zero_or_more_<text>>,
        obs_body
      >> {
        template <typename... Ts>
        body(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("3.6 Field Definitions")
      /*
      fields = *(trace *optional-field / *(resent-date / resent-from / resent-sender / resent-to /
                  resent-cc / resent-bcc / resent-msg-id))
               *(orig-date / from / sender / reply-to / to / cc / bcc / message-id / in-reply-to /
                 references / subject / comments / keywords / optional-field)
      */
      using fields_resent = P::one_or_more_<P::or_<resent_date, resent_from, resent_sender, resent_to, resent_cc, resent_bcc, resent_msg_id>>;
      using fields_block = P::or_<
        P::and_<trace, P::zero_or_more_<optional_field>>,
        fields_resent
      >;
      using fields_named = P::or_<orig_date, from, sender, reply_to, to, cc, bcc, message_id,
        in_reply_to, references, subject, comments, keywords, optional_field>;

      struct fields : P::rule<fields, P::and_<P::zero_or_more_<fields_block>, P::zero_or_more_<fields_named>>> {
        template <typename... Ts>
        fields(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.6.1.  The Origination Date Field
      //- orig-date = "Date:" date-time CRLF
      struct orig_date : P::rule<orig_date, P::and_<orig_date_prefix, date_time, CRLF>> {
        template <typename... Ts>
        orig_date(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.6.2.  Originator Fields
      //- from = "From:" mailbox-list CRLF
      struct from : P::rule<from, P::and_<from_prefix, mailbox_list, CRLF>> {
        template <typename... Ts>
        from(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- sender = "Sender:" mailbox CRLF
      struct sender : P::rule<sender, P::and_<sender_prefix, mailbox, CRLF>> {
        template <typename... Ts>
        sender(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- reply-to = "Reply-To:" address-list CRLF
      struct reply_to : P::rule<reply_to, P::and_<reply_to_prefix, address_list, CRLF>> {
        template <typename... Ts>
        reply_to(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.6.3.  Destination Address Fields
      //- to = "To:" address-list CRLF
      struct to : P::rule<to, P::and_<to_prefix, address_list, CRLF>> {
        template <typename... Ts>
        to(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- cc = "Cc:" address-list CRLF
      struct cc : P::rule<cc, P::and_<cc_prefix, address_list, CRLF>> {
        template <typename... Ts>
        cc(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- bcc = "Bcc:" [address-list / CFWS] CRLF
      struct bcc : P::rule<bcc, P::and_<bcc_prefix, P::zero_or_one_<P::or_<address_list, CFWS>>, CRLF>> {
        template <typename... Ts>
        bcc(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.6.4.  Identification Fields
      //- message-id = "Message-ID:" msg-id CRLF
      struct message_id : P::rule<message_id, P::and_<message_id_prefix, msg_id, CRLF>> {
        template <typename... Ts>
        message_id(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- in-reply-to = "In-Reply-To:" 1*msg-id CRLF
      struct in_reply_to : P::rule<in_reply_to, P::and_<in_reply_to_prefix, P::one_or_more_<msg_id>, CRLF>> {
        template <typename... Ts>
        in_reply_to(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- references = "References:" 1*msg-id CRLF
      struct references : P::rule<references, P::and_<references_prefix, P::one_or_more_<msg_id>, CRLF>> {
        template <typename... Ts>
        references(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- msg-id = [CFWS] "<" id-left "@" id-right ">" [CFWS]
      struct msg_id : P::rule<msg_id, P::and_<
        P::zero_or_one_<CFWS>, LessThan, id_left, At, id_right, GreaterThan, P::zero_or_one_<CFWS>
      >> {
        template <typename... Ts>
        msg_id(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- id-left = dot-atom-text / obs-id-left
      struct id_left : P::rule<id_left, P::or_<dot_atom_text, obs_id_left>> {
        template <typename... Ts>
        id_left(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- id-right = dot-atom-text / no-fold-literal / obs-id-right
      struct id_right : P::rule<id_right, P::or_<dot_atom_text, no_fold_literal, obs_id_right>> {
        template <typename... Ts>
        id_right(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- no-fold-literal = "[" *dtext "]"
      struct no_fold_literal : P::rule<no_fold_literal, P::and_<OpenBracket, P::zero_or_more_<dtext>, CloseBracket>> {
        template <typename... Ts>
        no_fold_literal(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.6.5.  Informational Fields
      //- subject = "Subject:" unstructured CRLF
      struct subject : P::rule<subject, P::and_<subject_prefix, unstructured, CRLF>> {
        template <typename... Ts>
        subject(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- comments = "Comments:" unstructured CRLF
      struct comments : P::rule<comments, P::and_<comments_prefix, unstructured, CRLF>> {
        template <typename... Ts>
        comments(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- keywords = "Keywords:" phrase *("," phrase) CRLF
      struct keywords : P::rule<keywords, P::and_<
        keywords_prefix, phrase, P::zero_or_more_<P::and_<Comma, phrase>>, CRLF
      >> {
        template <typename... Ts>
        keywords(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.6.6.  Resent Fields
      //- resent-date = "Resent-Date:" date-time CRLF
      struct resent_date : P::rule<resent_date, P::and_<resent_date_prefix, date_time, CRLF>> {
        template <typename... Ts>
        resent_date(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- resent-from = "Resent-From:" mailbox-list CRLF
      struct resent_from : P::rule<resent_from, P::and_<resent_from_prefix, mailbox_list, CRLF>> {
        template <typename... Ts>
        resent_from(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- resent-sender = "Resent-Sender:" mailbox CRLF
      struct resent_sender : P::rule<resent_sender, P::and_<resent_sender_prefix, mailbox, CRLF>> {
        template <typename... Ts>
        resent_sender(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- resent-to = "Resent-To:" address-list CRLF
      struct resent_to : P::rule<resent_to, P::and_<resent_to_prefix, address_list, CRLF>> {
        template <typename... Ts>
        resent_to(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- resent-cc = "Resent-Cc:" address-list CRLF
      struct resent_cc : P::rule<resent_cc, P::and_<resent_cc_prefix, address_list, CRLF>> {
        template <typename... Ts>
        resent_cc(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- resent-bcc = "Resent-Bcc:" [address-list / CFWS] CRLF
      struct resent_bcc : P::rule<resent_bcc, P::and_<
        resent_bcc_prefix, P::zero_or_one_<P::or_<address_list, CFWS>>, CRLF
      >> {
        template <typename... Ts>
        resent_bcc(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- resent-msg-id = "Resent-Message-ID:" msg-id CRLF
      struct resent_msg_id : P::rule<resent_msg_id, P::and_<resent_msg_id_prefix, msg_id, CRLF>> {
        template <typename... Ts>
        resent_msg_id(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.6.7.  Trace Fields
      //- trace = [return] 1*received
      struct trace : P::rule<trace, P::and_<P::zero_or_one_<Return>, P::one_or_more_<received>>> {
        template <typename... Ts>
        trace(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- return = "Return-Path:" path CRLF  (named `Return`; "return" is a C++ keyword)
      struct Return : P::rule<Return, P::and_<return_prefix, path, CRLF>> {
        template <typename... Ts>
        Return(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- path = angle-addr / ([CFWS] "<" [CFWS] ">" [CFWS])
      struct path : P::rule<path, P::or_<
        angle_addr,
        P::and_<P::zero_or_one_<CFWS>, LessThan, P::zero_or_one_<CFWS>, GreaterThan, P::zero_or_one_<CFWS>>
      >> {
        template <typename... Ts>
        path(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- received = "Received:" *received-token ";" date-time CRLF
      struct received : P::rule<received, P::and_<
        received_prefix, P::zero_or_more_<received_token>, SemiColon, date_time, CRLF
      >> {
        template <typename... Ts>
        received(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- received-token = word / angle-addr / addr-spec / domain
      struct received_token : P::rule<received_token, P::or_<word, angle_addr, addr_spec, domain>> {
        template <typename... Ts>
        received_token(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 3.6.8.  Optional Fields
      //- optional-field = field-name ":" unstructured CRLF
      struct optional_field : P::rule<optional_field, P::and_<field_name, Colon, unstructured, CRLF>> {
        template <typename... Ts>
        optional_field(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- field-name = 1*ftext
      struct field_name : P::rule<field_name, P::one_or_more_<ftext>> {
        template <typename... Ts>
        field_name(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("4.1 Miscellaneous Obsolete Tokens")
      //- obs-qp = "\" (%d0 / obs-NO-WS-CTL / LF / CR)
      struct obs_qp : P::rule<obs_qp, P::and_<BackSlash, P::or_<Nul, obs_NO_WS_CTL, LF, CR>>> {
        template <typename... Ts>
        obs_qp(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-body = *((*LF *CR *((%d0 / text) *LF *CR)) / CRLF)
      using obs_body_inner = P::and_<
        P::zero_or_more_<LF>, P::zero_or_more_<CR>,
        P::zero_or_more_<P::and_<P::or_<Nul, text>, P::zero_or_more_<LF>, P::zero_or_more_<CR>>>
      >;
      struct obs_body : P::rule<obs_body, P::zero_or_more_<P::or_<obs_body_inner, CRLF>>> {
        template <typename... Ts>
        obs_body(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-unstruct = *((*LF *CR *(obs-utext *LF *CR)) / FWS)
      using obs_unstruct_inner = P::and_<
        P::zero_or_more_<LF>, P::zero_or_more_<CR>,
        P::zero_or_more_<P::and_<obs_utext, P::zero_or_more_<LF>, P::zero_or_more_<CR>>>
      >;
      struct obs_unstruct : P::rule<obs_unstruct, P::zero_or_more_<P::or_<obs_unstruct_inner, FWS>>> {
        template <typename... Ts>
        obs_unstruct(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-phrase = word *(word / "." / CFWS)
      struct obs_phrase : P::rule<obs_phrase, P::and_<word, P::zero_or_more_<P::or_<word, Period, CFWS>>>> {
        template <typename... Ts>
        obs_phrase(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-phrase-list = [phrase / CFWS] *("," [phrase / CFWS])
      struct obs_phrase_list : P::rule<obs_phrase_list, P::and_<
        P::zero_or_one_<P::or_<phrase, CFWS>>,
        P::zero_or_more_<P::and_<Comma, P::zero_or_one_<P::or_<phrase, CFWS>>>>
      >> {
        template <typename... Ts>
        obs_phrase_list(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("4.2 Obsolete Folding White Space")
      //- obs-FWS = 1*WSP *(CRLF 1*WSP)
      struct obs_FWS : P::rule<obs_FWS, P::and_<
        P::one_or_more_<WSP>, P::zero_or_more_<P::and_<CRLF, P::one_or_more_<WSP>>>
      >> {
        template <typename... Ts>
        obs_FWS(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("4.3 Obsolete Date and Time")
      //- obs-day-of-week = [CFWS] day-name [CFWS]
      struct obs_day_of_week : P::rule<obs_day_of_week, P::and_<P::zero_or_one_<CFWS>, day_name, P::zero_or_one_<CFWS>>> {
        template <typename... Ts>
        obs_day_of_week(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-day = [CFWS] 1*2DIGIT [CFWS]
      struct obs_day : P::rule<obs_day, P::and_<
        P::zero_or_one_<CFWS>, P::or_<P::and_<DIGIT, DIGIT>, DIGIT>, P::zero_or_one_<CFWS>
      >> {
        template <typename... Ts>
        obs_day(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-year = [CFWS] 2*DIGIT [CFWS]
      struct obs_year : P::rule<obs_year, P::and_<
        P::zero_or_one_<CFWS>, P::and_<DIGIT, DIGIT, P::zero_or_more_<DIGIT>>, P::zero_or_one_<CFWS>
      >> {
        template <typename... Ts>
        obs_year(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-hour = [CFWS] 2DIGIT [CFWS]
      struct obs_hour : P::rule<obs_hour, P::and_<P::zero_or_one_<CFWS>, P::and_<DIGIT, DIGIT>, P::zero_or_one_<CFWS>>> {
        template <typename... Ts>
        obs_hour(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-minute = [CFWS] 2DIGIT [CFWS]
      struct obs_minute : P::rule<obs_minute, P::and_<P::zero_or_one_<CFWS>, P::and_<DIGIT, DIGIT>, P::zero_or_one_<CFWS>>> {
        template <typename... Ts>
        obs_minute(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-second = [CFWS] 2DIGIT [CFWS]
      struct obs_second : P::rule<obs_second, P::and_<P::zero_or_one_<CFWS>, P::and_<DIGIT, DIGIT>, P::zero_or_one_<CFWS>>> {
        template <typename... Ts>
        obs_second(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-zone = "UT" / "GMT" / "EST" / "EDT" / "CST" / "CDT" / "MST" / "MDT" / "PST" / "PDT" /
      //             %d65-73 / %d75-90 / %d97-105 / %d107-122
      struct obs_zone : P::rule<obs_zone, P::or_<
        UT, GMT, EST, EDT, CST, CDT, MST, MDT, PST, PDT,
        ZoneMilUpperA, ZoneMilUpperK, ZoneMilLowerA, ZoneMilLowerK
      >> {
        template <typename... Ts>
        obs_zone(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("4.4 Obsolete Addressing")
      //- obs-angle-addr = [CFWS] "<" obs-route addr-spec ">" [CFWS]
      struct obs_angle_addr : P::rule<obs_angle_addr, P::and_<
        P::zero_or_one_<CFWS>, LessThan, obs_route, addr_spec, GreaterThan, P::zero_or_one_<CFWS>
      >> {
        template <typename... Ts>
        obs_angle_addr(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-route = obs-domain-list ":"
      struct obs_route : P::rule<obs_route, P::and_<obs_domain_list, Colon>> {
        template <typename... Ts>
        obs_route(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-domain-list = *(CFWS / ",") "@" domain *("," [CFWS] ["@" domain])
      struct obs_domain_list : P::rule<obs_domain_list, P::and_<
        P::zero_or_more_<P::or_<CFWS, Comma>>, At, domain,
        P::zero_or_more_<P::and_<Comma, P::zero_or_one_<CFWS>, P::zero_or_one_<P::and_<At, domain>>>>
      >> {
        template <typename... Ts>
        obs_domain_list(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-mbox-list = *([CFWS] ",") mailbox *("," [mailbox / CFWS])
      struct obs_mbox_list : P::rule<obs_mbox_list, P::and_<
        P::zero_or_more_<P::and_<P::zero_or_one_<CFWS>, Comma>>, mailbox,
        P::zero_or_more_<P::and_<Comma, P::zero_or_one_<P::or_<mailbox, CFWS>>>>
      >> {
        template <typename... Ts>
        obs_mbox_list(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-addr-list = *([CFWS] ",") address *("," [address / CFWS])
      struct obs_addr_list : P::rule<obs_addr_list, P::and_<
        P::zero_or_more_<P::and_<P::zero_or_one_<CFWS>, Comma>>, address,
        P::zero_or_more_<P::and_<Comma, P::zero_or_one_<P::or_<address, CFWS>>>>
      >> {
        template <typename... Ts>
        obs_addr_list(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-group-list = 1*([CFWS] ",") [CFWS]
      struct obs_group_list : P::rule<obs_group_list, P::and_<
        P::one_or_more_<P::and_<P::zero_or_one_<CFWS>, Comma>>, P::zero_or_one_<CFWS>
      >> {
        template <typename... Ts>
        obs_group_list(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-local-part = word *("." word)
      struct obs_local_part : P::rule<obs_local_part, P::and_<word, P::zero_or_more_<P::and_<Period, word>>>> {
        template <typename... Ts>
        obs_local_part(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-domain = atom *("." atom)
      struct obs_domain : P::rule<obs_domain, P::and_<atom, P::zero_or_more_<P::and_<Period, atom>>>> {
        template <typename... Ts>
        obs_domain(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-dtext = obs-NO-WS-CTL / quoted-pair
      struct obs_dtext : P::rule<obs_dtext, P::or_<obs_NO_WS_CTL, quoted_pair>> {
        template <typename... Ts>
        obs_dtext(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

#pragma region("4.5 Obsolete Header Fields")
      /*
      obs-fields = *(obs-return / obs-received / obs-orig-date / obs-from / obs-sender / obs-reply-to /
                     obs-to / obs-cc / obs-bcc / obs-message-id / obs-in-reply-to / obs-references /
                     obs-subject / obs-comments / obs-keywords / obs-resent-date / obs-resent-from /
                     obs-resent-send / obs-resent-rply / obs-resent-to / obs-resent-cc / obs-resent-bcc /
                     obs-resent-mid / obs-optional)
      */
      using obs_fields_alt = P::or_<
        obs_return, obs_received, obs_orig_date, obs_from, obs_sender, obs_reply_to, obs_to, obs_cc, obs_bcc,
        obs_message_id, obs_in_reply_to, obs_references, obs_subject, obs_comments, obs_keywords,
        obs_resent_date, obs_resent_from, obs_resent_send, obs_resent_rply, obs_resent_to, obs_resent_cc,
        obs_resent_bcc, obs_resent_mid, obs_optional
      >;
      struct obs_fields : P::rule<obs_fields, P::zero_or_more_<obs_fields_alt>> {
        template <typename... Ts>
        obs_fields(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- message = *(modern-field / obs-field) [CRLF body]
      //  Interleaves modern + obs alternatives so obsolete spellings parse at top level.
      using message_field = P::or_<fields_block, fields_named, obs_fields_alt>;
      struct message : P::rule<message, P::and_<
        P::zero_or_more_<message_field>, P::zero_or_one_<P::and_<CRLF, body>>
      >> {
        template <typename... Ts>
        message(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 4.5.1.  Obsolete Origination Date Field
      //- obs-orig-date = "Date" *WSP ":" date-time CRLF
      struct obs_orig_date : P::rule<obs_orig_date, P::and_<Date_name, P::zero_or_more_<WSP>, Colon, date_time, CRLF>> {
        template <typename... Ts>
        obs_orig_date(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 4.5.2.  Obsolete Originator Fields
      //- obs-from = "From" *WSP ":" mailbox-list CRLF
      struct obs_from : P::rule<obs_from, P::and_<From_name, P::zero_or_more_<WSP>, Colon, mailbox_list, CRLF>> {
        template <typename... Ts>
        obs_from(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-sender = "Sender" *WSP ":" mailbox CRLF
      struct obs_sender : P::rule<obs_sender, P::and_<Sender_name, P::zero_or_more_<WSP>, Colon, mailbox, CRLF>> {
        template <typename... Ts>
        obs_sender(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-reply-to = "Reply-To" *WSP ":" address-list CRLF
      struct obs_reply_to : P::rule<obs_reply_to, P::and_<ReplyTo_name, P::zero_or_more_<WSP>, Colon, address_list, CRLF>> {
        template <typename... Ts>
        obs_reply_to(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 4.5.3.  Obsolete Destination Address Fields
      //- obs-to = "To" *WSP ":" address-list CRLF
      struct obs_to : P::rule<obs_to, P::and_<To_name, P::zero_or_more_<WSP>, Colon, address_list, CRLF>> {
        template <typename... Ts>
        obs_to(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-cc = "Cc" *WSP ":" address-list CRLF
      struct obs_cc : P::rule<obs_cc, P::and_<Cc_name, P::zero_or_more_<WSP>, Colon, address_list, CRLF>> {
        template <typename... Ts>
        obs_cc(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-bcc = "Bcc" *WSP ":" (address-list / (*([CFWS] ",") [CFWS])) CRLF
      struct obs_bcc : P::rule<obs_bcc, P::and_<
        Bcc_name, P::zero_or_more_<WSP>, Colon,
        P::or_<address_list, P::and_<P::zero_or_more_<P::and_<P::zero_or_one_<CFWS>, Comma>>, P::zero_or_one_<CFWS>>>,
        CRLF
      >> {
        template <typename... Ts>
        obs_bcc(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 4.5.4.  Obsolete Identification Fields
      //- obs-message-id = "Message-ID" *WSP ":" msg-id CRLF
      struct obs_message_id : P::rule<obs_message_id, P::and_<MessageID_name, P::zero_or_more_<WSP>, Colon, msg_id, CRLF>> {
        template <typename... Ts>
        obs_message_id(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-in-reply-to = "In-Reply-To" *WSP ":" *(phrase / msg-id) CRLF
      struct obs_in_reply_to : P::rule<obs_in_reply_to, P::and_<
        InReplyTo_name, P::zero_or_more_<WSP>, Colon, P::zero_or_more_<P::or_<phrase, msg_id>>, CRLF
      >> {
        template <typename... Ts>
        obs_in_reply_to(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-references = "References" *WSP ":" *(phrase / msg-id) CRLF
      struct obs_references : P::rule<obs_references, P::and_<
        References_name, P::zero_or_more_<WSP>, Colon, P::zero_or_more_<P::or_<phrase, msg_id>>, CRLF
      >> {
        template <typename... Ts>
        obs_references(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-id-left = local-part
      struct obs_id_left : P::rule<obs_id_left, local_part> {
        template <typename... Ts>
        obs_id_left(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-id-right = domain
      struct obs_id_right : P::rule<obs_id_right, domain> {
        template <typename... Ts>
        obs_id_right(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 4.5.5.  Obsolete Informational Fields
      //- obs-subject = "Subject" *WSP ":" unstructured CRLF
      struct obs_subject : P::rule<obs_subject, P::and_<Subject_name, P::zero_or_more_<WSP>, Colon, unstructured, CRLF>> {
        template <typename... Ts>
        obs_subject(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-comments = "Comments" *WSP ":" unstructured CRLF
      struct obs_comments : P::rule<obs_comments, P::and_<Comments_name, P::zero_or_more_<WSP>, Colon, unstructured, CRLF>> {
        template <typename... Ts>
        obs_comments(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-keywords = "Keywords" *WSP ":" obs-phrase-list CRLF
      struct obs_keywords : P::rule<obs_keywords, P::and_<Keywords_name, P::zero_or_more_<WSP>, Colon, obs_phrase_list, CRLF>> {
        template <typename... Ts>
        obs_keywords(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 4.5.6.  Obsolete Resent Fields
      //- obs-resent-from = "Resent-From" *WSP ":" mailbox-list CRLF
      struct obs_resent_from : P::rule<obs_resent_from, P::and_<ResentFrom_name, P::zero_or_more_<WSP>, Colon, mailbox_list, CRLF>> {
        template <typename... Ts>
        obs_resent_from(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-resent-send = "Resent-Sender" *WSP ":" mailbox CRLF
      struct obs_resent_send : P::rule<obs_resent_send, P::and_<ResentSender_name, P::zero_or_more_<WSP>, Colon, mailbox, CRLF>> {
        template <typename... Ts>
        obs_resent_send(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-resent-date = "Resent-Date" *WSP ":" date-time CRLF
      struct obs_resent_date : P::rule<obs_resent_date, P::and_<ResentDate_name, P::zero_or_more_<WSP>, Colon, date_time, CRLF>> {
        template <typename... Ts>
        obs_resent_date(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-resent-to = "Resent-To" *WSP ":" address-list CRLF
      struct obs_resent_to : P::rule<obs_resent_to, P::and_<ResentTo_name, P::zero_or_more_<WSP>, Colon, address_list, CRLF>> {
        template <typename... Ts>
        obs_resent_to(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-resent-cc = "Resent-Cc" *WSP ":" address-list CRLF
      struct obs_resent_cc : P::rule<obs_resent_cc, P::and_<ResentCc_name, P::zero_or_more_<WSP>, Colon, address_list, CRLF>> {
        template <typename... Ts>
        obs_resent_cc(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-resent-bcc = "Resent-Bcc" *WSP ":" (address-list / (*([CFWS] ",") [CFWS])) CRLF
      struct obs_resent_bcc : P::rule<obs_resent_bcc, P::and_<
        ResentBcc_name, P::zero_or_more_<WSP>, Colon,
        P::or_<address_list, P::and_<P::zero_or_more_<P::and_<P::zero_or_one_<CFWS>, Comma>>, P::zero_or_one_<CFWS>>>,
        CRLF
      >> {
        template <typename... Ts>
        obs_resent_bcc(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-resent-mid = "Resent-Message-ID" *WSP ":" msg-id CRLF
      struct obs_resent_mid : P::rule<obs_resent_mid, P::and_<ResentMessageID_name, P::zero_or_more_<WSP>, Colon, msg_id, CRLF>> {
        template <typename... Ts>
        obs_resent_mid(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-resent-rply = "Resent-Reply-To" *WSP ":" address-list CRLF
      struct obs_resent_rply : P::rule<obs_resent_rply, P::and_<ResentReplyTo_name, P::zero_or_more_<WSP>, Colon, address_list, CRLF>> {
        template <typename... Ts>
        obs_resent_rply(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 4.5.7.  Obsolete Trace Fields
      //- obs-return = "Return-Path" *WSP ":" path CRLF
      struct obs_return : P::rule<obs_return, P::and_<ReturnPath_name, P::zero_or_more_<WSP>, Colon, path, CRLF>> {
        template <typename... Ts>
        obs_return(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //- obs-received = "Received" *WSP ":" *received-token CRLF
      struct obs_received : P::rule<obs_received, P::and_<
        Received_name, P::zero_or_more_<WSP>, Colon, P::zero_or_more_<received_token>, CRLF
      >> {
        template <typename... Ts>
        obs_received(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };

      //+ 4.5.8.  Obsolete optional fields
      //- obs-optional = field-name *WSP ":" unstructured CRLF
      struct obs_optional : P::rule<obs_optional, P::and_<field_name, P::zero_or_more_<WSP>, Colon, unstructured, CRLF>> {
        template <typename... Ts>
        obs_optional(Ts&&... a) : rule(std::forward<Ts>(a)...) {}
      };
#pragma endregion

    }
  }
}

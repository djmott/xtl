#ifndef __RFC_5322_HPP_INCLUDED__
#define __RFC_5322_HPP_INCLUDED__

/*
Internet Message Format
https://tools.ietf.org/html/rfc5322
*/

namespace xtd {
	namespace Grammars {
		namespace RFC5322 {
			using namespace RFC5234;
			using namespace xtd::Parser::MultiByte;
#pragma region("forward declerations")
			struct quoted_pair;
			struct FWS;
			struct ctext;
			struct ccontent;
			struct comment;
			struct CFWS;
			struct atext;
			struct atom;
			struct dot_atom_text;
			struct dot_atom;
			struct specials;
			struct qtext;
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
			struct dtext;
			struct message;
			struct body;
			struct text;
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
			struct ftext;
			struct obs_NO_WS_CTL;
			struct obs_ctext;
			struct obs_qtext;
			struct obs_utext;
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
			struct obs_subject;
			struct obs_comments;
			struct obs_keywords;
			struct obs_resent_date;
			struct obs_resent_from;
			struct obs_resent_send;
			struct obs_resent_rply;
			struct obs_resent_to;
			struct obs_resent_cc;
			struct obs_resent_bcc;
			struct obs_resent_mid;
			struct obs_optional;
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
			struct obs_resent_from;
			struct obs_resent_send;
			struct obs_resent_date;
			struct obs_resent_to;
			struct obs_resent_cc;
			struct obs_resent_bcc;
			struct obs_resent_mid;
			struct obs_resent_rply;
			struct obs_return;
			struct obs_received;
			struct obs_optional;
#pragma endregion

#pragma region("strings")
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
			STRING(received_prefix, "Received:");
			STRING(return_prefix, "Return-Path:");
			STRING(resent_date_prefix, "Resent-Date:");
			STRING(resent_from_prefix, "Resent-From:");
			STRING(resent_sender_prefix, "Resent-Sender:");
			STRING(resent_to_prefix, "Resent-To:");
			STRING(resent_cc_prefix, "Resent-Cc:");
			STRING(resent_bcc_prefix, "Resent-Bcc:");
			STRING(resent_msg_id_prefix, "Resent-Message-ID:");

			STRING(subject_prefix, "Subject:");
			STRING(comments_prefix, "Comments:");
			STRING(keywords_prefix, "Keywords:");
			STRING(references_prefix, "References:");
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
			STRING(orig_date_prefix, "Date:");
			STRING(from_prefix, "From:");
			STRING(sender_prefix, "Sender:");
			STRING(reply_to_prefix, "Reply-To:");
			STRING(to_prefix, "To:");
			STRING(cc_prefix, "Cc:");
			STRING(bcc_prefix, "Bcc:");
			STRING(message_id_prefix, "Message-ID:");
			STRING(in_reply_to_prefix, "In-Reply_To:");
			STRING(Mon, "Mon");
			STRING(Tue, "Tue");
			STRING(Wed, "Wed");
			STRING(Thu, "Thu");
			STRING(Fri, "Fri");
			STRING(Sat, "Sat");
			STRING(Sun, "Sun");
#pragma endregion

#pragma region("imports")
#pragma endregion

#pragma region("rules")
			//+ 3.2.1.  Quoted characters
			//- quoted-pair     =   ("\" (VCHAR / WSP)) / obs-qp
			struct quoted_pair : Rule < quoted_pair, Or<And<BackSlash, Or<VCHAR, WSP>>, obs_qp> > {};

			//+ 3.2.2.  Folding White Space and Comments
			//-  FWS             =   ([*WSP CRLF] 1*WSP) /  obs-FWS ; Folding white space
			struct FWS : Rule < FWS, Or<And<ZeroOrOne<ZeroOrMore<WSP>, OneOrMore<WSP>>>, obs_FWS> > {};

			//- ctext           =   %d33-39 /  %d42-91 /  %d93-126 / obs-ctext
			struct ctext : Rule < ctext, Or<Character<char, 33, 39>, Character<char, 42, 91>, Character<char, 93, 126>, obs_ctext> > {};

			//- ccontent        =   ctext / quoted-pair / comment
			struct ccontent : Rule < ccontent, Or<ctext, quoted_pair, comment> > {};

			//- comment         =   "(" *([FWS] ccontent) [FWS] ")"
			struct comment : Rule < comment, OpenParan, ZeroOrMore<ZeroOrOne<FWS>, ccontent>, ZeroOrOne<FWS>, CloseParan > {};

			//-  CFWS            =   (1*([FWS] comment) [FWS]) / FWS
			struct CFWS : Rule < CFWS, Or<OneOrMore<ZeroOrOne<FWS>, comment>, ZeroOrOne<FWS>>, FWS > {};

			//+ 3.2.3.  Atom
			//- atext           =   ALPHA / DIGIT /  "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "/" / "=" / "?" / "^" / "_" / "`" / "{" / "|" / "}" / "~"
			struct atext : Rule < atext, Or<AlphaNumeric, Exclaimation, Hash, Dollar, Percent, Ampersand, SingleQuote, Asterisk, Plus, Hyphen, ForwardSlash, Equal, Question, Caret, UnderScore, Apostrophe, OpenBrace, CloseBrace, Pipe, Tilde> > {};

			//- atom            =   [CFWS] 1*atext [CFWS]
			struct atom : Rule < atom, ZeroOrOne<CFWS>, OneOrMore<atext>, ZeroOrOne<CFWS> > {};

			//- dot-atom-text   =   1*atext *("." 1*atext)
			struct dot_atom_text : Rule < dot_atom_text, OneOrMore<atext>, ZeroOrOne<Period, OneOrMore<atext>> > {};

			//- dot-atom        =   [CFWS] dot-atom-text [CFWS]
			struct dot_atom : Rule < dot_atom, ZeroOrOne<CFWS>, dot_atom_text, ZeroOrOne<CFWS> > {};

			//- specials        =   "(" / ")" /  "<" / ">" / "[" / "]" / ":" / ";" / "@" / "\" / "," / "." / DQUOTE
			struct specials : Rule < specials, Or<OpenParan, CloseParan, LessThan, GreaterThan, OpenBracket, CloseBracket, Colon, SemiColon, At, BackSlash, Comma, Period, DQUOTE> > {};

			//+ 3.2.4.  Quoted Strings
			//- qtext           =   %d33 / %d35-91 / %d93-126 / obs-qtext
			struct qtext : Rule < qtext, Character<char, 33>, Character<char, 35, 91>, Character<char, 93, 126>, obs_qtext > {};

			//- qcontent        =   qtext / quoted-pair
			struct qcontent : Rule < qcontent, Or<qtext, quoted_pair> > {};

			//- quoted-string   =   [CFWS] DQUOTE *([FWS] qcontent)[FWS] DQUOTE [CFWS]
			struct quoted_string : Rule < quoted_string, ZeroOrOne<CFWS>, DQUOTE, ZeroOrMore<ZeroOrOne<FWS>, qcontent>, ZeroOrOne<FWS>, DQUOTE, ZeroOrOne<CFWS> > {};

			//+ 3.2.5.  Miscellaneous Tokens
			//- word            =   atom / quoted-string
			struct word : Rule < word, Or<atom, quoted_string> > {};

			//- phrase          =   1*word / obs-phrase
			struct phrase : Rule < phrase, Or<OneOrMore<word>, obs_phrase> > {};

			//- unstructured    =   (*([FWS] VCHAR) *WSP) / obs-unstruct
			struct unstructured : Rule < unstructured, Or<ZeroOrMore<ZeroOrOne<FWS>, VCHAR, ZeroOrOne<WSP>>, obs_unstruct> > {};

			//+ 3.3.  Date and Time Specification
			//- date-time       =   [ day-of-week "," ] date time [CFWS]
			struct date_time : Rule < date_time, ZeroOrOne<day_of_week, Comma>, date, time, ZeroOrOne<CFWS> > {};

			//- day-of-week     =   ([FWS] day-name) / obs-day-of-week
			struct day_of_week : Rule < day_of_week, Or<And<ZeroOrOne<FWS>, day_name>, obs_day_of_week> > {};

			//- day-name        =   "Mon" / "Tue" / "Wed" / "Thu" / "Fri" / "Sat" / "Sun"
			struct day_name : Rule < day_name, Mon, Tue, Wed, Thu, Fri, Sat, Sun > {};

			//- date            =   day month year
			struct date : Rule < date, day, month, year > {};

			//- day             =   ([FWS] 1*2DIGIT FWS) / obs-day
			struct day : Rule < day, Or<And<ZeroOrOne<FWS>, Repeat<Digit, 1, 2>, FWS>, obs_day> > {};

			//- month           =   "Jan" / "Feb" / "Mar" / "Apr" / "May" / "Jun" / "Jul" / "Aug" / "Sep" / "Oct" / "Nov" / "Dec"
			struct month : Rule < month, Or<Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec> > {};

			//- year            =   (FWS 4*DIGIT FWS) / obs-year
			struct year : Rule < year, Or<And<FWS, Repeat<Digit, 4, -1>, FWS>, obs_year> > {};

			//- time            =   time-of-day zone
			struct time : Rule < time, time_of_day, zone > {};

			//- time-of-day     =   hour ":" minute [ ":" second ]
			struct time_of_day : Rule < time_of_day, hour, Colon, minute, Colon, ZeroOrOne<Colon, second> > {};

			//- hour            =   2DIGIT / obs-hour
			struct hour : Rule < hour, Or<Repeat<DIGIT, 2>, obs_hour> > {};

			//- minute          =   2DIGIT / obs-minute
			struct minute : Rule < minute, Or<Repeat<DIGIT, 2>, obs_minute> > {};

			//- second          =   2DIGIT / obs-second
			struct second : Rule < second, Or<Repeat<DIGIT, 2>, obs_second> > {};

			//- zone            =   (FWS ( "+" / "-" ) 4DIGIT) / obs-zone
			struct zone : Rule < zone, Or< And<FWS, Or<Plus, Hyphen>, Repeat<DIGIT, 4>>, obs_zone> > {};

			//+ 3.4.  Address Specification
			//- address         =   mailbox / group
			struct address : Rule < address, Or<mailbox, group> > {};

			//- mailbox         =   name-addr / addr-spec
			struct mailbox : Rule < mailbox, Or<name_addr, addr_spec> > {};

			//- name-addr       =   [display-name] angle-addr
			struct name_addr : Rule < name_addr, ZeroOrOne<display_name>, angle_addr > {};

			//- angle-addr      =   [CFWS] "<" addr-spec ">" [CFWS] / obs-angle-addr
			struct angle_addr : Rule < angle_addr, Or<And<ZeroOrOne<CFWS>, LessThan, addr_spec, GreaterThan, ZeroOrOne<CFWS>>, obs_angle_addr> > {};

			//- group           =   display-name ":" [group-list] ";" [CFWS]
			struct group : Rule < group, display_name, Colon, ZeroOrOne<group_list>, SemiColon, ZeroOrOne<CFWS> > {};

			//- display-name    =   phrase
			struct display_name : Rule < display_name, phrase > {};

			//- mailbox-list    =   (mailbox *("," mailbox)) / obs-mbox-list
			struct mailbox_list : Rule < mailbox_list, Or<And<mailbox, ZeroOrMore<Comma, mailbox>>, obs_mbox_list> > {};

			//- address-list    =   (address *("," address)) / obs-addr-list
			struct address_list : Rule < address_list, Or<And<address, ZeroOrMore<Comma, address>>, obs_addr_list> > {};

			//- group-list      =   mailbox-list / CFWS / obs-group-list
			struct group_list : Rule < group_list, Or<mailbox_list, CFWS, obs_group_list> > {};

			//+ 3.4.1.  Addr-Spec Specification
			//- addr-spec       =   local-part "@" domain
			struct addr_spec : Rule < addr_spec, local_part, At, domain > {};

			//- local-part      =   dot-atom / quoted-string / obs-local-part
			struct local_part : Rule < local_part, Or<dot_atom, quoted_string, obs_local_part> > {};

			//- domain          =   dot-atom / domain-literal / obs-domain
			struct domain : Rule < domain, Or<dot_atom, domain_literal, obs_domain> > {};

			//- domain-literal  =   [CFWS] "[" *([FWS] dtext) [FWS] "]" [CFWS]
			struct domain_literal : Rule < domain_literal, ZeroOrOne<CFWS>, OpenBracket, ZeroOrMore<ZeroOrOne<FWS>, dtext>, ZeroOrOne<FWS>, CloseBracket, ZeroOrOne<CFWS> > {};

			//dtext           =   %d33-90 / %d94 - 126 / obs - dtext
			struct dtext : Rule < dtext, Or<Character<char, 33, 90>, Character<char, 94, 126>, obs_dtext> > {};

			//+ 3.5.  Overall Message Syntax
			//-  message         =   (fields / obs-fields) [CRLF body]
			struct message : Rule < message, Or<fields, obs_fields>, ZeroOrOne<CRLF, body> > {};

			//- body            =   (*(*998text CRLF) *998text) / obs-body
			struct body : Rule < body, Or<And<ZeroOrMore<Repeat<text, 0, 998>, CRLF>, Repeat<text, 0, 998>>, obs_body> > {};

			//- text            =   %d1-9 / %d11 / %d12 / %d14 - 127
			struct text : Rule < text, Or<Character<char, 1, 9>, Character<char, 11>, Character<char, 12>, Character<char, 14, 127>> > {};

			//+ 3.6.  Field Definitions
			/*
			fields          =   *(trace *optional-field / *(resent-date / resent-from / resent-sender / resent-to / resent-cc / resent-bcc / resent-msg-id))
			*(orig-date / from / sender / reply-to / to / cc / bcc / message-id / in-reply-to / references / subject / comments / keywords / optional-field)
			*/
			struct fields : Rule < fields,
				ZeroOrMore<trace, Or<ZeroOrMore<optional_field>, ZeroOrMore<Or<resent_date, resent_from, resent_sender, resent_to, resent_cc, resent_bcc, resent_msg_id>>>>,
				ZeroOrMore < Or<orig_date, from, sender, reply_to, to, cc, bcc, message_id, in_reply_to, references, subject, comments, keywords, optional_field> >
			> {};

			//+ 3.6.1.  The Origination Date Field
			//- orig-date       =   "Date:" date-time CRLF
			struct orig_date : Rule < orig_date, orig_date_prefix, date_time, CRLF > {};

			//+ 3.6.2.  Originator Fields
			//- from            =   "From:" mailbox-list CRLF
			struct from : Rule < from, from_prefix, mailbox_list, CRLF > {};

			//- sender = "Sender:" mailbox CRLF
			struct sender : Rule < sender, sender_prefix, mailbox, CRLF > {};

			//- reply-to = "Reply-To:" address-list CRLF
			struct reply_to : Rule < reply_to, reply_to_prefix, address_list, CRLF > {};

			//+ 3.6.3.  Destination Address Fields
			//-  to              =   "To:" address-list CRLF
			struct to : Rule < to, to_prefix, address_list, CRLF > {};

			//- cc              =   "Cc:" address-list CRLF
			struct cc : Rule < cc, cc_prefix, address_list, CRLF > {};

			//- bcc             =   "Bcc:" [address-list / CFWS] CRLF
			struct bcc : Rule < bcc, bcc_prefix, ZeroOrOne<Or<address_list, CFWS>>, CRLF > {};

			//+ 3.6.4.  Identification Fields
			//- message-id      =   "Message-ID:" msg-id CRLF
			struct message_id : Rule < message_id, message_id_prefix, msg_id, CRLF > {};

			//-   in-reply-to     =   "In-Reply-To:" 1*msg-id CRLF
			struct in_reply_to : Rule < in_reply_to, in_reply_to_prefix, OneOrMore<msg_id>, CRLF > {};

			//-  references      =   "References:" 1*msg-id CRLF
			struct references : Rule < references, references_prefix, OneOrMore<msg_id>, CRLF > {};

			//-  msg-id          =   [CFWS] "<" id-left "@" id-right ">" [CFWS]
			struct msg_id : Rule < msg_id, ZeroOrOne<CFWS>, LessThan, id_left, At, id_right, GreaterThan, ZeroOrOne<CFWS> > {};

			//- id-left         =   dot-atom-text / obs-id-left
			struct id_left : Rule < id_left, Or<dot_atom_text, obs_id_left> > {};

			//- id-right        =   dot-atom-text / no-fold-literal / obs-id-right
			struct id_right : Rule < id_right, Or<dot_atom_text, no_fold_literal, obs_id_right> > {};

			//-  no-fold-literal =   "[" *dtext "]"
			struct no_fold_literal : Rule < no_fold_literal, OpenBracket, ZeroOrMore<dtext>, CloseBracket > {};

			//+ 3.6.5.  Informational Fields
			//-  subject         =   "Subject:" unstructured CRLF
			struct subject : Rule < subject, subject_prefix, unstructured, CRLF > {};

			//-comments        =   "Comments:" unstructured CRLF
			struct comments : Rule < comments, comments_prefix, unstructured, CRLF > {};

			//-  keywords        =   "Keywords:" phrase *("," phrase) CRLF
			struct keywords : Rule < keywords, keywords_prefix, phrase, ZeroOrOne<Comma, phrase>, CRLF > {};

			//+ 3.6.6.  Resent Fields
			//- resent-date     =   "Resent-Date:" date-time CRLF
			struct resent_date : Rule < resent_date, resent_date_prefix, date_time, CRLF > {};

			//-   resent-from     =   "Resent-From:" mailbox-list CRLF
			struct resent_from : Rule < resent_from, resent_from_prefix, mailbox_list, CRLF > {};

			//-   resent-sender   =   "Resent-Sender:" mailbox CRLF
			struct resent_sender : Rule < resent_sender, resent_sender_prefix, mailbox, CRLF > {};

			//-   resent-to       =   "Resent-To:" address-list CRLF
			struct resent_to : Rule < resent_to, resent_to_prefix, address_list, CRLF > {};

			//-   resent-cc       =   "Resent-Cc:" address-list CRLF
			struct resent_cc : Rule < resent_cc, resent_cc_prefix, address_list, CRLF > {};

			//-   resent-bcc      =   "Resent-Bcc:" [address-list / CFWS] CRLF
			struct resent_bcc : Rule < resent_bcc, resent_bcc_prefix, ZeroOrOne<Or<address_list, CFWS>>, CRLF > {};

			//-   resent-msg-id   =   "Resent-Message-ID:" msg-id CRLF
			struct resent_msg_id : Rule < resent_msg_id, resent_msg_id_prefix, msg_id, CRLF > {};

			//+ 3.6.7.  Trace Fields
			//- trace           =   [return] 1 * received
			struct trace : Rule < trace, ZeroOrOne<Return>, OneOrMore<received> > {};

			//- return          =   "Return-Path:" path CRLF
			struct Return : Rule < Return, return_prefix, path, CRLF > {};

			//- path            =   angle-addr / ([CFWS] "<" [CFWS] ">" [CFWS])
			struct path : Rule < path, Or<angle_addr, And<ZeroOrOne<CFWS>, LessThan>> > {};

			//- received        =   "Received:" *received-token ";" date-time CRLF
			struct received : Rule < received, received_prefix, ZeroOrMore<received_token>, SemiColon, date_time, CRLF > {};

			//- received-token  =   word / angle-addr / addr-spec / domain
			struct received_token : Rule < received_token, Or<word, angle_addr, addr_spec, domain> > {};

			//+ 3.6.8.  Optional Fields
			//- optional-field  =   field-name ":" unstructured CRLF
			struct optional_field : Rule < optional_field, field_name, Colon, unstructured, CRLF > {};

			//- field-name      =   1*ftext
			struct field_name : Rule < field_name, OneOrMore<ftext> > {};

			//-   ftext           =   %d33-57 /  %d59-126
			struct ftext : Rule < ftext, Or<Character<char, 33, 57>, Character<char, 59, 126>> > {};

			//+ 4.  Obsolete Syntax
			//+ 4.1.  Miscellaneous Obsolete Tokens

			//-  obs-NO-WS-CTL   =   %d1-8 /   %d11 /  %d12 /  %d14-31 /   %d127
			struct obs_NO_WS_CTL : Rule < obs_NO_WS_CTL, Or<Character<char, 1, 8>, Character<char, 11>, Character<char, 12>, Character<char, 14, 31>, Character<char, 127>> > {};

			//-   obs-ctext       =   obs-NO-WS-CTL
			struct obs_ctext : Rule < obs_ctext, obs_NO_WS_CTL > {};

			//-  obs-qtext       =   obs-NO-WS-CTL
			struct obs_qtext : Rule < obs_qtext, obs_NO_WS_CTL > {};

			//-  obs-utext       =   %d0 / obs-NO-WS-CTL / VCHAR
			struct obs_utext : Rule < obs_utext, Or<Character<char, 0>, obs_NO_WS_CTL, VCHAR> > {};

			//- obs-qp          =   "\" (%d0 / obs-NO-WS-CTL / LF / CR)
			struct obs_qp : Rule < obs_qp, BackSlash, Or<Character<char, 0>, obs_NO_WS_CTL, LF, CR> > {};

			//-  obs-body        =   *((*LF *CR *((%d0 / text) *LF *CR)) / CRLF)
			struct obs_body : Rule < obs_body, ZeroOrMore<Or<And<ZeroOrMore<LF>, ZeroOrMore<CR>, ZeroOrMore<Or<Character<char, 0>, text>, ZeroOrMore<CR>, ZeroOrMore<LF>>>, CRLF>> > {};

			//-  obs-unstruct    =   *((*LF *CR *(obs-utext *LF *CR)) / FWS)
			struct obs_unstruct : Rule < obs_unstruct, ZeroOrMore<Or<And<ZeroOrMore<LF>, ZeroOrMore<CR>, ZeroOrMore<obs_utext, ZeroOrMore<LF>, ZeroOrMore<CR>>>, FWS >> > {};

			//-  obs-phrase      =   word *(word / "." / CFWS)
			struct obs_phrase : Rule < obs_phrase, word, ZeroOrMore<Or<word, Period, CFWS>> > {};

			//-obs-phrase-list =   [phrase / CFWS] *("," [phrase / CFWS])
			struct obs_phrase_list : Rule < obs_phrase_list, ZeroOrOne<Or<phrase, CFWS>>, ZeroOrMore<Comma, Or<phrase, CFWS>> > {};

			//+ 4.2.  Obsolete Folding White Space

			//-obs-FWS         =   1*WSP *(CRLF 1*WSP)
			struct obs_FWS : Rule < obs_FWS, OneOrMore<WSP>, ZeroOrOne<CRLF, OneOrMore<WSP>> > {};

			//++ 4.3.  Obsolete Date and Time

			//- obs-day-of-week =   [CFWS] day-name [CFWS]
			struct obs_day_of_week : Rule < obs_day_of_week, ZeroOrOne<CFWS>, day_name, ZeroOrOne<CFWS> > {};

			//- obs-day         =   [CFWS] 1*2DIGIT [CFWS]
			struct obs_day : Rule < obs_day, ZeroOrOne<CFWS>, Repeat<Digit, 1, 2>, ZeroOrOne<CFWS> > {};

			//- obs-year        =   [CFWS] 2*DIGIT [CFWS]
			struct obs_year : Rule < obs_year, ZeroOrOne<CFWS>, Repeat<Digit, 2, -1>, ZeroOrOne<CFWS> > {};

			//- obs-hour        =   [CFWS] 2DIGIT [CFWS]
			struct obs_hour : Rule < obs_hour, ZeroOrOne<CFWS>, Repeat<Digit, 2>, ZeroOrOne<CFWS> > {};

			//- obs-minute      =   [CFWS] 2DIGIT [CFWS]
			struct obs_minute : Rule < obs_minute, ZeroOrOne<CFWS>, Repeat<Digit, 2>, ZeroOrOne<CFWS> > {};

			//- obs-second      =   [CFWS] 2DIGIT [CFWS]
			struct obs_second : Rule < obs_second, ZeroOrOne<CFWS>, Repeat<Digit, 2>, ZeroOrOne<CFWS> > {};

			//- obs-zone        =   "UT" / "GMT" /   "EST" / "EDT" /  "CST" / "CDT" / "MST" / "MDT" / "PST" / "PDT" / %d65-73 / %d75-90 / %d97-105 / %d107-122
			struct obs_zone : Rule < obs_zone, Or<UT, GMT, EST, EDT, CST, CDT, MST, MDT, PST, PDT, Character<char, 65, 73>, Character<char, 75, 90>, Character<char, 97, 105>, Character<char, 107, 122>> > {};

			//+ 4.4.  Obsolete Addressing

			//- obs-angle-addr  =   [CFWS] "<" obs-route addr-spec ">" [CFWS]
			struct obs_angle_addr : Rule < obs_angle_addr, ZeroOrOne<CFWS>, LessThan, obs_route, addr_spec, GreaterThan, ZeroOrOne<CFWS> > {};

			//- obs-route       =   obs-domain-list ":"
			struct obs_route : Rule < obs_route, obs_domain_list, Colon > {};

			//- obs-domain-list =   *(CFWS / ",") "@" domain *("," [CFWS] ["@" domain])
			struct obs_domain_list : Rule < obs_domain_list, ZeroOrMore<Or<CFWS, Comma>>, At, domain, ZeroOrMore<Comma, ZeroOrOne<CFWS>, ZeroOrOne<At, domain>> > {};

			//- obs-mbox-list   =   *([CFWS] ",") mailbox *("," [mailbox / CFWS])
			struct obs_mbox_list : Rule < obs_mbox_list, ZeroOrMore<ZeroOrOne<CFWS>, Comma>, mailbox, ZeroOrMore<Comma, ZeroOrOne<Or<mailbox, CFWS>>> > {};

			//- obs-addr-list   =   *([CFWS] ",") address *("," [address / CFWS])
			struct obs_addr_list : Rule < obs_addr_list, ZeroOrMore<ZeroOrOne<CFWS>, Comma>, address, ZeroOrMore<Comma, ZeroOrOne<Or<address, CFWS>>> > {};

			//- obs-group-list  =   1*([CFWS] ",") [CFWS]
			struct obs_group_list : Rule < obs_group_list, OneOrMore<ZeroOrOne<CFWS>, Comma>, ZeroOrOne<CFWS> > {};

			//- obs-local-part  =   word *("." word)
			struct obs_local_part : Rule < obs_local_part, word, ZeroOrMore<Period, word> > {};

			//- obs-domain      =   atom *("." atom)
			struct obs_domain : Rule < obs_domain, atom, ZeroOrMore<Period, atom> > {};

			//- obs-dtext       =   obs-NO-WS-CTL / quoted-pair
			struct obs_dtext : Rule < obs_dtext, Or<obs_NO_WS_CTL, quoted_pair> > {};

			//+ 4.5.  Obsolete Header Fields

			struct obs_field : Rule < obs_field, Or<obs_return, obs_received, obs_orig_date, obs_from, obs_sender, obs_reply_to, obs_to, obs_cc, obs_bcc, obs_message_id, obs_in_reply_to, obs_references, obs_subject, obs_comments, obs_keywords, obs_resent_date, obs_resent_from, obs_resent_send, obs_resent_rply, obs_resent_to, obs_resent_cc, obs_resent_bcc, obs_resent_mid, obs_optional> > {};

			//+ 4.5.1.  Obsolete Origination Date Field

			//- obs-orig-date   =   "Date" *WSP ":" date-time CRLF
			struct obs_orig_date : Rule < obs_orig_date > {};

			//+ 4.5.2.  Obsolete Originator Fields

			//- obs-from        =   "From" *WSP ":" mailbox-list CRLF
			struct obs_from : Rule < obs_from > {};

			//- obs-sender      =   "Sender" *WSP ":" mailbox CRLF
			struct obs_sender : Rule < obs_sender > {};

			//- obs-reply-to    =   "Reply-To" *WSP ":" address-list CRLF
			struct obs_reply_to : Rule < obs_reply_to > {};

			//+ 4.5.3.  Obsolete Destination Address Fields

			//- obs-to          =   "To" *WSP ":" address-list CRLF
			struct obs_to : Rule < obs_to > {};

			//- obs-cc          =   "Cc" *WSP ":" address-list CRLF
			struct obs_cc : Rule < obs_cc > {};

			//- obs-bcc         =   "Bcc" *WSP ":" (address-list / (*([CFWS] ",") [CFWS])) CRLF
			struct obs_bcc : Rule < obs_bcc > {};

			//+ 4.5.4.  Obsolete Identification Fields

			//- obs-message-id  =   "Message-ID" *WSP ":" msg-id CRLF
			struct obs_message_id : Rule < obs_message_id > {};

			//- obs-in-reply-to =   "In-Reply-To" *WSP ":" *(phrase / msg-id) CRLF
			struct obs_in_reply_to : Rule < obs_in_reply_to > {};

			//- obs-references  =   "References" *WSP ":" *(phrase / msg-id) CRLF
			struct obs_references : Rule < obs_references > {};

			//- obs-id-left     =   local-part
			struct obs_id_left : Rule < obs_id_left > {};

			//- obs-id-right    =   domain
			struct obs_id_right : Rule < obs_id_right > {};

			//+ 4.5.5.  Obsolete Informational Fields

			//- obs-subject     =   "Subject" *WSP ":" unstructured CRLF
			struct obs_subject : Rule < obs_subject > {};

			//- obs-comments    =   "Comments" *WSP ":" unstructured CRLF
			struct obs_comments : Rule < obs_comments > {};

			//- obs-keywords    =   "Keywords" *WSP ":" obs-phrase-list CRLF
			struct obs_keywords : Rule < obs_keywords > {};

			//+ 4.5.6.  Obsolete Resent Fields

			//- obs-resent-from =   "Resent-From" *WSP ":" mailbox-list CRLF
			struct obs_resent_from : Rule < obs_resent_from > {};

			//- obs-resent-send =   "Resent-Sender" *WSP ":" mailbox CRLF
			struct obs_resent_send : Rule < obs_resent_send > {};

			//- obs-resent-date =   "Resent-Date" *WSP ":" date-time CRLF
			struct obs_resent_date : Rule < obs_resent_date > {};

			//- obs-resent-to   =   "Resent-To" *WSP ":" address-list CRLF
			struct obs_resent_to : Rule < obs_resent_to > {};

			//- obs-resent-cc   =   "Resent-Cc" *WSP ":" address-list CRLF
			struct obs_resent_cc : Rule < obs_resent_cc > {};

			//- obs-resent-bcc  =   "Resent-Bcc" *WSP ":" (address-list / (*([CFWS] ",") [CFWS])) CRLF
			struct obs_resent_bcc : Rule < obs_resent_bcc > {};

			//- obs-resent-mid  =   "Resent-Message-ID" *WSP ":" msg-id CRLF
			struct obs_resent_mid : Rule < obs_resent_mid > {};

			//- obs-resent-rply =   "Resent-Reply-To" *WSP ":" address-list CRLF
			struct obs_resent_rply : Rule < obs_resent_rply > {};

			//+ 4.5.7.  Obsolete Trace Fields

			//- obs-return      =   "Return-Path" *WSP ":" path CRLF
			struct obs_return : Rule < obs_return > {};

			//- obs-received    =   "Received" *WSP ":" *received-token CRLF
			struct obs_received : Rule < obs_received > {};

			//+ 4.5.8.  Obsolete optional fields

			//- obs-optional    =   field-name *WSP ":" unstructured CRLF
			struct obs_optional : Rule < obs_optional > {};
#pragma endregion
		}
	}
}

#endif //__RFC_5322_HPP_INCLUDED__
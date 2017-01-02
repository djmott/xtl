#ifndef __RFC_7231_HPP_INCLUDED__
#define __RFC_7231_HPP_INCLUDED__

/*
Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content
https://tools.ietf.org/html/rfc7231
*/

namespace xtd {
	namespace Grammars {
		namespace RFC7231 {
			using namespace xtd::Parser;
			using namespace xtd::Parser::MultiByte;
#pragma region("forward declerations")
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

#pragma region("strings")
			STRING(QEQUAL, "q=");
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
#pragma endregion

#pragma region("rules")
			//Accept = [ ( "," / ( media-range [ accept-params ] ) ) *( OWS "," [OWS ( media-range [ accept-params ] ) ] ) ]
			struct accept : Rule < accept, ZeroOrOne< Or<Comma, And<media_range, ZeroOrOne<accept_params>>, ZeroOrMore<OWS, Comma, ZeroOrOne< OWS, media_range, ZeroOrOne<accept_params>>>>> > {};

			//Accept-Charset = *( "," OWS ) ( ( charset / "*" ) [ weight ] ) *( OWS "," [ OWS ( ( charset / "*" ) [ weight ] ) ] )
			struct Accept_Charset : Rule < Accept_Charset, ZeroOrMore<Comma, OWS>, Or<charset, Asterisk>, ZeroOrOne<weight>, ZeroOrMore< OWS, Comma, ZeroOrOne<OWS, Or<charset, Asterisk>, ZeroOrOne<weight>>> > {};

			//Accept-Encoding = [ ( "," / ( codings [ weight ] ) ) *( OWS "," [ OWS ( codings [ weight ] ) ] ) ]
			struct Accept_Encoding : Rule < Accept_Encoding, ZeroOrOne<Or<Comma, And<codings, ZeroOrOne<weight>>>, ZeroOrMore<OWS, Comma, ZeroOrOne<OWS, codings, ZeroOrOne<weight>>>> > {};

			//Accept-Language = *( "," OWS ) ( language-range [ weight ] ) *( OWS "," [ OWS ( language-range [ weight ] ) ] )
			struct Accept_Language : Rule < Accept_Language, ZeroOrMore<Comma, OWS>, language_range, ZeroOrOne<weight>, ZeroOrMore<OWS, Comma, ZeroOrOne<OWS, language_range, ZeroOrOne<weight>>> > {};

			//Allow = [ ( "," / method ) *( OWS "," [ OWS method ] ) ]
			struct Allow : Rule < Allow, ZeroOrOne<Or<Comma, method>, ZeroOrMore<OWS, Comma, ZeroOrOne<OWS, method>>> > {};

			//Content-Encoding = *( "," OWS ) content-coding *( OWS "," [ OWS content-coding ] )
			struct Content_Encoding : Rule < Content_Encoding, ZeroOrMore<Comma, OWS>, content_coding, ZeroOrMore<OWS, Comma, ZeroOrOne<OWS, content_coding>> > {};

			//Content-Language = *( "," OWS ) language-tag *( OWS "," [ OWS language-tag ] )
			struct Content_Language : Rule < Content_Language, ZeroOrMore<Comma, OWS>, language_tag, ZeroOrMore<OWS, Comma, ZeroOrOne<OWS, language_tag>> > {};

			//Content-Location = absolute-URI / partial-URI
			struct Content_Location : Rule < Content_Location, Or<absolute_URI, partial_URI> > {};

			//Content-Type = media-type
			struct Content_Type : Rule < Content_Type, media_type > {};

			//Date = HTTP-date
			struct Date : Rule < Date, HTTP_date > {};

			//Expect = "100-continue"
			struct Expect : Rule < Expect, _100_continue > {};

			//From = mailbox
			struct From : Rule < From, mailbox > {};

			//HTTP-date = IMF-fixdate / obs-date
			struct HTTP_date : Rule < HTTP_date, Or<IMF_fixdate, obs_date> > {};

			//IMF-fixdate = day-name "," SP date1 SP time-of-day SP GMT
			struct IMF_fixdate : Rule < IMF_fixdate, day_name, Comma, Space, date1, Space, time_of_day, Space, GMT > {};

			//Location = URI-reference
			struct Location : Rule < Location, URI_reference > {};

			//Max-Forwards = 1*DIGIT
			struct Max_Forwards : Rule < Max_Forwards, OneOrMore<Digit> > {};

			//Referer = absolute-URI / partial-URI
			struct Referer : Rule < Referer, Or<absolute_URI, partial_URI> > {};

			//Retry-After = HTTP-date / delay-seconds
			struct Retry_After : Rule < Retry_After, Or<HTTP_date, delay_seconds> > {};

			//Server = product *( RWS ( product / comment ) )
			struct Server : Rule < Server, ZeroOrMore<RWS, Or<product, comment>> > {};

			//User-Agent = product *( RWS ( product / comment ) )
			struct User_Agent : Rule < User_Agent, product, ZeroOrMore<RWS, Or<product, comment>> > {};

			//Vary = "*" / ( *( "," OWS ) field-name *( OWS "," [ OWS field-name ] ) )
			struct Vary : Rule < Vary, Or<Asterisk, And<ZeroOrMore<Comma, OWS>, field_name, ZeroOrMore<OWS, Comma, ZeroOrOne<OWS, field_name>>>> > {};

			//accept-ext = OWS ";" OWS token [ "=" ( token / quoted-string ) ]
			struct accept_ext : Rule < accept_ext, OWS, SemiColon, OWS, token, ZeroOrOne<Equal, Or<token, quoted_string>> > {};

			//accept-params = weight *accept-ext
			struct accept_params : Rule < accept_params, weight, ZeroOrMore<accept_ext> > {};

			//asctime-date = day-name SP date3 SP time-of-day SP year
			struct asctime_date : Rule < asctime_date, day_name, Space, date3, Space, time_of_day, Space, year > {};

			//charset = token
			struct charset : Rule < charset, token > {};

			//codings = content-coding / "identity" / "*"
			struct codings : Rule < codings, Or<content_coding, identity, Asterisk> > {};

			//content-coding = token
			struct content_coding : Rule < content_coding, token > {};

			//date1 = day SP month SP year
			struct date1 : Rule < date1, day, Space, month, Space, year > {};

			//date2 = day "-" month "-" 2DIGIT
			struct date2 : Rule < date2, day, Hyphen, month, Hyphen, Repeat<Digit, 2> > {};

			//date3 = month SP ( 2DIGIT / ( SP DIGIT ) )
			struct date3 : Rule < date3, month, Space, Or<Repeat<Digit, 2>, And<Space, Digit>> > {};

			//day = 2DIGIT
			struct day : Rule < day, Repeat<Digit, 2> > {};

			/*
			day-name = %x4D.6F.6E ; Mon
			/ %x54.75.65 ; Tue
			/ %x57.65.64 ; Wed
			/ %x54.68.75 ; Thu
			/ %x46.72.69 ; Fri
			/ %x53.61.74 ; Sat
			/ %x53.75.6E ; Sun
			*/
			struct day_name : Rule < day_name, Or<Mon, Tue, Wed, Thu, Fri, Sat, Sun> > {};
			/*
			day-name-l = %x4D.6F.6E.64.61.79 ; Monday
			/ %x54.75.65.73.64.61.79 ; Tuesday
			/ %x57.65.64.6E.65.73.64.61.79 ; Wednesday
			/ %x54.68.75.72.73.64.61.79 ; Thursday
			/ %x46.72.69.64.61.79 ; Friday
			/ %x53.61.74.75.72.64.61.79 ; Saturday
			/ %x53.75.6E.64.61.79 ; Sunday
			*/
			struct day_name_1 : Rule < day_name_1, Or<Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday> > {};

			//delay-seconds = 1*DIGIT
			struct delay_seconds : Rule < delay_seconds, OneOrMore<Digit> > {};

			//hour = 2DIGIT
			struct hour : Rule < hour, Repeat<Digit, 2> > {};

			//media-range = ( "*/*" / ( type "/*" ) / ( type "/" subtype ) ) *( OWS ";" OWS parameter )
			struct media_range : Rule < media_range, Or<media_range_all, And<type, media_subrange_all>, And<type, ForwardSlash, subtype>>, ZeroOrMore<OWS, SemiColon, OWS, parameter> > {};

			//media-type = type "/" subtype *( OWS ";" OWS parameter )
			struct media_type : Rule < media_type, type, ForwardSlash, subtype, ZeroOrMore<OWS, SemiColon, parameter> > {};

			//method = token
			struct method : Rule < method, token > {};

			//minute = 2DIGIT
			struct minute : Rule < minute, Repeat<Digit, 2> > {};

			/*
			month = %x4A.61.6E ; Jan
			/ %x46.65.62 ; Feb
			/ %x4D.61.72 ; Mar
			/ %x41.70.72 ; Apr
			/ %x4D.61.79 ; May
			/ %x4A.75.6E ; Jun
			/ %x4A.75.6C ; Jul
			/ %x41.75.67 ; Aug
			/ %x53.65.70 ; Sep
			/ %x4F.63.74 ; Oct
			/ %x4E.6F.76 ; Nov
			/ %x44.65.63 ; Dec
			*/
			struct month : Rule < month, Or<Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec> > {};

			//obs-date = rfc850-date / asctime-date
			struct obs_date : Rule < obs_date, Or<rfc850_date, asctime_date> > {};

			//parameter = token "=" ( token / quoted-string )
			struct parameter : Rule < parameter, token, Equal, Or<token, quoted_string> > {};

			//product = token [ "/" product-version ]
			struct product : Rule < product, token, ZeroOrOne<ForwardSlash, product_version> > {};

			//product-version = token
			struct product_version : Rule < product_version, token > {};

			//qvalue = ( "0" [ "." *3DIGIT ] ) / ( "1" [ "." *3"0" ] )
			struct qvalue : Rule < qvalue, Or<And<_0, ZeroOrOne<Period, Repeat<Digit, 0, 3>>>, And<_1, ZeroOrOne<Period, Repeat<_0, 0, 3>>>> > {};

			//rfc850-date = day-name-l "," SP date2 SP time-of-day SP GMT
			struct rfc850_date : Rule < rfc850_date, day_name_1, Comma, Space, date2, Space, time_of_day, Space, GMT > {};

			//second = 2DIGIT
			struct second : Rule < second, Repeat<Digit, 2> > {};
			//subtype = token
			struct subtype : Rule < subtype, token > {};
			//time-of-day = hour ":" minute ":" second
			struct time_of_day : Rule < time_of_day, hour, Colon, minute, Colon, second > {};

			//type = token
			struct type : Rule < type, token > {};

			//weight = OWS ";" OWS "q=" qvalue
			struct weight : Rule < weight, OWS, SemiColon, QEQUAL, qvalue > {};

			//year = 4DIGIT
			struct year : Rule < year, Repeat<Digit, 4> > {};
#pragma endregion
		}
	}
}

#endif //__RFC_7231_HPP_INCLUDED__

#if __BUILD_UNIT_TESTS__

class RFC7231_Test : public ParserTest < RFC7231_Test > {
public:

	//2.7.1.  http URI Scheme
	TEST_METHOD(RFC7231_Tests) {
		using namespace xtd::Grammars;
		PassGrammar<RFC7231::accept>("text/html");// , application / xhtml + xml, application / xml; q = 0.9, image / webp, */*;q=0.8"));
	}
};

#endif //__BUILD_DEBUG__
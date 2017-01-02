#ifndef __RFC_2616_HPP_INCLUDED__
#define __RFC_2616_HPP_INCLUDED__

/*
Hypertext Transfer Protocol -- HTTP/1.1
https://tools.ietf.org/html/rfc2616
*/

namespace xtd {
	namespace Grammars {
		namespace RFC2616 {
			using namespace xtd::Parser;
			using namespace xtd::Parser::MultiByte;
#pragma region("forward declerations")
			struct asctime_date;
			struct attribute;
			struct chunk;
			struct chunk_data;
			struct chunk_extension;
			struct chunk_ext_name;
			struct chunk_ext_val;
			struct chunk_size;
			struct Chunked_Body;
			struct comment;
			struct entity_header;
			struct HTTP_date;
			struct http_URL;
			struct media_type;
			struct last_chunk;
			struct parameter;
			struct product;
			struct product_version;
			struct quoted_string;
			struct qvalue;
			struct subtype;
			struct trailer;
			struct transfer_coding;
			struct transfer_extension;
			struct type;
			struct value;
#pragma endregion

#pragma region("strings")
#pragma endregion

#pragma region("imports")
#pragma endregion

#pragma region("rules")
			//+ 2.2 Basic Rules
			//-  UPALPHA        = <any US-ASCII uppercase letter "A".."Z">
			using UPALPHA = Character < char, 'A', 'Z' > ;

			//-  LOALPHA        = <any US-ASCII lowercase letter "a".."z">
			using LOALPHA = Character < char, 'a', 'z' > ;

			//- ALPHA          = UPALPHA | LOALPHA
			struct ALPHA : Rule < ALPHA, Or<UPALPHA, LOALPHA> > {};

			//+ 14.4 Accept-Language

			//- language-range  = ( ( 1*8ALPHA *( "-" 1*8ALPHA ) ) | "*" )
			struct language_range : Rule < language_range, Or< And<Repeat<ALPHA, 1, 8>, ZeroOrMore<MultiByte::Hyphen, Repeat<ALPHA, 1, 8>>>, MultiByte::Asterisk > > {};

#pragma endregion

			// #pragma region("3.1")
			// 			REGEX(HTTP_Version, "HTTP\\/\\d+\\.\\d+");
			// #pragma endregion
			//
			// 			REGEX(OCTET, ".");
			// 			REGEX(CHAR, "[\\x00-\\x7f]");
			// 			REGEX(chunked, "chunked");
			//
			// 			REGEX(CTL, "[\\x00-\\x1f\\x7f]");
			// 			REGEX(ctext, "[\\x20-\\xff-[\\x7f\\(\\)]]|(\\x0d\\x0a)?(\\x20|\\x09)");
			// 			REGEX(HTTP_Protocol, "http:\\/\\/");
			//
			// 			REGEX(LWS, "(\\x0d\\x0a)?(\\x20|\\x09)");
			//
			// 			REGEX(qdtext, "[\\x20-\\xff-[\\x7f\\x22]]|(\\x0d\\x0a)?(\\x20|\\x09)");
			// 			REGEX(qvalue_1, "0\\.\\d{0,3}");
			// 			REGEX(qvalue_2, "1\\.0{0,3}");
			// 			REGEX(quoted_pair, "\\\\[\\x00-\\x7f]");
			//
			// 			REGEX(seperators, "'('|')'|'<'|'>'|'@'|','|';'|':'|'\\'|\\x22|'/'|'['|']'|'?'|'='|'{'|'}'|\\x20|\\x09");
			//
			// 			REGEX(TEXT, "[\\x20-\\xff-[\\x7f]]|(\\x0d\\x0a)?(\\x20|\\x09)");
			// 			REGEX(token, "[\\x20-\\xff-['('|')'|'<'|'>'|'@'|','|';'|':'|\\\\|\\x22|'/'|\\[|\\]|'?'|'='|'{'|'}'|\\x20|\\x09|\\x7f]]");
			//
			// 			struct attribute : Rule < attribute, token > {};
			// 			struct comment : Rule < comment, OpenParan, Or<ctext, quoted_pair, comment>, CloseParan > {};
			// 			struct chunk : Rule < chunk, Or <
			// 				And<chunk_size, ZeroOrOne<chunk_extension>, CRLF>,
			// 				And<chunk_data, CRLF>
			// 			> > {};
			// 			struct chunk_data : Rule < chunk_data, ZeroOrMore<OCTET> > {};
			// 			struct chunk_extension : Rule < chunk_extension, ZeroOrMore<SemiColon, chunk_ext_name, ZeroOrOne<Equal, chunk_ext_val>> > {};
			// 			struct chunk_ext_name : Rule < chunk_ext_name, token > {};
			// 			struct chunk_ext_val : Rule < chunk_ext_val, Or<token, quoted_string> > {};
			// 			struct chunk_size : Rule < chunk_size, HexDigit > {};
			// 			struct Chunked_Body : Rule < Chunked_Body, Or<ZeroOrMore<chunk>, last_chunk, trailer, CRLF> > {};
			// 			struct HTTP_date : Rule < HTTP_date, Or<RFC1123::Date, RFC850::Date, asctime_date> > {};
			// // 			struct http_URL : Rule < http_URL, HTTP_Protocol, RFC3986::host, ZeroOrOne<Colon, RFC2396::port>, ZeroOrOne<  RFC2396::abs_path, ZeroOrOne<Question, RFC2396::query>> > {};
			// 			struct media_type : Rule < media_type, type, ForwardSlash, subtype, ZeroOrMore <SemiColon, parameter>> {};
			// 			struct last_chunk : Rule < last_chunk, _0, ZeroOrOne<chunk_extension>, CRLF > {};
			// 			struct parameter : Rule < parameter, Equal, value > {};
			// 			struct product : Rule<product, token, ZeroOrOne<ForwardSlash, product_version>> {};
			// 			struct product_version : Rule<product_version, token> {};
			// 			struct quoted_string : Rule < quoted_string, DoubleQuote, ZeroOrMore<Or<qdtext, quoted_pair>>, DoubleQuote > {};
			// 			struct qvalue : Rule<qvalue, Or<qvalue_1, qvalue_2>> {};
			// 			struct subtype : Rule < subtype, token > {};
			// 			struct trailer : Rule < trailer, ZeroOrMore<entity_header, CRLF> > {};
			// 			struct transfer_coding : Rule < transfer_coding, Or<chunked, transfer_extension> > {};
			// 			struct transfer_extension : Rule<transfer_extension, token, ZeroOrMore<SemiColon, parameter>> {};
			// 			struct type : Rule < type, token > {};
			// 			struct value : Rule <value, Or<token, quoted_string>> {};
		}
	}
}

#endif //__RFC_2616_HPP_INCLUDED__

#if __BUILD_DEBUG__

/*
class RFC2616_Test : public ParserTest<RFC2616_Test>{
public:
TEST_METHOD(HTTP_Version_Test)
{
using namespace xtd::Grammars::RFC2616;
PassGrammar<HTTP_Version>("HTTP/2.4");
PassGrammar<HTTP_Version>("HTTP/1.0");
PassGrammar<HTTP_Version>("HTTP/1.1");
PassGrammar<HTTP_Version>("HTTP/2.13");
PassGrammar<HTTP_Version>("HTTP/12.3");
FailGrammar<HTTP_Version>("FTP/12.3");
}
};
*/

#endif
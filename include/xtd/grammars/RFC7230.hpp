#ifndef __RFC_7230_HPP_INCLUDED__
#define __RFC_7230_HPP_INCLUDED__

/*
Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing
https://tools.ietf.org/html/rfc7230
*/

namespace xtd {
	namespace Grammars {
		namespace RFC7230 {
			using namespace xtd::Parser;
			using namespace xtd::Parser::MultiByte;

#pragma region("forward declerations")

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

#pragma region("strings")
			STRING(chunked, "chunked");
			STRING(compress, "compress");
			STRING(deflate, "deflate");
			STRING(gzip, "gzip");
			STRING(HTTP_name, "HTTP");
			STRING(http_URI_prefix, "http://");
			STRING(https_URI_prefix, "https://");
			STRING(trailers, "trailers");
#pragma endregion

#pragma region("imports")
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
			using VCHAR = RFC5234::VCHAR;
#pragma endregion

#pragma region("rules")
			using OWS = ZeroOrMore < Or<Space, Tab> > ;
			using RWS = OneOrMore < Or<Space, Tab> > ;
			using BWS = OWS;
			struct Connection : Rule < Connection, ZeroOrMore<Comma, OWS>, connection_option, ZeroOrMore<OWS, Comma, ZeroOrMore<OWS, connection_option>> > {};
			struct Content_Length : Rule < Content_Length, OneOrMore<Digit> > {};
			struct HTTP_message : Rule < HTTP_message, start_line, ZeroOrMore<header_field, CRLF>, CRLF, ZeroOrOne<message_body> > {};
			struct HTTP_version : Rule < HTTP_version, HTTP_name, ForwardSlash, Digit, Period, Digit > {};
			struct Host : Rule < Host, uri_host, ZeroOrOne<Colon, port> > {};
			struct TE : Rule < TE, ZeroOrOne<Or<Comma, t_codings>, ZeroOrMore<OWS, Comma, ZeroOrOne<OWS, t_codings>>> > {};
			struct Trailer : Rule < Trailer, ZeroOrMore<Comma, OWS>, field_name, ZeroOrMore<OWS, Comma, ZeroOrOne<OWS, field_name>> > {};
			struct Transfer_Encoding : Rule < ZeroOrMore<Comma, OWS>, transfer_coding, ZeroOrMore<OWS, Comma, ZeroOrOne<OWS, transfer_coding>> > {};
			struct Upgrade : Rule < Upgrade, ZeroOrMore<Comma, OWS>, protocol, ZeroOrMore<OWS, Comma, ZeroOrOne<OWS, protocol>> > {};
			using Via_prefix = ZeroOrMore < Comma, OWS > ;
			using Via_suffix = ZeroOrMore < OWS, Comma, ZeroOrOne<OWS, received_protocol, RWS, received_by, ZeroOrOne<RWS, comment>> > ;
			struct Via : Rule < Via, Via_prefix, received_protocol, RWS, received_by, ZeroOrOne<RWS, comment>, Via_suffix > {};
			struct absolute_form : Rule < absolute_form, absolute_URI > {};
			struct absolute_path : Rule < absolute_path, OneOrMore<ForwardSlash, segment> > {};
			struct asterisk_form : Rule < asterisk_form, Asterisk > {};
			struct authority_form : Rule < authority_form, authority > {};
			struct chunk : Rule < chunk, chunk_size, ZeroOrOne<chunk_ext>, CRLF, chunk_data, CRLF > {};
			struct chunk_data : Rule < chunk_data, OneOrMore<Any> > {};
			struct chunk_ext_name : Rule < chunk_ext_name, token > {};
			struct chunk_ext_val : Rule < chunk_ext_val, Or<token, quoted_string> > {};
			struct chunk_ext : Rule < chunk_ext, ZeroOrMore<SemiColon, chunk_ext_name, ZeroOrOne<Equal, chunk_ext_val>> > {};
			struct chunk_size : Rule < chunk_size, OneOrMore<HexDigit> > {};
			struct chunked_body : Rule < chunked_body, ZeroOrMore<chunk>, last_chunk, trailer_part, CRLF > {};
			struct comment : Rule < comment, OpenParan, ZeroOrMore<Or<ctext, quoted_pair, comment>>, CloseParan > {};
			struct connection_option : Rule < connection_option, token > {};
			struct ctext : Rule < ctext, Tab, Space, Character<char, 0x21, 0x27>, Character<char, 0x2a, 0x5b>, Character<char, 0x5d, 0x7e>, obs_text > {};
			struct field_content :Rule < field_content, field_vchar, ZeroOrOne<OneOrMore<Or<Space, Tab>>, field_vchar> > {};
			struct field_name : Rule < field_name, token > {};
			struct field_value : Rule < field_value, ZeroOrMore<Or<field_content, obs_fold>> > {};
			struct field_vchar : Rule < field_vchar, Or<VCHAR, obs_text> > {};
			struct header_field : Rule < header_field, field_name, Colon, OWS, field_value, OWS > {};
			struct http_URI : Rule < http_URI, http_URI_prefix, authority, path_abempty, ZeroOrOne<Question, query>, ZeroOrOne<Hash, fragment> > {};
			struct https_URI : Rule < https_URI, https_URI_prefix, authority, path_abempty, ZeroOrOne<Question, query>, ZeroOrOne<Hash, fragment> > {};
			struct last_chunk : Rule < last_chunk, OneOrMore<_0>, ZeroOrOne<chunk_ext>, CRLF > {};
			struct message_body : Rule < message_body, ZeroOrMore<Any> > {};
			struct method : Rule < method, token > {};
			struct obs_fold : Rule < obs_fold, CRLF, OneOrMore<Or<Space, Tab>> > {};
			struct obs_text : Rule < obs_text, Character<char, 0x80, 0xff> > {};
			struct origin_form : Rule < origin_form, absolute_path, ZeroOrOne<Question, query> > {};
			struct partial_URI : Rule < partial_URI, relative_part, ZeroOrOne<Question, query> > {};
			struct protocol : Rule < protocol, protocol_name, ZeroOrOne<ForwardSlash, protocol_version> > {};
			struct protocol_name : Rule < protocol_name, token > {};
			struct protocol_version : Rule < protocol_version, token > { };
			struct pseudonym : Rule < pseudonym, token > {};
			struct qdtext : Rule < qdtext, Or<Tab, Space, Exclaimation, Character<char, 0x23, 0x5b>, Character<char, 0x5d, 0x7e>, obs_text> > {};
			struct quoted_pair : Rule < quoted_pair, BackSlash, Or<Tab, Space, VCHAR, obs_text> > {};
			struct quoted_string : Rule < quoted_string, DoubleQuote, ZeroOrMore<Or<qdtext, quoted_pair>>, DoubleQuote > {};
			struct rank : Rule < rank, Or< And< _0, ZeroOrOne<Period, Repeat<Digit, 0, 3>>>>, And<_1, ZeroOrOne<Period, Repeat<_0, 0, 3>>> > {};
			struct reason_phrase : Rule < reason_phrase, ZeroOrMore<Or<Tab, Space, VCHAR, obs_text>> > {};
			struct received_by : Rule < received_by, Or< And<uri_host, ZeroOrOne<Colon, port>>, pseudonym > > {};
			struct received_protocol : Rule < received_protocol, ZeroOrOne<protocol_name, ForwardSlash>, protocol_version > {};
			struct request_line : Rule < request_line, method, Space, request_target, Space, HTTP_version, CRLF > {};
			struct request_target : Rule < request_target, Or<origin_form, absolute_form, authority_form, asterisk_form> > {};
			struct start_line : Rule < start_line, Or<request_line, status_line> > {};
			struct status_code : Rule < status_code, Repeat<Digit, 3, 3> > {};
			struct status_line : Rule < status_line, HTTP_version, Space, status_code, Space, reason_phrase, CRLF > {};
			struct t_codings : Rule < t_codings, Or< trailers, And<transfer_coding, ZeroOrOne<t_ranking>>> > {};
			struct t_ranking : Rule < t_ranking, OWS, SemiColon, OWS, _q, Equal, rank > {};
			struct tchar : Rule < tchar, Or<Exclaimation, Hash, Dollar, Percent, Ampersand, SingleQuote, Asterisk, Plus, Hyphen, Period, Caret, UnderScore, Apostrophe, Pipe, Tilde, AlphaNumeric> > {};
			struct token : Rule < token, OneOrMore<tchar> > {};
			struct trailer_part : Rule < trailer_part, ZeroOrMore<header_field, CRLF> > {};
			struct transfer_coding : Rule < transfer_coding, chunked, compress, deflate, gzip, transfer_extension > {};
			struct transfer_extension : Rule < transfer_extension, token, ZeroOrMore<OWS, SemiColon, OWS, transfer_parameter> > {};
			struct transfer_parameter : Rule < transfer_parameter, token, BWS, Equal, BWS, Or<token, quoted_string> > {};
#pragma endregion
		}
	}
}

#endif //__RFC_7230_HPP_INCLUDED__

#if __BUILD_UNIT_TESTS__

class RFC7230_Test : public ParserTest < RFC7230_Test > {
public:

	//2.7.1.  http URI Scheme
	TEST_METHOD(HTTP_URI_Test) {
		using namespace xtd::Grammars::RFC7230;
		PassGrammar<http_URI>("http://www.yahoo.com/foobar/here/now.txt");
	}
	TEST_METHOD(HTTP_message_Test){
		using namespace xtd::Grammars::RFC7230;
		DumpAST(PassGrammar<HTTP_message>("GET /hello.txt HTTP/1.1\r\nUser-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\nHost: www.example.com\r\nAccept-Language: en, mi\r\n\r\n"));
	}
};

#endif //__BUILD_DEBUG__
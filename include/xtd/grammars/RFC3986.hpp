#ifndef __RFC_3986_HPP_INCLUDED__
#define __RFC_3986_HPP_INCLUDED__

namespace xtd {
	namespace Grammars {
		//https://tools.ietf.org/html/rfc3986
		namespace RFC3986 {
			using namespace xtd::Parser;
			using namespace xtd::Parser::MultiByte;

#pragma region("forward declerations")
			struct URI;
			struct hier_part;
			struct URI_reference;
			struct absolute_URI;
			struct relative_ref;
			struct relative_part;
			struct scheme;
			struct authority;
			struct userinfo;
			struct host;
			struct port;
			struct IP_literal;
			struct IPvFuture;
			struct IPv6address;
			struct ls32;
			struct IPv4address;
			struct dec_octet;
			struct reg_name;
			struct path;
			struct path_abempty;
			struct path_absolute;
			struct path_noscheme;
			struct path_rootless;
			struct segment;
			struct segment_nz;
			struct segment_nz_nc;
			struct pchar;
			struct query;
			struct fragment;
			struct pct_encoded;
#pragma endregion

			STRING(DoubleForwardSlash, "//");

			using gen_delims = Or < Colon, ForwardSlash, Question, Hash, OpenBracket, CloseBracket, At > ;

			using sub_delims = Or < Exclaimation, Dollar, Ampersand, SingleQuote, OpenParan, CloseParan, Asterisk, Plus, Comma, SemiColon, Equal > ;

			using unreserved = Or < AlphaNumeric, Hyphen, Period, UnderScore, Tilde > ;

			using reserved = Or < gen_delims, sub_delims > ;

			using path_empty = EndOfFile;

			struct URI : Rule < URI, scheme, Colon, hier_part, ZeroOrOne<Question, query>, ZeroOrOne<Hash, fragment> > {};

			struct hier_part : Rule < hier_part, Or< And<DoubleForwardSlash, authority, path_abempty>, path_absolute, path_rootless, path_empty > > {};

			struct URI_reference : Rule < URI_reference, Or<URI, relative_ref> > {};

			struct absolute_URI : Rule < absolute_URI, scheme, Colon, hier_part, ZeroOrOne<Question, query> > {};

			struct relative_ref : Rule < relative_ref, relative_part, ZeroOrOne<Question, query>, ZeroOrOne<Hash, fragment> > {};

			struct relative_part : Rule < relative_part, Or<And<DoubleForwardSlash, authority, path_abempty>, path_absolute, path_noscheme, path_empty> > {};

			struct scheme : Rule < scheme, Alpha, ZeroOrMore<Or<AlphaNumeric, Plus, Hyphen, Period>> > {};

			struct authority : Rule < authority, ZeroOrOne<userinfo, At>, host, ZeroOrOne<Colon, port> > {};

			struct userinfo : Rule < userinfo, ZeroOrMore< Or< unreserved, pct_encoded, sub_delims, Colon  > > > {};

			struct host : Rule < host, Or<IP_literal, IPv4address, reg_name> > {};

			struct port : Rule < port, ZeroOrMore<Digit> > {};

			struct IP_literal : Rule < IP_literal, OpenBracket, Or<IPv6address, IPvFuture>, CloseBracket > {};

			struct IPvFuture : Rule < IPvFuture, _v, OneOrMore<HexDigit>, Period, OneOrMore<Or<unreserved, sub_delims, Colon>> > {};

			using h16 = Repeat < HexDigit, 1, 4 > ;

			struct ls32 : Rule < ls32, Or<And<h16, Colon, h16>, IPv4address> > {};

			struct IPv6address : Rule < IPv6address, And<Repeat<And<h16, Colon>, 6, 6>, ls32> > {};

			struct IPv4address : Rule < IPv4address, dec_octet, Period, dec_octet, Period, dec_octet, Period, dec_octet > {};

			STRING(_25, "25");

			struct dec_octet : Rule < dec_octet, Or <
				Digit,
				Character<char, 0x31, 0x39>,
				And<_1, Digit, Digit>,
				And<_2, Character<char, 0x30, 0x34>, Digit>,
				And < _25, Character<char, 0x30, 0x35> >
				>> {};

			struct reg_name : Rule < reg_name, ZeroOrMore<Or<unreserved, pct_encoded, sub_delims>> > {	};

			struct path : Rule < path, Or<path_abempty, path_absolute, path_noscheme, path_rootless, path_empty> > {};

			struct path_abempty : Rule < path_abempty, ZeroOrMore<ForwardSlash, segment> > {};

			struct path_absolute : Rule < path_absolute, ForwardSlash, ZeroOrOne<segment_nz, ZeroOrMore<ForwardSlash, segment>> > {};

			struct path_noscheme : Rule < path_noscheme, segment_nz_nc, ZeroOrMore<ForwardSlash, segment> > {};

			struct path_rootless : Rule < path_rootless, segment_nz, ZeroOrMore<ForwardSlash, segment> > {};

			struct segment : Rule < segment, ZeroOrMore<pchar> > {};

			struct segment_nz : Rule < segment_nz, OneOrMore<pchar> > {};

			struct segment_nz_nc : Rule < segment_nz_nc, OneOrMore<Or<unreserved, pct_encoded, sub_delims, At>> > {};

			struct pchar : Rule < pchar, Or<unreserved, pct_encoded, sub_delims, Colon, At> > {};

			struct query : Rule < query, ZeroOrMore<Or<pchar, ForwardSlash, Question>> > {};

			struct fragment : Rule < fragment, ZeroOrMore<Or<pchar, ForwardSlash, Question>> > {};

			struct pct_encoded : Rule < pct_encoded, Percent, HexDigit, HexDigit > {};
		}
	}
}

#endif //__RFC_3986_HPP_INCLUDED__

#if __BUILD_UNIT_TESTS__

class RFC3986_Test : public ParserTest < RFC3986_Test > {
public:

	TEST_METHOD(version_Test) {
		using namespace xtd::Grammars::RFC3986;
	}

	//3.2.1.  User Information
	TEST_METHOD(User_Information_Test) {
		using namespace xtd::Grammars::RFC3986;
		PassGrammar<userinfo>("%20%ff%00");
		PassGrammar<userinfo>("-_~snafoo");
		PassGrammar<userinfo>("!$&'");
		PassGrammar<userinfo>("...");
	}

	//3.2.  Authority
	TEST_METHOD(Authority_Test) {
		using namespace xtd::Grammars::RFC3986;
		PassGrammar<authority>("squeegy:80");
		PassGrammar<authority>("yahoo.com");
		PassGrammar<authority>("fnord@foo:80");
	}

	//3.1.
	TEST_METHOD(Scheme_Test) {
		using namespace xtd::Grammars::RFC3986;
		PassGrammar<scheme>("fnord");
		PassGrammar<scheme>("s");
		FailGrammar<scheme>("1");
		FailGrammar<scheme>("...");
	}

	//3.  Syntax Components
	TEST_METHOD(Syntax_Components_Test) {
		using namespace xtd::Grammars::RFC3986;
		PassGrammar<URI>("foo://example.com:8042/over/there?name=ferret#nose");
		PassGrammar<URI>("urn:example:animal:ferret:nose");
		PassGrammar<URI>("foo:");
		PassGrammar<URI>("smash:");
#pragma TODO("extend hier_part tests")
		PassGrammar<hier_part>("//fnord/squeegy");
	}

	//2.3.  Unreserved Characters
	TEST_METHOD(Unreserved_Characters_Test) {
		using namespace xtd::Grammars::RFC3986;
		PassGrammar<unreserved>("a");
		PassGrammar<unreserved>("A");
		PassGrammar<unreserved>("z");
		PassGrammar<unreserved>("Z");
		PassGrammar<unreserved>("0");
		PassGrammar<unreserved>("9");
		PassGrammar<unreserved>("-");
		PassGrammar<unreserved>(".");
		PassGrammar<unreserved>("_");
		PassGrammar<unreserved>("~");
		FailGrammar<unreserved>("!");
		FailGrammar<unreserved>("$");
		FailGrammar<unreserved>("'");
		FailGrammar<unreserved>(":");
		FailGrammar<unreserved>("/");
	}

	//2.2.  Reserved Characters
	TEST_METHOD(Reserved_Characters_Test) {
		using namespace xtd::Grammars::RFC3986;
		PassGrammar<sub_delims>("!");
		PassGrammar<sub_delims>("$");
		PassGrammar<sub_delims>("'");
		PassGrammar<sub_delims>("(");
		PassGrammar<sub_delims>(")");
		PassGrammar<sub_delims>("*");
		PassGrammar<sub_delims>("+");
		PassGrammar<sub_delims>(",");
		PassGrammar<sub_delims>(";");
		PassGrammar<sub_delims>("=");
		FailGrammar<sub_delims>("%");
		FailGrammar<sub_delims>(":");
		FailGrammar<sub_delims>("/");
		FailGrammar<sub_delims>("?");
		PassGrammar<gen_delims>(":");
		PassGrammar<gen_delims>("/");
		PassGrammar<gen_delims>("?");
		PassGrammar<gen_delims>("#");
		PassGrammar<gen_delims>("[");
		PassGrammar<gen_delims>("]");
		PassGrammar<gen_delims>("@");
		FailGrammar<gen_delims>("%0");
		FailGrammar<gen_delims>("%000");
	}

	//2.1.  Percent-Encoding
	TEST_METHOD(Percent_Encoding_Test)
	{
		using namespace xtd::Grammars::RFC3986;
		PassGrammar<pct_encoded>("%ff");
		PassGrammar<pct_encoded>("%00");
		PassGrammar<pct_encoded>("%11");
		PassGrammar<pct_encoded>("%99");
		PassGrammar<pct_encoded>("%aa");
		FailGrammar<pct_encoded>("%f");
		FailGrammar<pct_encoded>("%fff");
		FailGrammar<pct_encoded>("%0");
		FailGrammar<pct_encoded>("%000");
	}
};

#endif
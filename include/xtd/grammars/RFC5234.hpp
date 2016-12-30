#ifndef __RFC_5234_HPP_INCLUDED__
#define __RFC_5234_HPP_INCLUDED__

/*
Augmented BNF for Syntax Specifications: ABNF
https://tools.ietf.org/html/rfc5234
*/

namespace xtd {
	namespace Grammars {
		namespace RFC5234 {
			using namespace xtd::Parser;
#pragma region("forward declerations")
#pragma endregion
			using CR = Character < char, 0x0d > ;
			using CTL = Or < Character<char, 0x00, 0x1f>, Character<char, 0x7f> > ;
			using DIGIT = Character < char, 0x30, 0x39 > ;
			using DQUOTE = Character < char, 0x22 > ;
			using HEXDIG = Or < DIGIT, Character<char, 'a', 'f'>, Character<char, 'A', 'F'> > ;
			using HTAB = Character < char, 0x09 > ;
			using LF = Character < char, 0x0a > ;
			using CRLF = And < CR, LF > ;
			using OCTET = Character < char, 0x00, 0xff > ;
			using SP = Character < char, 0x20 > ;
			using VCHAR = Character < char, 0x21, 0x7e > ;
			//-  WSP            =  SP / HTAB

			using WSP = Or < SP, HTAB > ;
			using LWSP = ZeroOrMore < Or<WSP, And<CRLF, WSP>> > ;
		}
	}
}

#endif //__RFC_5234_HPP_INCLUDED__
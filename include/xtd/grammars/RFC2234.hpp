#ifndef __RFC_2234_HPP_INCLUDED__
#define __RFC_2234_HPP_INCLUDED__

/*
Augmented BNF for Syntax Specifications: ABNF
https://tools.ietf.org/html/rfc2234
*/

namespace xtd {
	namespace Grammars {
		namespace RFC2234 {
			using namespace xtd::Parser;
#pragma region("forward declerations")
#pragma endregion

#pragma region("strings")
#pragma endregion

#pragma region("imports")
#pragma endregion

#pragma region("rules")
			struct ALPHA : Rule < ALPHA, Or<Character<char, 0x41, 0x5a>, Character<char, 0x61, 0x7a>> > {};
			struct DIGIT : Rule < DIGIT, Character<char, 0x30, 0x39> > {};
#pragma endregion
		}
	}
}

#endif //__RFC_2234_HPP_INCLUDED__
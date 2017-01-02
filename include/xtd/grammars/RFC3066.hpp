#ifndef __RFC_3066_HPP_INCLUDED__
#define __RFC_3066_HPP_INCLUDED__

/*
Tags for the Identification of Languages
https://tools.ietf.org/html/rfc3066

*/

namespace xtd {
	namespace Grammars {
		namespace RFC3066 {
			using namespace xtd::Parser;

#pragma region("forward declerations")
			struct Language_Tag;
			struct Primary_subtag;
			struct Subtag;
#pragma endregion

#pragma region("strings")
#pragma endregion

#pragma region("imports")
			using ALPHA = RFC2234::ALPHA;
			using DIGIT = RFC2234::DIGIT;
			using language_range = RFC2616::language_range;
#pragma endregion

#pragma region("rules")
			struct Language_Tag : Rule < Language_Tag, Primary_subtag, ZeroOrMore<MultiByte::Hyphen, Subtag> > {};
			struct Primary_subtag : Rule < Primary_subtag, Repeat<ALPHA, 1, 8> > {};
			struct Subtag : Rule < Subtag, Repeat<Or<ALPHA, DIGIT>, 1, 8> > {};
#pragma endregion
		}
	}
}

#endif //__RFC_3066_HPP_INCLUDED__
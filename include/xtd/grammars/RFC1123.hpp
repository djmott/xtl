#ifndef __RFC_1123_HPP_INCLUDED__
#define __RFC_1123_HPP_INCLUDED__

// #include "RFC822.hpp"

/*
Requirements for Internet Hosts -- Application and Support
https://tools.ietf.org/html/rfc1123
*/

namespace xtd {
	namespace Grammars {
		namespace RFC1123 {
			using namespace xtd::Parser;
			using namespace xtd::Parser::MultiByte;

			// 			struct Date : Rule < Date, RFC822::_2DIGIT, RFC822::Month, RFC822::_4DIGIT, RFC822::_4DIGIT > {};
		}
	}
}
#endif //__RFC_1123_HPP_INCLUDED__
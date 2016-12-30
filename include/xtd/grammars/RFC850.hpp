#ifndef __RFC_850_HPP_INCLUDED__
#define __RFC_850_HPP_INCLUDED__

/*
Standard for Interchange of USENET Messages
https://tools.ietf.org/html/rfc850
*/
namespace xtd {
	namespace Grammars {
		namespace RFC850 {
			// 			using namespace xtd::Parser;
			// 			using namespace xtd::Parser::MultiByte;
			// 			struct Weekday;
			// 			struct Date2;
			// 			struct Month;
			// 			struct Time;
			//
			// 			REGEX(_2DIGIT, "\\d{2}");
			// 			REGEX(_4DIGIT, "\\d{4}");
			//
			// 			STRING(Monday, "Monday");
			// 			STRING(Tuesday, "Tuesday");
			// 			STRING(Wednesday, "Wednesday");
			// 			STRING(Thursday, "Thursday");
			// 			STRING(Friday, "Friday");
			// 			STRING(Saturday, "Saturday");
			// 			STRING(Sunday, "Sunday");
			//
			// 			STRING(GMT, "GMT");
			//
			// 			STRING(Jan, "Jan");
			// 			STRING(Feb, "Feb");
			// 			STRING(Mar, "Mar");
			// 			STRING(Apr, "Apr");
			// 			STRING(May, "May");
			// 			STRING(Jun, "Jun");
			// 			STRING(Jul, "Jul");
			// 			STRING(Aug, "Aug");
			// 			STRING(Sep, "Sep");
			// 			STRING(Oct, "Oct");
			// 			STRING(Nov, "Nov");
			// 			STRING(Dec, "Dec");
			//
			// 			struct Date : Rule < Date, Weekday, Comma, Space, Date2, Space, Time, Space, GMT > {};
			//
			// 			struct Weekday : Rule < Weekday, Or<Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday> > {};
			//
			// 			struct Date2 : Rule < Date2, _2DIGIT, Hyphen, Month, Hyphen, _2DIGIT > {};
			//
			// 			struct Month : Rule < Month, Or<Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec> > {};
			//
			// 			struct Time : Rule < Time, _2DIGIT, Colon, _2DIGIT, Colon, _2DIGIT > {};
			//
		}
	}
}

#endif //__RFC_850_HPP_INCLUDED__
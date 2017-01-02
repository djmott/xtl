#ifndef __RFC_822_HPP_INCLUDED__
#define __RFC_822_HPP_INCLUDED__

//https://tools.ietf.org/html/rfc822
namespace xtd {
	namespace Grammars {
		namespace RFC822 {
			// 			using namespace xtd::Parser;
			// 			using namespace xtd::Parser::MultiByte;
			// 			struct DateTime;
			// 			struct Day;
			// 			struct Date;
			// 			struct Month;
			// 			struct Time;
			// 			struct Hour;
			// 			struct Zone;
			//
			// 			REGEX(_2DIGIT, "\\d{2}");
			// 			REGEX(_4DIGIT, "\\d{4}");
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
			// 			STRING(Mon, "Mon");
			// 			STRING(Tue, "Tue");
			// 			STRING(Wed, "Wed");
			// 			STRING(Thu, "Thu");
			// 			STRING(Fri, "Fri");
			// 			STRING(Sat, "Sat");
			// 			STRING(Sun, "Sun");
			//
			// 			STRING(UT, "UT");
			// 			STRING(GMT, "GMT");
			// 			STRING(EST, "EST");
			// 			STRING(EDT, "EDT");
			// 			STRING(CST, "CST");
			// 			STRING(CDT, "CDT");
			// 			STRING(MDT, "MDT");
			// 			STRING(MST, "MST");
			// 			STRING(PST, "PST");
			// 			STRING(PDT, "PDT");
			//
			//
			//
			// 			struct DateTime : Rule < DateTime, ZeroOrOne<Day, Comma>, Date, Time > {};
			//
			// 			struct Day : Rule < Day, Or<Mon, Tue, Wed, Thu, Fri, Sat, Sun> > {};
			//
			//
			// 			struct Date : Rule < Date, _2DIGIT, Month, _2DIGIT > {};
			//
			// 			struct Month : Rule < Month, Or<Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec> > {};
			//
			// 			struct Time : Rule < Time, Hour, Zone > {};
			//
			// 			struct Hour : Rule < Hour, _2DIGIT, Colon, _2DIGIT, ZeroOrOne<Colon, _2DIGIT> > {};
			//
			// 			struct Zone : Rule < Zone, Or<UT, GMT, EST, EDT, CST, CDT, MST, MDT, PST, PDT, Alpha, ZeroOrOne<Or<Plus, Hyphen>, _4DIGIT> > > {};
		}
	}
}
#endif //__RFC_822_HPP_INCLUDED__
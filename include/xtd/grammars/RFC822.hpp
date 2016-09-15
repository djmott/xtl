#pragma once

//https://tools.ietf.org/html/rfc822
namespace xtd {
	namespace parse {
		namespace grammar {

			namespace RFC822 {
				struct DateTime;
				struct Day;
				struct Date;
				struct Month;
				struct Time;
				struct Hour;
				struct Zone;

				REGEX(_2DIGIT,"\\d{2}");
				REGEX(_4DIGIT,"\\d{4}");

				STRING(Jan,"Jan");
				STRING(Feb,"Feb");
				STRING(Mar,"Mar");
				STRING(Apr,"Apr");
				STRING(May,"May");
				STRING(Jun,"Jun");
				STRING(Jul,"Jul");
				STRING(Aug,"Aug");
				STRING(Sep,"Sep");
				STRING(Oct,"Oct");
				STRING(Nov,"Nov");
				STRING(Dec,"Dec");

				STRING(Mon,"Mon");
				STRING(Tue,"Tue");
				STRING(Wed,"Wed");
				STRING(Thu,"Thu");
				STRING(Fri,"Fri");
				STRING(Sat,"Sat");
				STRING(Sun,"Sun");

				STRING(UT,"UT");
				STRING(GMT,"GMT");
				STRING(EST,"EST");
				STRING(EDT,"EDT");
				STRING(CST,"CST");
				STRING(CDT,"CDT");
				STRING(MDT,"MDT");
				STRING(MST,"MST");
				STRING(PST,"PST");
				STRING(PDT,"PDT");

				struct DateTime : rule<DateTime, zero_or_one_ < and_ < Day, Comma>>, Date, Time > {
					template <typename ... _ArgTs> DateTime(_ArgTs&&...oArgs) : rule (std::forward<_ArgTs>(oArgs)...) {}
				};

				struct Day : rule<Day, or_ < Mon, Tue, Wed, Thu, Fri, Sat, Sun> > {
					template <typename ... _ArgTs> Day(_ArgTs&&...oArgs) : rule (std::forward<_ArgTs>(oArgs)...) {}
				};


				struct Date : rule<Date, _2DIGIT, Month, _2DIGIT> {
					template <typename ... _ArgTs> Date(_ArgTs&&...oArgs) : rule (std::forward<_ArgTs>(oArgs)...) {}
				};

				struct Month : rule<Month, or_ < Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec> > {
					template <typename ... _ArgTs> Month(_ArgTs&&...oArgs) : rule (std::forward<_ArgTs>(oArgs)...) {}
				};

				struct Time : rule<Time, Hour, Zone> {
					template <typename ... _ArgTs> Time(_ArgTs&&...oArgs) : rule (std::forward<_ArgTs>(oArgs)...) {}
				};

				struct Hour : rule<Hour, _2DIGIT, Colon, _2DIGIT, zero_or_one_ < and_ < Colon, _2DIGIT>> > {
					template <typename ... _ArgTs> Hour(_ArgTs&&...oArgs) : rule (std::forward<_ArgTs>(oArgs)...) {}
				};

				struct Zone : rule<Zone, or_ < UT, GMT, EST, EDT, CST, CDT, MST, MDT, PST, PDT, Alpha, zero_or_one_ < and_ < or_ < Plus, Hyphen>, _4DIGIT>> >> {
					template <typename ... _ArgTs> Zone(_ArgTs&&...oArgs) : rule (std::forward<_ArgTs>(oArgs)...) {}
				};
			}
		}
	}
}


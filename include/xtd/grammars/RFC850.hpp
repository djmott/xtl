#pragma once

//https://tools.ietf.org/html/rfc850

namespace xtd {
  namespace parse {
    namespace grammar {
      namespace RFC850 {
        struct Weekday;
        struct Date2;
        struct Month;
        struct Time;

        REGEX(_2DIGIT,"\d{2}");
        REGEX(_4DIGIT,"\d{4}");

        STRING(Monday,"Monday");
        STRING(Tuesday,"Tuesday");
        STRING(Wednesday,"Wednesday");
        STRING(Thursday,"Thursday");
        STRING(Friday,"Friday");
        STRING(Saturday,"Saturday");
        STRING(Sunday,"Sunday");

        STRING(GMT,"GMT");

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

        struct Date : rule<Date, and_<Weekday, Comma, Space, Date2, Space, Time, Space, GMT>> {
          template <typename ... _ArgTs> Date(_ArgTs&&...oArgs) : rule (std::forward<_ArgTs>(oArgs)...) {}
        };

        struct Weekday : rule<Weekday, or_<Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday> > {
          template <typename ... _ArgTs> Weekday(_ArgTs&&...oArgs) : rule (std::forward<_ArgTs>(oArgs)...) {}
        };

        struct Date2 : rule<Date2, and_<_2DIGIT, Hyphen, Month, Hyphen, _2DIGIT>> {
          template <typename ... _ArgTs> Date2(_ArgTs&&...oArgs) : rule (std::forward<_ArgTs>(oArgs)...) {}
        };

        struct Month : rule<Month, or_<Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec> > {
          template <typename ... _ArgTs> Month(_ArgTs&&...oArgs) : rule (std::forward<_ArgTs>(oArgs)...) {}
        };

        struct Time : rule<Time, and_<_2DIGIT, Colon, _2DIGIT, Colon, _2DIGIT>> {
          template <typename ... _ArgTs> Time(_ArgTs&&...oArgs) : rule (std::forward<_ArgTs>(oArgs)...) {}
        };
      }
    }
  }
}

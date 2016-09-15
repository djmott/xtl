#pragma once
//https://tools.ietf.org/html/rfc1123

namespace xtd {
  namespace parse {
    namespace grammar {
      namespace RFC1123 {
        struct Date : rule<Date, and_ < RFC822::_2DIGIT, RFC822::Month, RFC822::_4DIGIT, RFC822::_4DIGIT >> {
          template <typename ... _ArgTs> Date(_ArgTs&&...oArgs) : rule (std::forward<_ArgTs>(oArgs)...) {}
        };
      }
    }
  }
}

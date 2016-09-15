#pragma once
/*
HTTP/1.1
*/


//https://tools.ietf.org/html/rfc2616

namespace xtd {
  namespace parse {
    namespace grammar {
      namespace RFC2616 {
        struct RequestURI;
        struct Host;
        struct AbsPath;
        struct Port;
        struct Query;
        struct Comment;

        REGEX(_2DIGIT,"\d{2}");
        REGEX(_4DIGIT,"\d{4}");
        STRING(HTTP,"HTTP");
        STRING(http,"http");
        STRING(CRLF,"\r\n");
        CHARACTER_(Space, ' ');
        CHARACTER_(Tab, '\t');

        using Wkday = RFC822::Day;
        using Month = RFC822::Month;

        CHARACTERS_(Char, 0, 127);
        CHARACTERS_(CTL1, 0, 31);
        using CTL = or_ <CTL1, Delete>;

        using LWS = and_ <CRLF, one_or_more_<and_ < Space, Tab>>>;
        using Text = and_ <Not<CTL>, Between<0, 255>>;

        using Seperator = or_<OpenParan, CloseParan, LessThan, GreaterThan, At, Comma, SemiColon, Colon, BackSlash, Quote, ForwardSlash, OpenBracket, CloseBracket, Question, Equal, OpenBrace, CloseBrace, Space, Tab>;

        using Token = and_ <Not<CTL>, Not<Seperator>, Char>;

        using CText = and_ <Not<OpenParan>, Not<CloseParan>, Text>;

        using QDText = and_ <Not<Quote>, Text>;
        using QuotedPair = and_<BackSlash, Char>;
        using QuotedString = and_ <Quote, zero_or_more_<or_ < QDText, QuotedPair>>, Quote>;

        struct Comment : rule<Comment, OpenParan, zero_or_more_ < or_ < CText, QuotedPair, Comment>>, CloseParan > {
          template <typename ... _ArgTs> Comment(_ArgTs&&...oArgs) : rule (std::forward<_ArgTs>(oArgs)...) {}
        };
  
  
        using HTTPVersion = and_<HTTP, ForwardSlash, Numeric, Period, Numeric>;
  
        using Date3 = and_ <Month, Space, or_<_2DIGIT, and_ < Space, Digit>>>;
  
        using ASCTimeDate = and_<Wkday, Space, Date3, Space, RFC850::Time, Space, _4DIGIT>;
  
        using Date = or_<RFC1123::Date, RFC850::Date, ASCTimeDate>;
  
        using HTTPURL = and_ <http, Colon, ForwardSlash, ForwardSlash, Host, zero_or_one_< and_ < Colon, Port>>, AbsPath, zero_or_one_ <and_<Question, Query>>>;
  

        struct RequestURI;
        struct Host;
        struct AbsPath;
        struct Port;
        struct Query;
      }
    }
  }
}

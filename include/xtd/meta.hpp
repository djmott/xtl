/** @file
  template meta-programming utilities
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/


#pragma once

namespace xtd{

  ///Determine if a type is specified in a list
  template <typename, typename...> struct is_a;
  template <typename _Ty> struct is_a<_Ty> : std::false_type {};
  template <typename _Ty, typename ... _TailT> struct is_a<_Ty, _Ty, _TailT...> : std::true_type {};
  template <typename _Ty, typename _HeadT, typename ... _TailT> struct is_a<_Ty, _HeadT, _TailT...> : is_a<_Ty, _TailT...> {};


  ///Gets the type of a parameter in a method declaration
  template <int _ParamNum, typename _Ty> struct get_parameter;
  template <typename _ReturnT, typename _HeadT, typename ... _TailT> struct get_parameter<0, _ReturnT(_HeadT, _TailT...)>{
    using type = _HeadT;
  };
  template <int _ParamNum, typename _ReturnT, typename _HeadT, typename ... _TailT> struct get_parameter<_ParamNum, _ReturnT(_HeadT, _TailT...)>{
    using type = typename get_parameter<_ParamNum-1, _ReturnT(_TailT...)>::type;
  };

  /**
   \struct nibble_hex_char
   meta-function to convert a static nibble to an ascii hex value
   \tparam char nibble to convert
   */
  template <typename _ChT, _ChT> struct nibble_hex_char;
#if (!DOXY_INVOKED)
  template <> struct nibble_hex_char <char, 0 > { static const char value = '0'; };
  template <> struct nibble_hex_char <char, 1 > { static const char value = '1'; };
  template <> struct nibble_hex_char <char, 2 > { static const char value = '2'; };
  template <> struct nibble_hex_char <char, 3 > { static const char value = '3'; };
  template <> struct nibble_hex_char <char, 4 > { static const char value = '4'; };
  template <> struct nibble_hex_char <char, 5 > { static const char value = '5'; };
  template <> struct nibble_hex_char <char, 6 > { static const char value = '6'; };
  template <> struct nibble_hex_char <char, 7 > { static const char value = '7'; };
  template <> struct nibble_hex_char <char, 8 > { static const char value = '8'; };
  template <> struct nibble_hex_char <char, 9 > { static const char value = '9'; };
  template <> struct nibble_hex_char <char, 10 > { static const char value = 'a'; };
  template <> struct nibble_hex_char <char, 11 > { static const char value = 'b'; };
  template <> struct nibble_hex_char <char, 12 > { static const char value = 'c'; };
  template <> struct nibble_hex_char <char, 13 > { static const char value = 'd'; };
  template <> struct nibble_hex_char <char, 14 > { static const char value = 'e'; };
  template <> struct nibble_hex_char <char, 15 > { static const char value = 'f'; };
#endif

  template <typename _ChT, unsigned char _val> struct ByteToChar {
    typedef nibble_hex_char<_ChT, _val & 0x0f> LoNibble;
    typedef nibble_hex_char<_ChT, ((_val & 0xf0) >> 4)> HiNibble;
  };

  template <typename _ChT, unsigned short _val> struct ShortToChar {
    typedef ByteToChar<_ChT, _val & 0x00ff> LoByte;
    typedef ByteToChar<_ChT,((_val & 0xff00) >> 8)> HiByte;
  };

  template <typename _ChT, unsigned int _val> struct IntToChar {
    typedef ShortToChar<_ChT, _val & 0x0000ffff> LoWord;
    typedef ShortToChar<_ChT, ((_val & 0xffff0000) >> 16)> HiWord;
  };

  template <typename _ChT, unsigned long long _val> struct LongLongToChar {
    typedef IntToChar<_ChT, _val & 0x00000000ffffffff> LoWord;
    typedef IntToChar<_ChT, ((_val & 0xffffffff00000000) >> 32)> HiWord;
  };


  /**
    \struct nibble_hex_char
    meta-function to convert a static upper case ascii character to lower case
    \tparam _ChT character type
    \tparam _val value of type _ChT to convert
   */
  template <typename _ChT, _ChT _val> struct lower_case { static const _ChT value = _val; };
#if (!DOXY_INVOKED)
  template <> struct lower_case < char, 'A' > { static const char value = 'a'; };
  template <> struct lower_case < char, 'B' > { static const char value = 'b'; };
  template <> struct lower_case < char, 'C' > { static const char value = 'c'; };
  template <> struct lower_case < char, 'D' > { static const char value = 'd'; };
  template <> struct lower_case < char, 'E' > { static const char value = 'e'; };
  template <> struct lower_case < char, 'F' > { static const char value = 'f'; };
  template <> struct lower_case < char, 'G' > { static const char value = 'g'; };
  template <> struct lower_case < char, 'H' > { static const char value = 'h'; };
  template <> struct lower_case < char, 'I' > { static const char value = 'i'; };
  template <> struct lower_case < char, 'J' > { static const char value = 'j'; };
  template <> struct lower_case < char, 'K' > { static const char value = 'k'; };
  template <> struct lower_case < char, 'L' > { static const char value = 'l'; };
  template <> struct lower_case < char, 'M' > { static const char value = 'm'; };
  template <> struct lower_case < char, 'N' > { static const char value = 'n'; };
  template <> struct lower_case < char, 'O' > { static const char value = 'o'; };
  template <> struct lower_case < char, 'P' > { static const char value = 'p'; };
  template <> struct lower_case < char, 'Q' > { static const char value = 'q'; };
  template <> struct lower_case < char, 'R' > { static const char value = 'r'; };
  template <> struct lower_case < char, 'S' > { static const char value = 's'; };
  template <> struct lower_case < char, 'T' > { static const char value = 't'; };
  template <> struct lower_case < char, 'U' > { static const char value = 'u'; };
  template <> struct lower_case < char, 'V' > { static const char value = 'v'; };
  template <> struct lower_case < char, 'W' > { static const char value = 'w'; };
  template <> struct lower_case < char, 'X' > { static const char value = 'x'; };
  template <> struct lower_case < char, 'Y' > { static const char value = 'y'; };
  template <> struct lower_case < char, 'Z' > { static const char value = 'z'; };

  template <> struct lower_case < wchar_t, L'A' > { static const wchar_t value = L'a'; };
  template <> struct lower_case < wchar_t, L'B' > { static const wchar_t value = L'b'; };
  template <> struct lower_case < wchar_t, L'C' > { static const wchar_t value = L'c'; };
  template <> struct lower_case < wchar_t, L'D' > { static const wchar_t value = L'd'; };
  template <> struct lower_case < wchar_t, L'E' > { static const wchar_t value = L'e'; };
  template <> struct lower_case < wchar_t, L'F' > { static const wchar_t value = L'f'; };
  template <> struct lower_case < wchar_t, L'G' > { static const wchar_t value = L'g'; };
  template <> struct lower_case < wchar_t, L'H' > { static const wchar_t value = L'h'; };
  template <> struct lower_case < wchar_t, L'I' > { static const wchar_t value = L'i'; };
  template <> struct lower_case < wchar_t, L'J' > { static const wchar_t value = L'j'; };
  template <> struct lower_case < wchar_t, L'K' > { static const wchar_t value = L'k'; };
  template <> struct lower_case < wchar_t, L'L' > { static const wchar_t value = L'l'; };
  template <> struct lower_case < wchar_t, L'M' > { static const wchar_t value = L'm'; };
  template <> struct lower_case < wchar_t, L'N' > { static const wchar_t value = L'n'; };
  template <> struct lower_case < wchar_t, L'O' > { static const wchar_t value = L'o'; };
  template <> struct lower_case < wchar_t, L'P' > { static const wchar_t value = L'p'; };
  template <> struct lower_case < wchar_t, L'Q' > { static const wchar_t value = L'q'; };
  template <> struct lower_case < wchar_t, L'R' > { static const wchar_t value = L'r'; };
  template <> struct lower_case < wchar_t, L'S' > { static const wchar_t value = L's'; };
  template <> struct lower_case < wchar_t, L'T' > { static const wchar_t value = L't'; };
  template <> struct lower_case < wchar_t, L'U' > { static const wchar_t value = L'u'; };
  template <> struct lower_case < wchar_t, L'V' > { static const wchar_t value = L'v'; };
  template <> struct lower_case < wchar_t, L'W' > { static const wchar_t value = L'w'; };
  template <> struct lower_case < wchar_t, L'X' > { static const wchar_t value = L'x'; };
  template <> struct lower_case < wchar_t, L'Y' > { static const wchar_t value = L'y'; };
  template <> struct lower_case < wchar_t, L'Z' > { static const wchar_t value = L'z'; };
#endif
}

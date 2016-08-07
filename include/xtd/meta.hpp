/** @file
 * template meta-programming utilities
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#pragma once
#include <xtd/xtd.hpp>

/// @def RAII general purpose Resource Acquisition Is Initialization idiom to perform exception safe cleanup by executing arbitrary code when a scope exits
#define RAII(...) xtd::_::_RAII UNIQUE_IDENTIFIER(raii_object)([&](){ __VA_ARGS__ ; });

namespace xtd{
  namespace _{

    /** helper class for general purpose RAII
     * rarely created directly. instead use the RAII macro
     */
    class _RAII{
    public:
      template <typename _Ty>
      _RAII(_Ty newval) : _fn(newval){}
      ~_RAII(){ _fn(); }
      std::function<void()> _fn;
    };

  }

  template <typename _Ty> constexpr uint32_t hidword(_Ty value){
    static_assert(sizeof(_Ty) > 4, "parameter is <= 32 bits wide");
    return ((uint32_t)(((value) >> 32) & 0xffffffff));
  }
  template <typename _Ty> constexpr uint32_t lodword(_Ty value){
    static_assert(sizeof(_Ty) > 4, "parameter is <= 32 bits wide");
    return ((uint32_t)((value) & 0xffffffff));
  }

  /** meta-function to get the intrinsic of a specified size
  @tparam _Size size
  @return intrinsic of specified size
  */
  template <int _Size> struct intrinsic_of_size;
#if (!DOXY_INVOKED)
  template <> struct intrinsic_of_size<1>{ using type = uint8_t; };
  template <> struct intrinsic_of_size<2>{ using type = uint16_t; };
  template <> struct intrinsic_of_size<4>{ using type = uint32_t; };
  template <> struct intrinsic_of_size<8>{ using type = uint64_t; };


#endif

  /** meta-function to get the processor intrinsic storage for a type. should work with 8, 16, 32 and 64 bit processors
  @tparam _Ty type
  @return intrinsic storage
  */
  template <typename _Ty> struct processor_intrinsic{
    /// processor intrinsic of pointer type
    using type = typename intrinsic_of_size<sizeof(_Ty)>::type;
  };
#if (!DOXY_INVOKED)
  template <typename _Ty> struct processor_intrinsic<_Ty*>{
    using type = typename intrinsic_of_size<sizeof(_Ty*)>::type;
  };
#endif
  /** casts a pointer to the processor intrinsic storage type
  * @param src
  * @return
  */
  template <typename _Ty> constexpr typename processor_intrinsic<_Ty>::type intrinsic_cast(_Ty src){
    return src;
  } 
  template <typename _Ty> constexpr typename processor_intrinsic<_Ty*>::type intrinsic_cast(const _Ty * src){
    return reinterpret_cast<typename processor_intrinsic<_Ty>::type>(src);
  }

  /// Determine if a type is specified in a list
  template <typename, typename...> struct is_a;
  template <typename _Ty> struct is_a<_Ty> : std::false_type {};
  template <typename _Ty, typename ... _TailT> struct is_a<_Ty, _Ty, _TailT...> : std::true_type{ using type = _Ty; };
  template <typename _Ty, typename _HeadT, typename ... _TailT> struct is_a<_Ty, _HeadT, _TailT...> : is_a<_Ty, _TailT...>{ using type = _Ty; };


  /// Gets the type of a parameter in a method declaration
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
  template <> struct nibble_hex_char <char, 0 > { static constexpr char value = '0'; };
  template <> struct nibble_hex_char <char, 1 > { static constexpr char value = '1'; };
  template <> struct nibble_hex_char <char, 2 > { static constexpr char value = '2'; };
  template <> struct nibble_hex_char <char, 3 > { static constexpr char value = '3'; };
  template <> struct nibble_hex_char <char, 4 > { static constexpr char value = '4'; };
  template <> struct nibble_hex_char <char, 5 > { static constexpr char value = '5'; };
  template <> struct nibble_hex_char <char, 6 > { static constexpr char value = '6'; };
  template <> struct nibble_hex_char <char, 7 > { static constexpr char value = '7'; };
  template <> struct nibble_hex_char <char, 8 > { static constexpr char value = '8'; };
  template <> struct nibble_hex_char <char, 9 > { static constexpr char value = '9'; };
  template <> struct nibble_hex_char <char, 10 > { static constexpr char value = 'a'; };
  template <> struct nibble_hex_char <char, 11 > { static constexpr char value = 'b'; };
  template <> struct nibble_hex_char <char, 12 > { static constexpr char value = 'c'; };
  template <> struct nibble_hex_char <char, 13 > { static constexpr char value = 'd'; };
  template <> struct nibble_hex_char <char, 14 > { static constexpr char value = 'e'; };
  template <> struct nibble_hex_char <char, 15 > { static constexpr char value = 'f'; };
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
  template <> struct lower_case < char, 'A' > { static constexpr char value = 'a'; };
  template <> struct lower_case < char, 'B' > { static constexpr char value = 'b'; };
  template <> struct lower_case < char, 'C' > { static constexpr char value = 'c'; };
  template <> struct lower_case < char, 'D' > { static constexpr char value = 'd'; };
  template <> struct lower_case < char, 'E' > { static constexpr char value = 'e'; };
  template <> struct lower_case < char, 'F' > { static constexpr char value = 'f'; };
  template <> struct lower_case < char, 'G' > { static constexpr char value = 'g'; };
  template <> struct lower_case < char, 'H' > { static constexpr char value = 'h'; };
  template <> struct lower_case < char, 'I' > { static constexpr char value = 'i'; };
  template <> struct lower_case < char, 'J' > { static constexpr char value = 'j'; };
  template <> struct lower_case < char, 'K' > { static constexpr char value = 'k'; };
  template <> struct lower_case < char, 'L' > { static constexpr char value = 'l'; };
  template <> struct lower_case < char, 'M' > { static constexpr char value = 'm'; };
  template <> struct lower_case < char, 'N' > { static constexpr char value = 'n'; };
  template <> struct lower_case < char, 'O' > { static constexpr char value = 'o'; };
  template <> struct lower_case < char, 'P' > { static constexpr char value = 'p'; };
  template <> struct lower_case < char, 'Q' > { static constexpr char value = 'q'; };
  template <> struct lower_case < char, 'R' > { static constexpr char value = 'r'; };
  template <> struct lower_case < char, 'S' > { static constexpr char value = 's'; };
  template <> struct lower_case < char, 'T' > { static constexpr char value = 't'; };
  template <> struct lower_case < char, 'U' > { static constexpr char value = 'u'; };
  template <> struct lower_case < char, 'V' > { static constexpr char value = 'v'; };
  template <> struct lower_case < char, 'W' > { static constexpr char value = 'w'; };
  template <> struct lower_case < char, 'X' > { static constexpr char value = 'x'; };
  template <> struct lower_case < char, 'Y' > { static constexpr char value = 'y'; };
  template <> struct lower_case < char, 'Z' > { static constexpr char value = 'z'; };

  template <> struct lower_case < wchar_t, L'A' > { static constexpr wchar_t value = L'a'; };
  template <> struct lower_case < wchar_t, L'B' > { static constexpr wchar_t value = L'b'; };
  template <> struct lower_case < wchar_t, L'C' > { static constexpr wchar_t value = L'c'; };
  template <> struct lower_case < wchar_t, L'D' > { static constexpr wchar_t value = L'd'; };
  template <> struct lower_case < wchar_t, L'E' > { static constexpr wchar_t value = L'e'; };
  template <> struct lower_case < wchar_t, L'F' > { static constexpr wchar_t value = L'f'; };
  template <> struct lower_case < wchar_t, L'G' > { static constexpr wchar_t value = L'g'; };
  template <> struct lower_case < wchar_t, L'H' > { static constexpr wchar_t value = L'h'; };
  template <> struct lower_case < wchar_t, L'I' > { static constexpr wchar_t value = L'i'; };
  template <> struct lower_case < wchar_t, L'J' > { static constexpr wchar_t value = L'j'; };
  template <> struct lower_case < wchar_t, L'K' > { static constexpr wchar_t value = L'k'; };
  template <> struct lower_case < wchar_t, L'L' > { static constexpr wchar_t value = L'l'; };
  template <> struct lower_case < wchar_t, L'M' > { static constexpr wchar_t value = L'm'; };
  template <> struct lower_case < wchar_t, L'N' > { static constexpr wchar_t value = L'n'; };
  template <> struct lower_case < wchar_t, L'O' > { static constexpr wchar_t value = L'o'; };
  template <> struct lower_case < wchar_t, L'P' > { static constexpr wchar_t value = L'p'; };
  template <> struct lower_case < wchar_t, L'Q' > { static constexpr wchar_t value = L'q'; };
  template <> struct lower_case < wchar_t, L'R' > { static constexpr wchar_t value = L'r'; };
  template <> struct lower_case < wchar_t, L'S' > { static constexpr wchar_t value = L's'; };
  template <> struct lower_case < wchar_t, L'T' > { static constexpr wchar_t value = L't'; };
  template <> struct lower_case < wchar_t, L'U' > { static constexpr wchar_t value = L'u'; };
  template <> struct lower_case < wchar_t, L'V' > { static constexpr wchar_t value = L'v'; };
  template <> struct lower_case < wchar_t, L'W' > { static constexpr wchar_t value = L'w'; };
  template <> struct lower_case < wchar_t, L'X' > { static constexpr wchar_t value = L'x'; };
  template <> struct lower_case < wchar_t, L'Y' > { static constexpr wchar_t value = L'y'; };
  template <> struct lower_case < wchar_t, L'Z' > { static constexpr wchar_t value = L'z'; };
#endif
}

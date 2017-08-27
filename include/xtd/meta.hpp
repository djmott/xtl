/** @file
 * template meta-programming utilities
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#pragma once
#include <xtd/xtd.hpp>

#include <functional>

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

  template <typename...> using void_t = void;

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

  /// gets the last element of a parameter pack
  namespace _{
    template <size_t, typename ...> struct last_t;
    template <size_t _index, typename _HeadT, typename ... _TailT> struct last_t<_index, _HeadT, _TailT...>{
      using type = typename last_t<_index - 1, _TailT...>::type;
    };
    template <typename _HeadT> struct last_t<1, _HeadT>{
      using type = _HeadT;
    };
  }
  template <typename ... _Tys> struct last{
    using type = typename _::last_t<sizeof...(_Tys), _Tys...>::type;
  };

  
  /// chains together multiple methods in a single task

  template <typename ...> struct task;
  template <> struct task<>{
    template <typename _Ty> _Ty&& operator()(_Ty&& src){ return std::move(src); }
  };

  template <typename _HeadT, typename ... _TailT> struct task<_HeadT, _TailT...>{
    using final_task = typename last<_HeadT, _TailT...>::type;
    using return_type = decltype( typename std::decay<final_task>::type() );

    template <typename _ParamT>
    return_type operator()(_ParamT oParam) const{
      _HeadT oHead;
      task<_TailT...> oTail;
      return oTail(oHead(oParam));
    }

  };



  /// Determine if a type is specified in a list
  template <typename, typename...> struct is_a;
  template <typename _Ty> struct is_a<_Ty> : std::false_type {};
  template <typename _Ty, typename ... _TailT> struct is_a<_Ty, _Ty, _TailT...> : std::true_type{ using type = _Ty; };
  template <typename _Ty, typename _HeadT, typename ... _TailT> struct is_a<_Ty, _HeadT, _TailT...> : is_a<_Ty, _TailT...>{ using type = _Ty; };


  


  /// Gets the type of a parameter in a method declaration
  namespace _ {
    template <uint8_t _ParamNum, typename _Ty> struct _get_parameter;
    template <typename _ReturnT, typename _HeadT, typename ... _TailT> struct _get_parameter<0, _ReturnT(_HeadT, _TailT...)> {
      using type = _HeadT;
    };
    template <uint8_t _ParamNum, typename _ReturnT, typename _HeadT, typename ... _TailT> struct _get_parameter<_ParamNum, _ReturnT(_HeadT, _TailT...)> {
      using type = typename _::template _get_parameter<_ParamNum - 1, _ReturnT(_TailT...)>::type;
    };
  }
  
  template <uint8_t _ParamNum, typename _Ty> struct get_parameter;
  template <uint8_t _ParamNum, typename _ReturnT, typename ... _ArgTs> struct get_parameter<_ParamNum, _ReturnT(_ArgTs...)> {
    static_assert(sizeof...(_ArgTs) >= _ParamNum, "Specified parameter index exceeds number of parameters in function");
    using type = typename _::template _get_parameter<_ParamNum, _ReturnT(_ArgTs...)>::type;
  };

  //test for t::type member
  template <typename, typename = void> struct has_type_member : std::false_type{};
  template <typename _Ty> struct has_type_member<_Ty, void_t<typename _Ty::type>> : std::true_type{};

  //test for copy assignment
  template <typename _Ty> using copy_assignment_t = decltype(std::declval<_Ty&>() = std::declval<_Ty const&>());
  template <typename, typename = void> struct is_copy_assignable : std::false_type{};
  template <typename _Ty> struct is_copy_assignable<_Ty, void_t<copy_assignment_t<_Ty>>> : std::is_same<copy_assignment_t<_Ty>, _Ty&>{};

  //test for move assignment
  template <typename _Ty> using move_assignment_t = decltype(std::declval<_Ty&>() = std::declval<_Ty&&>());
  template <typename, typename = void> struct is_move_assignable : std::false_type{};
  template <typename _Ty> struct is_move_assignable<_Ty, void_t<move_assignment_t<_Ty>>> : std::is_same<move_assignment_t<_Ty>, _Ty&&>{};

  //test for invocation operator
  template <typename _Ty> using invokation_operator_t = decltype(std::declval<_Ty&>()());
  template <typename, typename = void> struct is_invokable : std::false_type{};
  template <typename _Ty> struct is_invokable<_Ty, void_t<invokation_operator_t<_Ty>>> : std::true_type{};

  /**
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

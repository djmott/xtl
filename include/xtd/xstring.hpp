/** @file
  specializations of std::basic_string for advanced and common string handling
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/
#pragma once

#include <xtd/xtd.hpp>

#if (XTD_OS_WINDOWS & XTD_OS)
#include <windows.h>
#endif

#include <algorithm>
#include <string>
#include <type_traits>
#include <cctype>
#include <cwctype>
#include <vector>
#include <functional>

namespace xtd {


  template <typename _ch_t> struct xstring;

  using cstring = xstring<char>;
  using ustring = xstring<wchar_t>;
  using tstring = xstring<tchar>;
  using utf8 = xstring<char>;
  using utf16 = xstring<wchar_t>;


  template <typename _ch_t> struct xstring : std::basic_string<_ch_t>{
    using _super_t = std::basic_string<_ch_t>;
    template <typename ... _arg_ts> xstring(_arg_ts&&...args) : _super_t(std::forward<_arg_ts>(args)...){}

    //template <typename _ty> void from(typename std::add_const<typename std::add_lvalue_reference<_ty>::type>::type val);
    template <typename _ty> xstring& from(const _ty& val);

    template <typename _ty, typename ... _arg_ts>
    static xstring Format(const _ty& val, _arg_ts&&...args) {
      return xstring().from(val) + Format(std::forward<_arg_ts>(args)...);
    }

    xstring& to_lower();

    xstring& to_upper();

    ///Trim leading whitespace
    xstring& ltrim(); 

    ///Trim trailing whitespace
    xstring& rtrim();

    //Trim leading and trailing whitespace
    xstring& trim() {
      ltrim();
      rtrim();
      return *this;
    }

    //replaces all occurrences of the characters in the oItems list with a specified character
    xstring& replace(std::initializer_list<_ch_t> oItems, _ch_t chReplace) {
      std::transform(_super_t::begin(), _super_t::end(), _super_t::begin(), [&](const _ch_t& ch) {
        auto i = std::find_if(oItems.begin(), oItems.end(), [ch](_ch_t item) { return ch == item; });
        return (std::end(oItems) == i ? ch : chReplace);
      });
      return *this;
    }


    ///splits the string by the specified delmiters into constituent elements
    std::vector<xstring<_ch_t>> split(const std::initializer_list<_ch_t>& delimiters, bool trimEmpty = false) const {
      using container_t = std::vector<xstring<_ch_t>>;
      container_t oRet;
      using _my_t = xstring<_ch_t>;
      typename _super_t::size_type pos;
      typename _super_t::size_type lastPos = 0;

      using value_type = typename container_t::value_type;

      forever{
        pos = _super_t::find_first_of(delimiters, lastPos);
        if (pos == _my_t::npos) {
          pos = _super_t::length();
          if (pos != lastPos || !trimEmpty) {
            oRet.push_back(value_type(_super_t::data() + lastPos, (pos - lastPos)));
          }
          break;
        }
        else if (pos != lastPos || !trimEmpty) {
          oRet.push_back(value_type(_super_t::data() + lastPos , (pos - lastPos)));
        }

        lastPos = pos + 1;
      }
      return oRet;
    }

    ///splits the string by the specified string into constituent elements
    std::vector<xstring<_ch_t>> split(const xstring<_ch_t>& delim, bool trimEmpty = false) const {
      using container_t = std::vector<xstring<_ch_t>>;
      container_t oRet;
      using _my_t = xstring<_ch_t>;
      typename _super_t::size_type pos;
      typename _super_t::size_type lastPos = 0;

      using value_type = typename container_t::value_type;

      forever{
        pos = find(delim, lastPos);
        if (pos == _my_t::npos) {
          pos = _super_t::length();
          if (pos != lastPos || !trimEmpty) {
            oRet.push_back(value_type(_super_t::data() + lastPos, (pos - lastPos)));
          }
          break;
        }
        else if (pos != lastPos || !trimEmpty) {
          oRet.push_back(value_type(_super_t::data() + lastPos , (pos - lastPos)));
        }

        lastPos = pos + 1;
      }
      return oRet;
    }

    ///splits the string by the user supplied unary function

    std::vector<xstring<_ch_t>> split(const std::function<bool(_ch_t)>& VisitorFN) const{
      using container_t = std::vector<xstring<_ch_t>>;
      using value_type = typename container_t::value_type;
      container_t oRet;
      using _my_t = xstring<_ch_t>;
      size_t iLast = 0;
      for (size_t iCurr = iLast; iCurr < _my_t::size(); ){
        if (!VisitorFN((*this)[iCurr])){
          ++iCurr;
          continue;
        }
        auto iLen = iCurr - iLast;
        if (!iLen) ++iLen;
        oRet.push_back(value_type(_super_t::data() + iLast, iLen));
        if (iCurr != iLast){
          oRet.push_back(value_type(_super_t::data() + iCurr, 1));
        }
        iLast = ++iCurr;
      }
      if (iLast < _my_t::size()){
        oRet.push_back(value_type(_super_t::data() + iLast, _my_t::size() - iLast));
      }
      return oRet;
    }

  };

  template <> xstring<char>& xstring<char>::to_lower() {
    std::transform(begin(), end(), begin(), [](value_type&ch) { return ch = std::tolower(ch); });
    return *this;
  }

  template <> xstring<wchar_t>& xstring<wchar_t>::to_lower() {
    std::transform(begin(), end(), begin(), [](value_type&ch) { return ch = std::towlower(ch); });
    return *this;
  }

  template <> xstring<char>& xstring<char>::to_upper() {
    std::transform(begin(), end(), begin(), [](value_type&ch) { return ch = std::toupper(ch); });
    return *this;
  }

  template <> xstring<wchar_t>& xstring<wchar_t>::to_upper() {
    std::transform(begin(), end(), begin(), [](value_type&ch) { return ch = std::towupper(ch); });
    return *this;
  }
  
  template <> xstring<char>& xstring<char>::ltrim() {
    erase(begin(), std::find_if(begin(), end(), [](value_type ch) { return !std::isspace(ch); }));
    return *this;
  }

  template <> xstring<wchar_t>& xstring<wchar_t>::ltrim() {
    erase(begin(), std::find_if(begin(), end(), [](value_type ch) { return !std::iswspace(ch); }));
    return *this;
  }

  template <> xstring<char>& xstring<char>::rtrim() {
    erase(std::find_if(rbegin(), rend(), [](value_type ch) {return !std::isspace(ch); }).base(), end());
    return *this;
  }

  template <> xstring<wchar_t>& xstring<wchar_t>::rtrim() {
    erase(std::find_if(rbegin(), rend(), [](value_type ch) {return !std::iswspace(ch); }).base(), end());
    return *this;
  }

  template <> template <>
  xstring<char>& xstring<char>::from<xstring<char>>(const xstring<char>& val) {
    return (*this = val);
  }

  template <> template <>
  xstring<wchar_t>& xstring<wchar_t>::from<xstring<wchar_t>>(const xstring<wchar_t>& val) {
    return (*this = val);
  }

  template <> template <>
  xstring<char>& xstring<char>::from<char *>( char * const& val) {
    return (*this = val);
  }

  template <> template <>
  xstring<wchar_t>& xstring<wchar_t>::from<wchar_t*>( wchar_t*const& val) {
    return (*this = val);
  }

#if (XTD_OS_WINDOWS & XTD_OS)

  template <> template <> cstring& cstring::from(const ustring& src) {
    int iLen;
    auto iSize = static_cast<int>(src.size());
    if (!(iLen = ::WideCharToMultiByte(CP_UTF8, 0, src.c_str(), iSize, nullptr, 0, "?", nullptr))) throw std::exception("wide string conversion failure");
    resize(iLen, 0);
    ::WideCharToMultiByte(CP_UTF8, 0, src.c_str(), iSize, &at(0), iLen, "?", nullptr);
    return *this;
  }

  template<> template<> ustring& ustring::from(const cstring& src) {
    int iLen;
    auto iSize = static_cast<int>(src.size());
    if (!(iLen = ::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), iSize, nullptr, 0))) throw std::exception("multi-byte string conversion failure");
    resize(iLen, 0);
    ::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), iSize, &at(0), iLen);
    return *this;
  }

#endif
}

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
      std::transform(begin(), end(), begin(), [&](const _ch_t& ch) {
        auto i = std::find_if(oItems.begin(), oItems.end(), [ch](_ch_t item) { return ch == item; });
        return (std::end(oItems) == i ? ch : chReplace);
      });
      return *this;
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

}



#if 0
#pragma once

#include <xtd/xtd.hpp>

#if (XTD_OS_WINDOWS & XTD_OS)
#include <windows.h>
#endif

#include <string>
#include <vector>
#include <stdexcept>
#include <functional>
#include <algorithm>

#if (XTD_HAS_CODECVT)
  #include <codecvt>
#elif XTD_HAS_EXP_CODECVT
  #include <experimental/codecvt>

#elif (XTD_HAS_ICONV)
  #include <iconv.h>
#endif
#include <cwctype>

#include <locale>

#include <string.h>

namespace xtd{

  template <typename _ch_t> class xstring;

  using string = xstring<char>;
  using wstring = xstring<wchar_t>;
  using tstring = xstring<tchar>;

#if (!DOXY_INVOKED)
  namespace _{
    template <typename, typename> class xstring_format;

  }
#endif

  /** Extends std::string with some added functionality
   @tparam _ch_t character type
   */
  template <typename _ch_t> class xstring : public std::basic_string<_ch_t>{
  public:
    using _super_t = std::basic_string<_ch_t>;
    using size_type = typename _super_t::size_type;

    ///Generic constructor forwards everything to the base class
    template <typename ... _arg_ts>
    xstring(_arg_ts&&...oArgs)
      : _super_t(std::forward<_arg_ts>(oArgs)...){}


    bool ends_with(const xtd::cstring& suffix) const{
      
      auto sDest = _super_t::rbegin();
      auto sSrc = suffix.rbegin();
      for (; suffix.rend() != sSrc && sDest != _super_t::rend(); ++sSrc, ++sDest){
        if (*sSrc != *sDest) return false;
      }
      return (sSrc == suffix.rend());
    }

    /**
    Type safe formatting
    Appends each item in the parameter list together performing type-safe verification and printing
    @param oArgs variable elements appended together
     */

    static xstring format(){
      xstring sRet = ((_ch_t*)"\0\0\0");
      return sRet;
    }


    template <typename _ty, typename ... _arg_ts>
    static xstring format(const _ty & val, _arg_ts&&...oArgs){
      xstring sRet = _::xstring_format<_ch_t, const _ty &>::format(static_cast<const _ty &>(val));
      sRet += format(std::forward<_arg_ts>(oArgs)...);
      return sRet;
    }


#if (XTD_OS_WINDOWS & XTD_OS)
    static xstring from_resource(uint32_t resid) {
      xstring sRet(10, 0);
      for(;;) {
        auto iRet = LoadString(GetModuleHandle(nullptr), resid, &sRet[0], sRet.size());
        if (!iRet) return xstring();
        if (sRet.size() < static_cast<size_t>(iRet)) {
          sRet.resize(iRet);
          return sRet;
        }
        sRet.resize(2 * sRet.size());
      }
    }
#endif

    xstring& reverse(){
      std::reverse(_super_t::begin(), _super_t::end());
      return *this;
    }
    /**
    Converts the string to lower case
    @param loc The locale to use during conversion
     */
    xstring& to_lower(const std::locale& loc){
      for (auto & ch : *this){
        ch = std::tolower(ch, loc);
      }
      return *this;
    }

    /**
    Converts the string to upper case
    @param loc The locale to use during conversion
     */
    xstring& to_upper(const std::locale& loc){
      for (auto & ch : *this){
        ch = std::toupper(ch, loc);
      }
      return *this;
    }

    ///Trim leading whitespace
    xstring& ltrim(){
      auto oBegin = _super_t::begin();
      if (oBegin >= _super_t::end()){
        return *this;
      }
      for(;oBegin != _super_t::end() ; ++oBegin){
        if (!std::iswspace(*oBegin)){
          break;
        }
      }
      _super_t::erase(_super_t::begin(), oBegin);
      return *this;
    }

    ///Trim trailing whitespace
    xstring& rtrim(){
      auto oBegin = _super_t::end();
      if (oBegin <= _super_t::begin()){
        return *this;
      }
      --oBegin;
      for(;oBegin != _super_t::begin() ; --oBegin){
        if (!std::iswspace(*oBegin) && *oBegin){
          break;
        }
      }
      _super_t::erase(++oBegin, _super_t::end());
      return *this;
    }

    //Trim leading and trailing whitespace
    xstring& trim(){
      ltrim();
      return rtrim();
    }

    //replaces all occurrences of the characters in the oItems list with a specified character
    xstring& replace(std::initializer_list<_ch_t> oItems, _ch_t chReplace) {
      for (auto & oCh : *this) {
        bool bFound = false;
        for (const auto & oFind : oItems) {
          if (oFind == oCh) {
            bFound = true;
            break;
          }
        }
        if (bFound) {
          oCh = chReplace;
        }
      }
      return *this;
    }

    ///replaces all instances of a sub-string with a character
    xstring& replace(const xstring& src, _ch_t chReplace) {
      forever{
        auto i = _super_t::find(src.c_str());
        if (_super_t::npos == i) break;
        _super_t::erase(i, src.size() - 1);
        (*this)[i] = chReplace;
      }
      return *this;
    }

    ///removes all occurrences of a list
    xstring& remove(const std::initializer_list<_ch_t>& chars) {
      forever{
        bool found = false;
        for (_ch_t ch : chars) {
          auto i = _super_t::find(ch);
          if (_super_t::npos == i) continue;
          found = true;
          _super_t::erase(i, size_t(1));
        }
        if (!found) break;
      }
      return *this;
    }

    ///finds the first occurrence of any item
    size_type find_first_of(const std::initializer_list<_ch_t>& delimiters, size_type pos = 0) const{
      size_type iRet = _super_t::npos;
      for (const _ch_t ch : delimiters){
        auto x = _super_t::find_first_of(ch, pos);
        if ((_super_t::npos != x) && (_super_t::npos == iRet || x < iRet)){
          iRet = x;
        }
      }
      return iRet;
    }
    ///finds the first occurrence of an item from a user-defined visitor
    size_type find_first_of(const std::function<bool(_ch_t)>& VisitorFN, size_type pos = 0) const{
      for (size_type i = pos; i < _super_t::size(); ++i){
        if (VisitorFN(_super_t::at(i))) return i;
      }
      return _super_t::npos;
    }

    size_type find_last_of(const std::initializer_list<_ch_t>& delimiters, size_type pos = _super_t::npos) const {
      size_type iRet = _super_t::npos;
      for (const _ch_t ch : delimiters){
        auto x = _super_t::find_last_of(ch, pos);
        if ((_super_t::npos != x) && (_super_t::npos == iRet || x > iRet)){
          iRet = x;
        }
      }
      return iRet;
    }

    ///splits the string by the specified delmiters into constituent elements
    std::vector<xstring<_ch_t>> split(const std::initializer_list<_ch_t>& delimiters, bool trimEmpty = false) const {
      using container_t = std::vector<xstring<_ch_t>>;
      container_t oRet;
      using _my_t = xstring<_ch_t>;
      size_type pos;
      size_type lastPos = 0;

      using value_type = typename container_t::value_type;

      forever{
        pos = find_first_of(delimiters, lastPos);
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
      size_type pos;
      size_type lastPos = 0;

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

#if (!(XTD_HAS_CODECVT | XTD_HAS_EXP_CODECVT)) && (XTD_HAS_ICONV)

    ///iconv handle wrapper
    class iconv_helper{
    public:
      iconv_helper(const iconv_helper&) = delete;
      iconv_helper& operator=(const iconv_helper&) = delete;
      iconv_helper(const char * to, const char * from) : _iconv(iconv_open(to, from)){
        if ((iconv_t)-1 == _iconv){
          throw std::runtime_error("Failure creating iconv handle");
        }
      }
      ~iconv_helper(){
        iconv_close(_iconv);
      }
      operator iconv_t(){ return _iconv; }
      iconv_t _iconv;
    };
#endif
  };

#if (!DOXY_INVOKED)

  namespace _{

  #if (XTD_HAS_CODECVT || XTD_HAS_EXP_CODECVT)
    template <> class xstring_format<char, const wchar_t * const &> {
    public:
      static inline string format(const wchar_t * const & src) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> oConv;
        return oConv.to_bytes(src);
      }
    };

    template <> class xstring_format<wchar_t, const char * const &>{
    public:
      static inline wstring format(const char * const & src){
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> oConv;
        return oConv.from_bytes(src);
      }
    };

#elif (!(XTD_HAS_CODECVT | XTD_HAS_EXP_CODECVT)) && (XTD_HAS_ICONV)

    template <> class xstring_format<char, const wchar_t * const &>{
    public:
      static string format(const wchar_t * const & src){
        static iconv_helper oIconv("UTF-8", "WCHAR_T");
        size_t srclen = wcslen(src);
        string sRet(srclen, 0);
        size_t retlen = srclen;
        iconv(oIconv, nullptr, nullptr, nullptr, &retlen);
        forever{
          retlen = sRet.size();
        auto pSrc = reinterpret_cast<const char *>(src);
        auto iSrcLen = srclen * sizeof(wchar_t);
        auto sRetPtr = &sRet[0];
        auto iRet = iconv(oIconv, const_cast<char**>(&pSrc), &iSrcLen, &sRetPtr, &retlen);
        if (static_cast<size_t>(-1) != iRet){
          return sRet;
        }
        if (errno != E2BIG){
          throw std::runtime_error("A string conversion error occurred");
        }
        sRet.resize(sRet.size() * 2);
        }
      }
    };

    template <> class xstring_format<wchar_t, const char * const &>{
    public:
      static inline wstring format(const char * const & src){
        static iconv_helper oIconv("WCHAR_T", "UTF-8");
        size_t srclen = strlen(src);
        wstring sRet(srclen, 0);
        size_t retlen = srclen;
        iconv(oIconv, nullptr, nullptr, nullptr, &retlen);
        forever{
          retlen = sRet.size() * sizeof(wchar_t);
        auto pSrc = const_cast<char*>(src);
        auto iSrcLen = srclen;
        auto sRetPtr = reinterpret_cast<char*>(&sRet[0]);
        auto iRet = iconv(oIconv, &pSrc, &iSrcLen, &sRetPtr, &retlen);
        if (static_cast<size_t>(-1) != iRet){
          return sRet;
        }
        if (errno != E2BIG){
          throw std::runtime_error("A string conversion error occurred");
        }
        sRet.resize(sRet.size() * 2);
        }
      }
    };
#elif (XTD_OS_WINDOWS & XTD_OS)

    template <> class xstring_format<char, const wchar_t * const &> {
    public:
      static string format(const wchar_t * const & src) {
        int iRet;
        if (!(iRet = ::WideCharToMultiByte(CP_UTF8, 0, src, -1, nullptr, 0, "?", nullptr)))
          throw std::runtime_error("A string conversion error occurred");
        string sRet(iRet-1, 0);
        ::WideCharToMultiByte(CP_UTF8, 0, src, -1, &sRet[0], iRet, "?", nullptr);
        return sRet;
      }
    };

    template <> class xstring_format<wchar_t, const char * const &> {
    public:
      static inline wstring format(const char * const & src) {
        int iRet;
        if (!(iRet = ::MultiByteToWideChar(CP_UTF8, 0, src, -1, nullptr, 0)))
          throw std::runtime_error("A string conversion error occurred");
        wstring sRet(iRet - 1, 0);
        ::MultiByteToWideChar(CP_UTF8, 0, src, -1, &sRet[0], iRet);
        return sRet;
      }
    };


  #else
    template <> class xstring_format<char, const wchar_t * const &>{
    public:
      static string format(const wchar_t * const & src){
        size_t srclen = wcslen(src);
        string sRet(srclen, 0);
        forever{
          //srclen = wcstombs_s(&sRet[0], sRet.size(), src, sRet.size());
          //wcstombs_s(&srclen, &sRet[0], sRet.size(), src, sRet.size());
          std::wcstombs(&sRet[0], src, sRet.size());
        if (static_cast<size_t>(-1) == srclen){
          throw std::runtime_error("A string conversion error occurred");
        }
        if (srclen < sRet.size()){
          break;
        }
        sRet.resize(srclen * 2);
        }
        sRet.resize(srclen);
        return sRet;
      }
    };

    template <> class xstring_format<wchar_t, const char * const &>{
    public:

      static inline wstring format(const char * const & src){
        size_t srclen = strlen(src);
        wstring sRet(1+srclen, '\0');
        forever{
          //srclen = mbstowcs_s(&sRet[0], sRet.size(), src, sRet.size());
          //mbstowcs_s(&srclen , &sRet[0], sRet.size(), src, sRet.size());
          std::mbstowcs(&sRet[0], src, sRet.size());
        if (static_cast<size_t>(-1) == srclen){
          throw std::runtime_error("A string conversion error occurred");
        }
        if (srclen <= sRet.size()){
          break;
        }
        sRet.resize(srclen * 2);
        }
        sRet.resize(srclen);
        return sRet;
      }
    };

  #endif

#if (XTD_OS_WINDOWS & XTD_OS)
    template <typename _ch_t> class xstring_format<_ch_t, const LPWSTR &> {
    public:
      static inline string format(const LPWSTR & src) {
        return xstring_format<_ch_t, const wchar_t*>::format(static_cast<const wchar_t*>(src));
      }
    };
#endif

    template <typename _ch_t> class xstring_format<_ch_t, const _ch_t * const &> {
    public:
      inline static xstring<_ch_t> format(const _ch_t * const & src) {
        return xstring<_ch_t>(src);
      }
    };

    template <typename _ch_t> class xstring_format<_ch_t, const _ch_t * > {
    public:
      inline static xstring<_ch_t> format(const _ch_t * src) {
        return xstring<_ch_t>(src);
      }
    };

    template <typename _ch_t, typename _ch2_t, size_t _Len> class xstring_format<_ch_t, const _ch2_t(&)[_Len]>{
    public:
      inline static xstring<_ch_t> format(const _ch2_t(&src)[_Len]){
        return xstring_format<_ch_t, const _ch2_t * const &>::format(src);
      }
    };

    template <typename _ch_t, typename _ch2_t> class xstring_format<_ch_t, const xtd::xstring<_ch2_t> &>{
    public:
      inline static xstring<_ch_t> format(const xtd::xstring<_ch2_t> & src){
        return xstring<_ch_t>::format(src.c_str());
      }
    };
    template <typename _ch_t, typename _ch2_t> class xstring_format<_ch_t, const std::basic_string<_ch2_t> &>{
    public:
      inline static xstring<_ch_t> format(const std::basic_string<_ch2_t> & src){
        return xstring<_ch_t>::format(src.c_str());
      }
    };


    template <> class xstring_format<char, const void * const &>{
    public:
      inline static string format(const void * const & value){
        return std::to_string(reinterpret_cast<size_t>(value));
      }
    };


    template <> class xstring_format<char, const int32_t &>{
    public:
      inline static string format(const int32_t & value){
        return std::to_string(value);
      }
    };

    template <> class xstring_format<char, const uint32_t &>{
    public:
      inline static string format(const uint32_t & value){
        return std::to_string(value);
      }
    };

    template <> class xstring_format<char, const int64_t &>{
    public:
      inline static string format(const int64_t & value){
        return std::to_string(value);
      }
    };

    template <> class xstring_format<char, const uint64_t &>{
    public:
      inline static string format(const uint64_t & value){
        return std::to_string(value);
      }
    };


    template <> class xstring_format<char, void * const &>{
    public:
      inline static string format(const void * const & value){
        return std::to_string(reinterpret_cast<size_t>(value));
      }
    };





    template <> class xstring_format<wchar_t, const void * const &> {
    public:
      inline static wstring format(const void * const & value) {
        return std::to_wstring(reinterpret_cast<size_t>(value));
      }
    };


    template <> class xstring_format<wchar_t, const int32_t &> {
    public:
      inline static wstring format(const int32_t & value) {
        return std::to_wstring(value);
      }
    };

    template <> class xstring_format<wchar_t, const uint32_t &> {
    public:
      inline static wstring format(const uint32_t & value) {
        return std::to_wstring(value);
      }
    };

    template <> class xstring_format<wchar_t, const int64_t &> {
    public:
      inline static wstring format(const int64_t & value) {
        return std::to_wstring(value);
      }
    };

    template <> class xstring_format<wchar_t, const uint64_t &> {
    public:
      inline static wstring format(const uint64_t & value) {
        return std::to_wstring(value);
      }
    };


    template <> class xstring_format<wchar_t, void * const &> {
    public:
      inline static wstring format(const void * const & value) {
        return std::to_wstring(reinterpret_cast<size_t>(value));
      }
    };


#if (XTD_OS_WINDOWS & XTD_OS)
    template <> class xstring_format<char, const DWORD &>{
    public:
      inline static string format(const DWORD & value){
        return std::to_string(value);
      }
    };
#endif
  }
#endif
}

namespace std{
  template <typename _ch_t> struct iterator_traits<xtd::xstring<_ch_t>> : std::iterator_traits<std::basic_string<_ch_t>>{};
}
#endif
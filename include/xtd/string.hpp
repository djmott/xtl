/** @file
  specializations of std::basic_string for advanced and common string handling
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#pragma once

#include <xtd/xtd.hpp>

#include <string>
#include <vector>
#include <stdexcept>
#include <functional>

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

  template <typename _ChT> class xstring;

  using string = xstring<char>;
  using wstring = xstring<wchar_t>;
  using tstring = xstring<tchar>;

#if (!DOXY_INVOKED)
  namespace _{
    template <typename, typename> class xstring_format;

  }
#endif

  /** Extends std::string with some added functionality
   @tparam _ChT character type
   */
  template <typename _ChT> class xstring : public std::basic_string<_ChT>{
  public:
    using _super_t = std::basic_string<_ChT>;
    using size_type = typename _super_t::size_type;

    ///Generic constructor forwards everything to the base class
    template <typename ... _ArgsT>
    xstring(_ArgsT&&...oArgs)
      : _super_t(std::forward<_ArgsT>(oArgs)...){}


    bool ends_with(const xtd::string& suffix) const{
      
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
      xstring sRet = ((_ChT*)"\0\0\0");
      return sRet;
    }


    template <typename _Ty, typename ... _ArgsT>
    static xstring format(const _Ty & val, _ArgsT&&...oArgs){
      xstring sRet = _::xstring_format<_ChT, const _Ty &>::format(static_cast<const _Ty &>(val));
      sRet += format(std::forward<_ArgsT>(oArgs)...);
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
      size_t iEnd = _super_t::size();
      for (size_t i=0 ; i<iEnd /2; ++i){
        std::swap((*this)[i], (*this)[iEnd - i - 1]);
      }
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
    xstring& replace(std::initializer_list<_ChT> oItems, _ChT chReplace) {
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
    xstring& replace(const xstring& src, _ChT chReplace) {
      forever{
        auto i = _super_t::find(src.c_str());
        if (_super_t::npos == i) break;
        _super_t::erase(i, src.size() - 1);
        (*this)[i] = chReplace;
      }
      return *this;
    }

    ///removes all occurrences of a list
    xstring& remove(const std::initializer_list<_ChT>& chars) {
      forever{
        bool found = false;
        for (_ChT ch : chars) {
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
    size_type find_first_of(const std::initializer_list<_ChT>& delimiters, size_type pos = 0) const{
      size_type iRet = _super_t::npos;
      for (const _ChT ch : delimiters){
        auto x = _super_t::find_first_of(ch, pos);
        if ((_super_t::npos != x) && (_super_t::npos == iRet || x < iRet)){
          iRet = x;
        }
      }
      return iRet;
    }
    ///finds the first occurrence of an item from a user-defined visitor
    size_type find_first_of(const std::function<bool(_ChT)>& VisitorFN, size_type pos = 0) const{
      for (size_type i = pos; i < _super_t::size(); ++i){
        if (VisitorFN(_super_t::at(i))) return i;
      }
      return _super_t::npos;
    }

    size_type find_last_of(const std::initializer_list<_ChT>& delimiters, size_type pos = _super_t::npos) const {
      size_type iRet = _super_t::npos;
      for (const _ChT ch : delimiters){
        auto x = _super_t::find_last_of(ch, pos);
        if ((_super_t::npos != x) && (_super_t::npos == iRet || x > iRet)){
          iRet = x;
        }
      }
      return iRet;
    }

    ///splits the string by the specified delmiters into constituent elements
    std::vector<xstring<_ChT>> split(const std::initializer_list<_ChT>& delimiters, bool trimEmpty = false) const {
      using container_t = std::vector<xstring<_ChT>>;
      container_t oRet;
      using _my_t = xstring<_ChT>;
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
    std::vector<xstring<_ChT>> split(const xstring<_ChT>& delim, bool trimEmpty = false) const {
      using container_t = std::vector<xstring<_ChT>>;
      container_t oRet;
      using _my_t = xstring<_ChT>;
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

    std::vector<xstring<_ChT>> split(const std::function<bool(_ChT)>& VisitorFN) const{
      using container_t = std::vector<xstring<_ChT>>;
      using value_type = typename container_t::value_type;
      container_t oRet;
      using _my_t = xstring<_ChT>;
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
    template <> class xstring_format<char, const wchar_t * const &>{
    public:
      static inline string format(const wchar_t * const & src){
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

    template <typename _ChT> class xstring_format<_ChT, const _ChT * const &> {
    public:
      inline static xstring<_ChT> format(const _ChT * const & src) {
        return xstring<_ChT>(src);
      }
    };

    template <typename _ChT> class xstring_format<_ChT, const _ChT * > {
    public:
      inline static xstring<_ChT> format(const _ChT * src) {
        return xstring<_ChT>(src);
      }
    };

    template <typename _ChT, typename _Ch2, size_t _Len> class xstring_format<_ChT, const _Ch2(&)[_Len]>{
    public:
      inline static xstring<_ChT> format(const _Ch2(&src)[_Len]){
        return xstring_format<_ChT, const _Ch2 * const &>::format(src);
      }
    };

    template <typename _ChT, typename _Ch2> class xstring_format<_ChT, const xtd::xstring<_Ch2> &>{
    public:
      inline static xstring<_ChT> format(const xtd::xstring<_Ch2> & src){
        return xstring<_ChT>::format(src.c_str());
      }
    };
    template <typename _ChT, typename _Ch2> class xstring_format<_ChT, const std::basic_string<_Ch2> &>{
    public:
      inline static xstring<_ChT> format(const std::basic_string<_Ch2> & src){
        return xstring<_ChT>::format(src.c_str());
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
  template <typename _ChT> struct iterator_traits<xtd::xstring<_ChT>> : std::iterator_traits<std::basic_string<_ChT>>{};
}
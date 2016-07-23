/** @file
  specializations of std::basic_string for advanced and common string handling
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#pragma once



namespace xtd{

  template <typename _ChT> class xstring;

  using string = xstring<char>;
  using wstring = xstring<wchar_t>;
  using tstring = xstring<tchar>;

#if (!DOXY_INVOKED)
  namespace _{
    template <typename, typename ...> struct xstring_format;

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

    /**
    Type safe formatting
    Appends each item in the parameter list together performing type-safe verification and printing
    @param _ArgsT variable elements appended together
     */

    template <typename ... _ArgsT>
    static xstring format(_ArgsT&&...oArgs){
      return _::xstring_format<_ChT, _ArgsT...>::format(std::forward<_ArgsT>(oArgs)...);
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

    //replaces all occurances of the characters in the oItems list with a specified character
    xstring& replace(std::initializer_list<_ChT> oItems, _ChT chReplace){
      for (auto & oCh : *this){
        bool bFound = false;
        for (const auto & oFind : oItems){
          if (oFind == oCh){
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

    ///finds the first occurance of any item
    size_type find_first_of(const std::initializer_list<_ChT>& delimiters, size_type pos = 0) const{
      size_type sRet = _super_t::npos;
      for (const _ChT ch : delimiters){
        auto x = _super_t::find_first_of(ch, pos);
        if ((_super_t::npos != x) && (_super_t::npos == sRet || x < sRet)){
          sRet = x;
        }
      }
      return sRet;
    }

    ///splits the string by the specified delmiters into constituent elements
    std::vector<xstring<_ChT>> split(const std::initializer_list<_ChT>& delimiters, bool trimEmpty = false) const{
      using container_t = std::vector<xstring<_ChT>>;
      container_t oRet;
      using _my_t = xstring<_ChT>;
      size_type pos;
      size_type lastPos = 0;

      using value_type = typename container_t::value_type;

      forever{
        pos = find_first_of(delimiters, lastPos);
        if (pos == _my_t::npos){
          pos = _super_t::length();
          if (pos != lastPos || !trimEmpty){
            oRet.push_back(value_type(_super_t::data() + lastPos, (pos - lastPos)));
          }
          break;
        } else if (pos != lastPos || !trimEmpty){
          oRet.push_back(value_type(_super_t::data() + lastPos , (pos - lastPos)));
        }

        lastPos = pos + 1;
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


  namespace _{

    //empty
    template <typename _ChT> struct xstring_format<_ChT>{
      template <typename ... _ArgTs> static xstring<_ChT> format(_ArgTs...){ return xstring<_ChT>(); }
    };


#if (XTD_HAS_CODECVT || XTD_HAS_EXP_CODECVT)

    template <typename ... _ArgTs> struct xstring_format<char, const wchar_t *, _ArgTs...>{
      static inline string format(const wchar_t * src, _ArgTs...oArgs){
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> oConv;
        return oConv.to_bytes(src) + xstring_format<char, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<wchar_t, const char *, _ArgTs...>{
      static inline wstring format(const char * src, _ArgTs...oArgs){
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> oConv;
        return oConv.from_bytes(src) + xstring_format<wchar_t, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

#elif (XTD_HAS_ICONV)

    template <typename ... _ArgTs> struct xstring_format<char, const wchar_t *, _ArgTs...>{
      static string format(const wchar_t * src, _ArgTs...oArgs){
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

    template <typename ... _ArgTs> struct xstring_format<wchar_t, const char *, _ArgTs...>{
      static inline wstring format(const char * src, _ArgTs...oArgs){
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
          if (static_cast<size_t>(-1) != iRet) {
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

    template <typename ... _ArgTs> struct xstring_format<char, const wchar_t *, _ArgTs...> {
      static string format(const wchar_t *src, _ArgTs...oArgs) {
        size_t srclen = wcslen(src);
        string sRet(srclen, 0);
        forever {
          srclen = wcstombs(&sRet[0], src, sRet.size());
          if (static_cast<size_t>(-1) == srclen) {
            throw std::runtime_error("A string conversion error occurred");
          }
          if (srclen < sRet.size()) {
            break;
          }
          sRet.resize(srclen * 2);
        }
        sRet.resize(srclen);
        return sRet;
      }
    };

    template <typename ... _ArgTs> struct xstring_format<wchar_t, const char *, _ArgTs...> {

      static inline wstring format(const char *src, _ArgTs...oArgs) {
        size_t srclen = strlen(src);
        wstring sRet(srclen, 0);
        forever {
          srclen = mbstowcs(&sRet[0], src, sRet.size());
          if (static_cast<size_t>(-1) == srclen) {
            throw std::runtime_error("A string conversion error occurred");
          }
          if (srclen < sRet.size()) {
            break;
          }
          sRet.resize(srclen * 2);
        }
        sRet.resize(srclen);
        return sRet;
      }
    };

#endif


    template <typename ... _ArgTs> struct xstring_format<char, int8_t&, _ArgTs...>{
      inline static string format(const int8_t &value, _ArgTs&&...oArgs){
        return std::to_string(value) + xstring_format<char, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<wchar_t, int8_t&, _ArgTs...>{
      inline static wstring format(const int8_t &value, _ArgTs&&...oArgs){
        return std::to_wstring(value) + xstring_format<wchar_t, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<char, uint8_t&, _ArgTs...>{
      inline static string format(const uint8_t &value, _ArgTs&&...oArgs){
        return std::to_string(value) + xstring_format<char, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<wchar_t, uint8_t&, _ArgTs...>{
      inline static wstring format(const uint8_t &value, _ArgTs&&...oArgs){
        return std::to_wstring(value) + xstring_format<wchar_t, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<char, int16_t&, _ArgTs...>{
      inline static string format(const int16_t &value, _ArgTs&&...oArgs){
        return std::to_string(value) + xstring_format<char, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<wchar_t, int16_t&, _ArgTs...>{
      inline static wstring format(const int16_t &value, _ArgTs&&...oArgs){
        return std::to_wstring(value) + xstring_format<wchar_t, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<char, uint16_t&, _ArgTs...>{
      inline static string format(const uint16_t &value, _ArgTs&&...oArgs){
        return std::to_string(value) + xstring_format<char, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<wchar_t, uint16_t&, _ArgTs...>{
      inline static wstring format(const uint16_t &value, _ArgTs&&...oArgs){
        return std::to_wstring(value) + xstring_format<wchar_t, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<char, int32_t&, _ArgTs...>{
      inline static string format(const int32_t &value, _ArgTs&&...oArgs){
        return std::to_string(value) + xstring_format<char, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<wchar_t, int32_t&, _ArgTs...>{
      inline static wstring format(const int32_t &value, _ArgTs&&...oArgs){
        return std::to_wstring(value) + xstring_format<wchar_t, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<char, uint32_t&, _ArgTs...>{
      inline static string format(const uint32_t &value, _ArgTs&&...oArgs){
        return std::to_string(value) + xstring_format<char, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<wchar_t, uint32_t&, _ArgTs...>{
      inline static wstring format(const uint32_t &value, _ArgTs&&...oArgs){
        return std::to_wstring(value) + xstring_format<wchar_t, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<char, int, _ArgTs...>{
      inline static string format(int value, _ArgTs&&...oArgs){
        return std::to_string(value) + xstring_format<char, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<wchar_t, int, _ArgTs...>{
      inline static wstring format(int value, _ArgTs&&...oArgs){
        return std::to_wstring(value) + xstring_format<wchar_t, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<char, unsigned int, _ArgTs...>{
      inline static string format(unsigned int value, _ArgTs&&...oArgs){
        return std::to_string(value) + xstring_format<char, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<wchar_t, unsigned int, _ArgTs...>{
      inline static wstring format(unsigned int value, _ArgTs&&...oArgs){
        return std::to_wstring(value) + xstring_format<wchar_t, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };
  #if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
    template <typename ... _ArgTs> struct xstring_format<char, DWORD&, _ArgTs...>{
      inline static string format(DWORD& value, _ArgTs&&...oArgs){
        return std::to_string(value) + xstring_format<char, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<wchar_t, DWORD&, _ArgTs...>{
      inline static wstring format(DWORD& value, _ArgTs&&...oArgs){
        return std::to_wstring(value) + xstring_format<wchar_t, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };
  #endif

    template <int _Len, typename ... _ArgTs> struct xstring_format<char, const wchar_t(&)[_Len], _ArgTs...>{
      inline static string format(const wchar_t(&src)[_Len], _ArgTs&&...oArgs){
        return xstring_format<char, const wchar_t*, _ArgTs...>::format(static_cast<const wchar_t*>(src), std::forward<_ArgTs>(oArgs)...);
      }
    };


    template <typename ... _ArgTs> struct xstring_format<char, xstring<wchar_t>&, _ArgTs...>{
      inline static string format(const xstring<wchar_t>&src, _ArgTs&&...oArgs){
        return xstring_format<char, const wchar_t*, _ArgTs...>::format(src.c_str(), std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <int _Len, typename ... _ArgTs> struct xstring_format<wchar_t, const char(&)[_Len], _ArgTs...>{
      inline static wstring format(const char(&src)[_Len], _ArgTs&&...oArgs){
        return xstring_format<wchar_t, const char*, _ArgTs...>::format(static_cast<const char*>(src), std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename ... _ArgTs> struct xstring_format<wchar_t, xstring<char>&, _ArgTs...>{
      inline static wstring format(const xstring<char>&src, _ArgTs&&...oArgs){
        return xstring_format<wchar_t, const char*, _ArgTs...>::format(src.c_str(), std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename _ChT, int _Len, typename ... _ArgTs> struct xstring_format<_ChT, const _ChT(&)[_Len], _ArgTs...>{
      inline static xstring<_ChT> format(const _ChT(&src)[_Len], _ArgTs&&...oArgs){
        return xstring_format<_ChT, const _ChT *, _ArgTs...>::format(static_cast<const _ChT*>(src), std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename _ChT, typename ... _ArgTs> struct xstring_format<_ChT, const xstring<_ChT>&, _ArgTs...>{
      inline static xstring<_ChT> format(const xstring<_ChT>& src, _ArgTs&&...oArgs){
        return src + xstring_format<_ChT, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };


    template <typename _ChT, typename ... _ArgTs> struct xstring_format<_ChT, const _ChT*&, _ArgTs...>{
      inline static xstring<_ChT> format(const _ChT*&src, _ArgTs&&...oArgs){
        return xstring<_ChT>(src) + xstring_format<_ChT, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };

    template <typename _ChT, typename ... _ArgTs> struct xstring_format<_ChT, const _ChT*, _ArgTs...>{
      inline static xstring<_ChT> format(const _ChT*src, _ArgTs&&...oArgs){
        return xstring<_ChT>(src) + xstring_format<_ChT, _ArgTs...>::format(std::forward<_ArgTs>(oArgs)...);
      }
    };


    template <typename _ChT, typename ... _TailT> struct xstring_format<_ChT, const void *, _TailT...>{
      static xstring<_ChT> format(const void * val, _TailT...oArgs){
        _ChT chars[] = { (_ChT)'0', (_ChT)'1', (_ChT)'2', (_ChT)'3', (_ChT)'4', (_ChT)'5', (_ChT)'6', (_ChT)'7',
                         (_ChT)'8', (_ChT)'9', (_ChT)'a', (_ChT)'b', (_ChT)'c', (_ChT)'d', (_ChT)'e', (_ChT)'f' };
        std::basic_string<_ChT> sTemp = "";
        auto value = reinterpret_cast<size_t>(val);
        for (uint8_t i=0 ; i<(sizeof(value) * 2) ; ++i, value <<= 4){
          auto ch = (0xf0000000 & value) >> 28;
          if (!ch && !sTemp.size()) continue;
          sTemp += chars[ch];
        }
        std::basic_string<_ChT> sRet = "0x";
        sRet += _::xstring_format<_ChT, _TailT...>::format(std::forward<_TailT>(oArgs)...);
        return sRet;
      }
    };

  }
}

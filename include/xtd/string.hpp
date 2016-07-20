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
    template <typename, typename > class xstring_tostring;
    template <typename, typename ...> class xstring_format;

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
      xstring sRet;
      _::xstring_format<_ChT, _ArgsT...>::format(sRet, std::forward<_ArgsT>(oArgs)...);
      return  sRet;
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

    ///Convert data to a string
    template <typename _Ty> inline static xstring<_ChT> from(const _Ty& src);
    template <typename _Ty> inline static xstring<_ChT> from(const _Ty* src);

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
      /** \brief The iconv. */
      iconv_t _iconv;
    };
#endif
  };

#if (!DOXY_INVOKED)
#if (XTD_HAS_CODECVT || XTD_HAS_EXP_CODECVT)
  template <> template <> inline string string::from<wchar_t>(const wchar_t* src){
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> oConv;
    return oConv.to_bytes(src);
  }

  template <> template <> inline wstring wstring::from<char>(const char * src){
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> oConv;
    return oConv.from_bytes(src);
  }

#elif (XTD_HAS_ICONV)

  template <> template <> inline string string::from<wchar_t>(const wchar_t * src){
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

  template <> template <> inline wstring wstring::from<char>(const char * src){
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
#else
  template <> template <> inline string string::from<wchar_t>(const wchar_t * src) {
    size_t srclen = wcslen(src);
    string sRet(srclen, 0);
    forever{
      srclen = wcstombs(&sRet[0], src, sRet.size());
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
  template <> template <> inline wstring wstring::from<char>(const char * src){
    size_t srclen = strlen(src);
    wstring sRet(srclen, 0);
    forever{
      srclen = mbstowcs(&sRet[0], src, sRet.size());
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
#endif


  template <> template <> inline wstring wstring::from<string>(const string& src){ return from<char>(src.c_str()); }
  template <> template <> inline string string::from<wstring>(const wstring& src){ return from<wchar_t>(src.c_str()); }

  template <> template <> inline string string::from<string>(const string& src){ return string(src); }
  template <> template <> inline wstring wstring::from<wstring>(const wstring& src){ return wstring(src); }

  namespace _{


    template <typename _ChT> class xstring_format<_ChT>{
    public:
      inline static void format(const xstring<_ChT>&){
        //specialization terminates 'recursive' calls to xstring_format::format() chains
      }
    };

    template <typename _ChT, typename ... _ArgsT>
    class xstring_format<_ChT, const _ChT*const&, _ArgsT...>{
    public:
      inline static void format(xstring<_ChT>& dest, const _ChT*const& oArg, _ArgsT&&...oArgs){
        xstring_format<_ChT, const _ChT*, _ArgsT...>::format(dest, oArg, std::forward<_ArgsT>(oArgs)...);
      }
    };


    template <typename ... _ArgsT>
    class xstring_format<wchar_t, const wchar_t *, _ArgsT...>{
    public:
      inline static void format(wstring& dest, const wchar_t * src, _ArgsT&&...oArgs){
        dest += src;
        xstring_format<wchar_t, _ArgsT...>::format(dest, std::forward<_ArgsT>(oArgs)...);
      }
    };


    template <typename ... _ArgsT>
    class xstring_format<wchar_t, const char *, _ArgsT...>{
    public:
      inline static void format(wstring& dest, const char * src, _ArgsT&&...oArgs){
        dest += wstring::from(src);
        xstring_format<wchar_t, _ArgsT...>::format(dest, std::forward<_ArgsT>(oArgs)...);
      }
    };


    template <typename ... _ArgsT>
    class xstring_format<char, const char *, _ArgsT...>{
    public:
      inline static void format(string& dest, const char * src, _ArgsT&&...oArgs){
        dest += src;
        xstring_format<char, _ArgsT...>::format(dest, std::forward<_ArgsT>(oArgs)...);
      }
    };


    template <typename ... _ArgsT>
    class xstring_format<char, const wchar_t *, _ArgsT...>{
    public:
      inline static void format(string& dest, const wchar_t * src, _ArgsT&&...oArgs){
        dest += string::from(src);
        xstring_format<char, _ArgsT...>::format(dest, std::forward<_ArgsT>(oArgs)...);
      }
    };

    template <typename _ChT, typename ... _ArgsT>
    class xstring_format<_ChT, const _ChT*&, _ArgsT...>{
    public:
      inline static void format(xstring<_ChT>& dest, const _ChT*&src, _ArgsT&&...oArgs){
        dest.append(src);
        xstring_format<_ChT, _ArgsT...>::format(dest, std::forward<_ArgsT>(oArgs)...);
      }
    };

    template <typename _ChT, int _Len, typename ... _ArgsT>
    class xstring_format<_ChT, const _ChT(&)[_Len], _ArgsT...>{
    public:
      inline static void format(xstring<_ChT>& dest, const _ChT(&src)[_Len], _ArgsT&&...oArgs){
        dest.append(&src[0], &src[_Len - 1]);
        xstring_format<_ChT, _ArgsT...>::format(dest, std::forward<_ArgsT>(oArgs)...);
      }
    };

    template <typename _ChT, typename ... _ArgsT>
    class xstring_format<_ChT, const xstring<_ChT>&, _ArgsT...>{
    public:
      inline static void format(xstring<_ChT> &dest, const xstring<_ChT> &oArg, _ArgsT &&...oArgs){
        dest.append(oArg);
        xstring_format<_ChT, _ArgsT...>::format(dest, std::forward<_ArgsT>(oArgs)...);
      }
    };

  #if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
    //DWORD
    template <typename ... _ArgsT>
    class xstring_format<char, DWORD&, _ArgsT...>{
    public:
      inline static void format(xstring<char> &dest, const DWORD& newval, _ArgsT &&...oArgs){
        dest.append(std::to_string(newval));
        xstring_format<char, _ArgsT...>::format(dest, std::forward<_ArgsT>(oArgs)...);
      }
    };
    template <typename ... _ArgsT>
    class xstring_format<wchar_t, DWORD&, _ArgsT...>{
    public:
      inline static void format(xstring<wchar_t> &dest, const DWORD& newval, _ArgsT &&...oArgs){
        dest.append(std::to_wstring(newval));
        xstring_format<wchar_t, _ArgsT...>::format(dest, std::forward<_ArgsT>(oArgs)...);
      }
    };

  #endif

  }
#endif
}

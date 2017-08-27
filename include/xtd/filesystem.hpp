/** @file
handle necessary filesystem and path functionality until C++17 is finalized
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/xtd.hpp>

#include <xtd/string.hpp>

#if (XTD_OS_UNIX & XTD_OS)
  #include <sys/stat.h>
  #include <paths.h>
#else
#include <xtd/exception.hpp>
#include <xtd/meta.hpp>
#endif

#include <xtd/exception.hpp>

#if (XTD_HAS_FILESYSTEM)

#include <filesystem>

#elif (XTD_HAS_EXP_FILESYSTEM)

#include <experimental/filesystem>

#endif

#if (XTD_HAS_FILESYSTEM || XTD_HAS_EXP_FILESYSTEM)


  namespace xtd{
    namespace filesystem{
      using namespace std::experimental::filesystem;
    }
  }


#else


namespace xtd{
  namespace filesystem{

    struct path : xtd::xstring<char> {

#if(XTD_OS_WINDOWS & XTD_OS)
      static const char preferred_separator = '\\';
      static const char not_preferred_separator = '/';
#else
      static const char preferred_separator = '/';
      static const char not_preferred_separator = '\\';
#endif

      template <typename ... _ArgTs> path(_ArgTs&&...oArgs) : xtd::string(std::forward<_ArgTs>(oArgs)...){}

      xtd::string& string() { return *this; }
      const xtd::string& string() const { return *this; }

      ///appends a perferred separator and path element
      path& append(const path& src){
        if (preferred_separator != back() && not_preferred_separator != back()
            && preferred_separator != src.front() && not_preferred_separator != src.front())
        {
          append(1, preferred_separator);
        }
        append(src.string().c_str());
        trim();
        return *this;
      }

      path& operator/=(const path& src) { return append(src); }

      //replaces all non-preferred separators with preferred separators
      path& make_preferred(){
        if (!length()) return *this;
        replace({not_preferred_separator}, preferred_separator);
        return *this;
      }

      //removes the last path element if it doesn't end in a separator
      path& remove_filename(){
        if (!length()) return *this;
        if (preferred_separator == back() || not_preferred_separator==back()) return *this;
        auto isep = find_last_of({preferred_separator, not_preferred_separator});
        if (xtd::string::npos != isep) erase(isep+1);
        return *this;
      }

      path& replace_filename(const path& replacement){
        remove_filename();
        return operator/=(replacement);
      }
    private:
      xtd::string _str;
    };


    inline bool remove(const path& oPath) { return (0==std::remove(oPath.c_str())); }

//temp_directory_path
#if (XTD_OS_WINDOWS & XTD_OS)


    inline path temp_directory_path() {
      xtd::string sTemp(1 + MAX_PATH, 0);
      sTemp.resize(xtd::windows::exception::throw_if(GetTempPath(MAX_PATH, &sTemp[0]), [](DWORD d){return 0 == d;}));
      return path(sTemp);
    }


#elif (XTD_OS_UNIX & XTD_OS)


    inline path temp_directory_path(){
      auto sTemp = getenv("TMPDIR");
      if (sTemp && strlen(sTemp)) return path(sTemp);
      sTemp = getenv("TEMP");
      if (sTemp && strlen(sTemp)) return path(sTemp);
#if defined(P_tmpdir)
      return path(P_tmpdir);
#endif
#if defined(_PATH_TMP)
      return path(_PATH_TMP);
#endif
      throw xtd::exception(here(), "Unable to determine temp path");
    }


#endif



  }
}

#endif


//Custom extensions to std::filesystem

namespace xtd{
  namespace filesystem{

  }
}




#if 0
/*
#include <stdlib.h>

namespace xtd {
  namespace filesystem {

    namespace _ {
      template <typename, typename>
      struct path_adder;
    }
  }
}
*/

#if (XTD_HAS_EXP_FILESYSTEM)
namespace xtd {
  namespace filesystem {


    struct path : std::experimental::filesystem::path {
      using _super_t = std::experimental::filesystem::path;

      template <typename ... _ArgTs>
      path(_ArgTs&&...oArgs) : _super_t(std::forward<_ArgTs>(oArgs)...) {}

      template <typename _Ty>
      inline path operator+(const _Ty& src) const {
        path oRet(*this);
        oRet /= src;
        return oRet;
      }

      path filename() const { return path(_super_t::filename()); }

    };

    static inline bool create_directories(const path& src) { return std::experimental::filesystem::create_directories(src); }

    static inline bool exists(const path& src) { return std::experimental::filesystem::exists(src); }

    static inline bool remove(const path& src) { return std::experimental::filesystem::remove(src); }

    static inline path temp_directory_path() { return path(std::experimental::filesystem::temp_directory_path()); }


#if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)

    static inline bool is_directory(const path& oPath) { return FILE_ATTRIBUTE_DIRECTORY & GetFileAttributes(xtd::tstring::format(oPath.string().c_str()).c_str()) ? true : false; }

#if (XTD_OS_WINDOWS & XTD_OS)
    template <const KNOWNFOLDERID & _id>
    static inline path known_path() {
      PWSTR sTemp;
      xtd::windows::exception::throw_if(SHGetKnownFolderPath(_id, 0, nullptr, &sTemp), [](HRESULT hr) { return FAILED(hr); });
      RAII(CoTaskMemFree(sTemp));
      return xtd::string::format(static_cast<const wchar_t*>(sTemp));
    }

    static inline path home_directory_path() { return known_path<FOLDERID_Profile>(); }
  #else
    static inline path home_directory_path() { return path(getenv("HOME")); }
#endif

  }
}
#elif (XTD_HAS_FILESYSTEM)

namespace xtd{
  namespace filesystem{
    using namespace std::filesystem;
  }
}

#else

namespace xtd{
  namespace filesystem{


    class path_base : public xtd::string{
    public:

      xtd::string& string() { return static_cast<xtd::string&>(*this);}
      const xtd::string& string() const { return static_cast<const xtd::string&>(*this);}
      template <typename ... _ArgTs> path_base(_ArgTs&&...oArgs) : xtd::string(std::forward<_ArgTs>(oArgs)...){}
    };

    class path : public path_base{
    public:
#if (XTD_OS_WINDOWS & XTD_OS)
      static constexpr value_type preferred_separator  = '\\';
      static constexpr value_type non_preferred_separator  = '/';
#else
      static constexpr value_type preferred_separator  = '/';
      static constexpr value_type non_preferred_separator  = '\\';
#endif

      template <typename ... _ArgTs> path(_ArgTs&&...oArgs) : path_base(std::forward<_ArgTs>(oArgs)...){}


      template <typename _RHST>
      path& operator /= (const _RHST& rhs){
        append(rhs);
        return *this;
      }

      template <typename _Ty>
      inline path operator+(const _Ty& src) const{
        path oRet(*this);
        oRet /= src;
        return oRet;
      }

      path& remove_filename(){
        if (preferred_separator == string().back()){
          return *this;
        }
        auto iEnd = --end();
        for (; iEnd > begin() && preferred_separator != *iEnd && non_preferred_separator != *iEnd; --iEnd);
        if (iEnd > begin()){
          *this = substr(0, iEnd-begin());
        }
        return *this;
      }

      path& replace_filename(const path& newval){
        remove_filename();
        *this /= newval;
        return *this;
      }

      path filename() const {
        auto iEndSep = string().find_last_of(preferred_separator);
        return string().substr(1+iEndSep);
      }

      path& make_preferred(){
        for (auto & oCh : *this){
          if (oCh == non_preferred_separator){
            oCh = preferred_separator;
          }
        }
        return *this;
      }

      path& append(const path& rhs){
        if (preferred_separator == back() || non_preferred_separator==back()) pop_back();
        if (preferred_separator != rhs.string().front() && non_preferred_separator != rhs.string().front()) push_back(preferred_separator);
        _super_t::append(rhs.string());
        return *this;
      }

      path& operator+=(const path& rhs){ return append(rhs); }

    };


#if (XTD_OS_WINDOWS & XTD_OS)
    inline path temp_directory_path() {
      xtd::string sTemp(1 + MAX_PATH, 0);
      sTemp.resize(xtd::windows::exception::throw_if(GetTempPath(MAX_PATH, &sTemp[0]), [](DWORD d){return 0 == d;}));
      return path(sTemp);
    }
#else

    static inline size_t file_size(const path& oPath){
      struct ::stat oStat;
      if (0==::stat(oPath.string().c_str(), &oStat)){
        return oStat.st_size;
      }
      return -1;
    }

    static inline bool exists(const path& oPath){
      FILE * pFile;
      fopen_s(&pFile, oPath.string().c_str(), "r");
      if (pFile) fclose(pFile);
      return (pFile ? true : false);
    }

    static inline path home_directory_path(){
      char * sRet;
      size_t len;
      _dupenv_s(&sRet, &len, "HOME");
      xtd::tstring sTemp(sRet);
      free(sRet);
      return path(sTemp);
    }


    inline path temp_directory_path(){
      char * cTempEnv;
      size_t len;
      _dupenv_s(&cTempEnv, &len, "TMPDIR");
      xtd::string sTemp(cTempEnv);
      free(cTempEnv);
      if (!cTempEnv || 0 == strlen(cTempEnv)) {
        _dupenv_s(&cTempEnv, &len, "TEMP");
        sTemp = cTempEnv;
        free(cTempEnv);
      }
#if defined(P_tmpdir)
      if (!cTempEnv || 0==strlen(cTempEnv)) sTemp = P_tmpdir;
#endif
    #if (XTD_COMPILER_GCC & XTD_COMPILER)
      if (!cTempEnv || 0==strlen(cTempEnv)) sTemp = _PATH_TMP;
    #endif
      return path(sTemp);
    }
#endif


    inline bool remove(const path& oPath){
      return 0==::remove(oPath.string().c_str());
    }
  }
}

#endif


namespace xtd {
  namespace filesystem {
    namespace _ {


      template <typename _ValueT, typename _Ch2, size_t _Dims>
      struct path_adder<_ValueT, const _Ch2(&)[_Dims]> {
        inline static path add(const path& dest, const _Ch2 (&src)[_Dims]) {
          auto str = xtd::xstring<_ValueT>::format(src);
          path oRet(dest);
          oRet.append(str);
          return oRet;
        }
      };

      template <typename _ValueT, typename _Ch2>
      struct path_adder<_ValueT, const _Ch2*> {
        inline static path add(const path& dest, const _Ch2* src) {
          auto str = xtd::xstring<_ValueT>::format(src);
          path oRet(dest);
          oRet.append(str);
          return oRet;
        }
      };

      template <typename _ValueT>
      struct path_adder<_ValueT, const path&> {
        inline static path add(const path& dest, const path& src) {
          path oRet(dest);
          oRet /= src;
          return oRet;
        }
      };

      template <typename _ValueT>
      struct path_adder<_ValueT, const xtd::xstring<_ValueT>&> {
        inline static path add(const path& dest, const xtd::xstring<_ValueT>& src) {
          return path_adder<_ValueT, const _ValueT*>::add(dest, src.c_str());
        }
      };
    }
  }
}
#endif
#endif
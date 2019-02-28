/** @file
handle necessary filesystem and path functionality until C++17 is finalized
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/xtd.hpp>

#if (XTD_OS_WINDOWS & XTD_OS)
  #include <windows.h>
	#include <ShlObj.h>
	#include <Shlwapi.h>
  #pragma comment(lib, "shlwapi")
#endif

#if (XTD_OS_UNIX & XTD_OS)
  #include <sys/stat.h>
  #include <paths.h>
  #include <pwd.h>
  #include <unistd.h>
#endif

#if (XTD_HAS_FILESYSTEM)
  #include <filesystem>
#elif (XTD_HAS_EXP_FILESYSTEM)
  #include <experimental/filesystem>
#endif

#include <xtd/xstring.hpp>
#include <xtd/exception.hpp>


namespace std {
	namespace experimental{}
	using namespace experimental;
}

#if (XTD_HAS_FILESYSTEM) 


namespace xtd {
  namespace filesystem {
    using namespace xtd::filesystem;
    using path = std::filesystem::path;
  }
}

#elif (XTD_HAS_EXP_FILESYSTEM)

namespace xtd {
  namespace filesystem {
    using namespace xtd::filesystem;
    using path = std::experimental::filesystem::path;
  }
}

#else

namespace xtd{
  namespace filesystem{

    struct path  {

#if(XTD_OS_WINDOWS & XTD_OS)
      static const tchar preferred_separator = __('\\');
      static const tchar not_preferred_separator = __('/');
#else
      static const tchar preferred_separator = __('/');
      static const tchar not_preferred_separator = __('\\');
#endif

      path() : _str() {}
      path(const tchar * src) : _str(src){}
      path(const xtd::tstring& src) : _str(src){}
      path(const path& src) : _str(src._str){}
      path(path&& src) : _str(std::move(src._str)){}

      path& operator=(const path& src){
        _str = src._str;
        return *this;
      }
      path& operator=(path&& src){
        std::swap(_str, src._str);
        return *this;
      }
      path& operator=(const xtd::tstring& src){
        _str = src;
        return *this;
      }

      path& operator=(const tchar * src){
        _str = src;
        return *this;
      }

      const xtd::tstring& tstring() const { return _str; }

      xtd::cstring string() const { return cstring().from(_str); }

      ///appends a preferred separator and path element
      path& append(const path& src) {
        if (preferred_separator != _str.back() && not_preferred_separator != _str.back()
            && preferred_separator != src._str.front() && not_preferred_separator != src._str.front())
        {
          _str.append(1, preferred_separator);
        }
        _str.append(src._str);
        _str.trim();
        return *this;
      }

      path& operator/=(const path& src) { return append(src); }
      path& operator/=(const xtd::tstring& src){ return append(src); }
      path& operator+=(const path& src) { _str.append(src._str); return *this; }

      //replaces all non-preferred separators with preferred separators
      path& make_preferred(){
        if (!_str.length()) return *this;
        _str.replace({not_preferred_separator}, preferred_separator);
        return *this;
      }

      //removes the last path element if it doesn't end in a separator
      path& remove_filename(){
        if (!_str.length()) return *this;
        if (preferred_separator == _str.back() || not_preferred_separator==_str.back()) return *this;
        auto isep = _str.find_last_of({preferred_separator, not_preferred_separator});
        if (xtd::tstring::npos != isep) _str.erase(isep);
        return *this;
      }

#if (XTD_OS_WINDOWS & XTD_OS)
      path& remove_path(){
        make_preferred();
        auto i = _str.rfind(preferred_separator, _str.length());
        if (xtd::tstring::npos != i) *this = _str.substr(1 + i, _str.length() - i);
        return *this;
      }
#endif

      path& replace_filename(const path& replacement){
        remove_filename();
        return operator/=(replacement);
      }

      xtd::tstring filename() const {
        path oPath(*this);
        oPath.make_preferred();
        auto i = _str.rfind(preferred_separator, _str.length());
        if (xtd::tstring::npos != i) return _str.substr(1+i, _str.length()-i);
        return __("");
      }

      bool operator<(const path& src) const { return _str < src._str; }

    private:
      xtd::tstring _str;
    };



//temp_directory_path
#if (XTD_OS_WINDOWS & XTD_OS)
    inline bool remove(const path& oPath) { return (::DeleteFile(oPath.tstring().c_str()) ? true : false); }

    inline path home_directory_path() {
      LPWSTR sRet = nullptr;
      xtd::windows::exception::throw_if(::SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &sRet), [](HRESULT h) { return FAILED(h); });
      auto sTemp = xtd::tstring().from(sRet);
      ::CoTaskMemFree(sRet);
      return path(sTemp);
    }

    inline path data_directory_path(){
      tstring sTemp(MAX_PATH, 0);
      xtd::windows::exception::throw_if(SHGetSpecialFolderPath(nullptr, &sTemp[0], CSIDL_COMMON_APPDATA, FALSE), [](BOOL b){ return !b; });
      //xtd::windows::exception::throw_if(::SHGetKnownFolderPath(FOLDERID_ProgramData, KF_FLAG_CREATE, nullptr, &sTemp[0]), [](HRESULT h) { return FAILED(h); });
      return path(sTemp.c_str());
    }

    inline path temp_directory_path() {
      xtd::tstring sTemp(1 + MAX_PATH, 0);
      sTemp.resize(xtd::windows::exception::throw_if(::GetTempPath(MAX_PATH, &sTemp[0]), [](DWORD d){return 0 == d;}));
      return path(sTemp);
    }

    inline bool exists(const path& oPath) {
      return (::PathFileExists(oPath.tstring().c_str()) ? true : false);
    }

    inline void create_directories(const path& oPath) {
      xtd::windows::exception::throw_if(::SHPathPrepareForWrite(nullptr, nullptr, oPath.tstring().c_str(), SHPPFW_DIRCREATE), [](HRESULT h) { return FAILED(h); });
    }

#elif (XTD_OS_UNIX & XTD_OS)

    inline bool remove(const path& oPath) { return (0 == std::remove(oPath.tstring().c_str())); }

    inline void create_directory(const path& oPath){
      mkdir(oPath.string().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    inline void create_directories(const path& oPath){
      path oTemp(oPath);
      oTemp.make_preferred();
      auto str = oTemp.string();
      auto dirs = str.split({path::preferred_separator});
      xtd::tstring sDir = "";
      for (const auto & dir : dirs){
        sDir += path::preferred_separator;
        sDir += dir;
        create_directory(sDir);
      }
    }

    inline path home_directory_path(){
      return path(getpwuid(getuid())->pw_dir);
    }


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

    inline bool exists(const path& oPath){
      return (-1 != access(oPath.string().c_str(), F_OK));
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


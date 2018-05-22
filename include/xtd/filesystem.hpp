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
  #include <pwd.h>
  #include <unistd.h>
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

namespace std{
  namespace filesystem{}
  namespace experimental{
    namespace filesystem{}
  }
}

#if (XTD_HAS_FILESYSTEM || XTD_HAS_EXP_FILESYSTEM)


  namespace xtd{
    namespace filesystem{
      using namespace std::experimental::filesystem;
      using namespace std::filesystem;
    }
  }


#else


namespace xtd{
  namespace filesystem{

    struct path  {

#if(XTD_OS_WINDOWS & XTD_OS)
      static const char preferred_separator = '\\';
      static const char not_preferred_separator = '/';
#else
      static const char preferred_separator = '/';
      static const char not_preferred_separator = '\\';
#endif

      path() : _str() {}
      path(const char * src) : _str(src){}
      path(const xtd::string& src) : _str(src){}
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
      path& operator=(const xtd::string& src){
        _str = src;
        return *this;
      }

      path& operator=(const char * src){
        _str = src;
        return *this;
      }

      const xtd::string& string() const { return _str; }

      ///appends a perferred separator and path element
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
        if (xtd::string::npos != isep) _str.erase(isep);
        return *this;
      }

      path& replace_filename(const path& replacement){
        remove_filename();
        return operator/=(replacement);
      }

      xtd::string filename() const {
        path oPath(*this);
        oPath.make_preferred();
        auto i = _str.rfind(preferred_separator, _str.length());
        if (std::string::npos != i) return _str.substr(1+i, _str.length()-i);
        return "";
      }

      bool operator<(const path& src) const { return _str < src._str; }

    private:
      xtd::string _str;
    };


    inline bool remove(const path& oPath) { return (0==std::remove(oPath.string().c_str())); }

//temp_directory_path
#if (XTD_OS_WINDOWS & XTD_OS)


    inline path temp_directory_path() {
      xtd::string sTemp(1 + MAX_PATH, 0);
      sTemp.resize(xtd::windows::exception::throw_if(GetTempPath(MAX_PATH, &sTemp[0]), [](DWORD d){return 0 == d;}));
      return path(sTemp);
    }


#elif (XTD_OS_UNIX & XTD_OS)
    inline void create_directory(const path& oPath){
      mkdir(oPath.string().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    inline void create_directories(const path& oPath){
      path oTemp(oPath);
      oTemp.make_preferred();
      auto str = oTemp.string();
      auto dirs = str.split({path::preferred_separator});
      xtd::string sDir = "";
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


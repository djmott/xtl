/** @file
handle necessary filesystem and path functionality until C++17 is finalized
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


#pragma once



namespace xtd{

  namespace _{
#if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
    using path_value_type = tchar;
#else
    using path_value_type = char;
#endif
  }

#if (XTD_HAS_FILESYSTEM)
  namespace filesystem{
    using path_base = std::experimental::filesystem::path;
  }
#endif

#if (!XTD_HAS_EXP_FILESYSTEM && !XTD_HAS_FILESYSTEM)

  namespace filesystem {
    /** base path class
    path_base is a placeholder for std::path and implements the same neccessary methods.  It will be removed when the standard is finalized.
    */

    class path_base : public xtd::xstring<_::path_value_type> {
    public:

#if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
      static const value_type seperator = __('\\');
#else
      static const value_type seperator = '/';
#endif

      using string_type = xtd::xstring<_::path_value_type>;

      inline operator string_type() const { return *this; }

      inline const string_type &native() const { return *this; }

      string_type &string() { return *this; }

      const string_type &string() const { return *this; }

      path_base() : string_type() {}

      explicit path_base(const path_base &src) : string_type(src) {}

      explicit path_base(path_base &&src) : string_type(std::move(src)) {}

      template<typename ... _Ty>
      path_base(_Ty &&... src) : string_type(std::forward<_Ty>(src)...) {}

      path_base &operator=(path_base &&src) {
        static_cast<string_type &>(*this) = std::move(src);
        return *this;
      }

      template<typename _Ty>
      path_base &append(const _Ty &src) {
        //add a seperator only if absent from the end of the current path and beginning of the appended item
        if (seperator != back() && seperator != src[0]) {
          *this += seperator;
        } else if (seperator == back() && seperator == src[0]) {
          //if both the end of the current path and the beginning of the appended path have a seperator then remove one
          pop_back();
        }
        *this += src;
        return *this;
      }

      template<typename _Ty>
      path_base &operator/=(const _Ty &src) { return append(src); }

      TODO("path_base::replace_filename needs a lot of work to be TS compliant");

      path_base &replace_filename(const path_base &src) {

        resize(1 + find_last_of(seperator));
        return append(src);
      }

      path_base &make_preferred() {
#if ((XTD_OS_LINUX | XTD_OS_CYGWIN | XTD_OS_MSYS) & XTD_OS)
        char non_preferred_seperator = '\\';
#elif ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
        char non_preferred_seperator = '/';
#endif
        for (size_type i = 0; i < size(); i++) {
          if ((*this)[i] == non_preferred_seperator) {
            (*this)[i] = seperator;
          }
        }
        return *this;
      }
    };
  }
#endif

  namespace filesystem {
    /** xtd specific path extensions
    */
    class path : public path_base {
      using _super_t = path_base;
    public:

    #if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
      static const value_type seperator = __('\\');
    #else
      static const value_type seperator = '/';
    #endif

      template<typename ... _ArgTs>
      path(_ArgTs...oArgs) : _super_t(std::forward<_ArgTs>(oArgs)...) {}

      path filename() const {
        std::string sRet = _super_t::string();
        auto iPos = sRet.find_last_of( seperator );
        if (xtd::string::npos != iPos && iPos < (sRet.size() - 1)){
          sRet.substr(1+iPos);
        }
        return sRet;
      }

#if ((XTD_OS_LINUX | XTD_OS_MSYS | XTD_OS_CYGWIN) & XTD_OS)

      /** returns the user's home directory
      */
      inline static path home_directory() {
        return path(std::getenv("HOME"));
      }
#elif ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
      inline static path home_directory(){
        std::string sTemp = std::getenv("HOMEPATH");
        if (0 == sTemp.c_str()){
          PWSTR pStr = nullptr;
          SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &pStr);
          RAII(CoTaskMemFree(pStr));
        }
        return sTemp;
      }
#endif

      /** returns the application data directory

      inline static path user_app_directory(){
        auto oRet = home_directory();
        oRet /= xtd::this_executable::get_path().filename();
        return oRet;
      }
        */
    };

  }
}

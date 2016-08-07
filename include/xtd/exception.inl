/** @file
deferred exception.hpp inline function definitions
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
#if ((XTD_OS_MINGW | XTD_OS_WINDOWS) & XTD_OS)
  namespace windows{

    inline exception::exception(const xtd::source_location& source, const std::string& expression) : xtd::exception(source, ""), _last_error(GetLastError()){
      const char * sTemp = nullptr;
      FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, _last_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&sTemp), 0, nullptr);
      _what = xtd::string::format("Windows exception ", _last_error, " ", xtd::string(sTemp).trim(), " ", expression);
      LocalFree((HLOCAL)sTemp);
    }

  }
#endif
}

#pragma once

#include <xtd/xtd.hpp>
#include <xtd/exception.hpp>

#define throw_false( _test )  _throw_false(here() ,  ( _test ) , # _test )

namespace xtd {
  namespace windows {
    struct exception : xtd::exception {
    public:



      template <typename _return_t, typename _expression_t>
      static _return_t _throw_if(const xtd::source_location& source, _return_t ret, _expression_t exp, const char* expstr) {
        if (exp(ret)) {
          auto iLastErr = GetLastError();
          throw exception(source, expstr, iLastErr);
        }
        return ret;
      }

      static BOOL _throw_false(const xtd::source_location& source, BOOL bTest, const char* expstr) {
        return _throw_if(source, bTest, [](BOOL b) { return !b; }, expstr);
      }

      exception(const xtd::source_location& source, const std::string& exp, uint32_t last_err) :xtd::exception(source, ""), _last_error(last_err) {
        const char* sTemp = nullptr;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, _last_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&sTemp), 0, nullptr);
        _what = xtd::string::format(sTemp, " : ", exp);
        LocalFree((HLOCAL)sTemp);
      }
    private:
      uint32_t _last_error;
    };
  }
}
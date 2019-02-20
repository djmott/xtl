/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once
#include <xtd/xtd.hpp>

#include <xtd/exception.hpp>


#define throw_hr_err(_fn, ...) xtd::com::exception::_throw_hr_error(here(), #_fn, _fn, __VA_ARGS__)

namespace xtd {
	namespace com {
		struct exception : xtd::exception {

      template <typename _SourceT>
      static HRESULT _throw_hr_error(xtd::source_location& loc, const char * code, HRESULT hr, _SourceT& src) {
        if (!FAILED(hr)) return hr;
        throw xtd::com::exception(loc, code, hr, src);
      }

      static HRESULT _throw_hr_error(xtd::source_location& loc, const char * code, HRESULT hr) {
        if (!FAILED(hr)) return hr;
        throw xtd::com::exception(loc, code, hr);
      }

      exception(const xtd::source_location& loc, const char * code, HRESULT hr) : xtd::exception(loc, "") {
      }

			template <typename _SourceT>
      exception(const xtd::source_location& loc, const char * code, HRESULT hr, _SourceT& src) : xtd::exception(loc,""){
        com::pointer<ISupportErrorInfo> oSupportErrInfo;
        com::pointer<IErrorInfo> oErrInfo;
        if (src	&& SUCCEEDED(src->QueryInterface(__uuidof(ISupportErrorInfo), (void**)&oSupportErrInfo))
          && SUCCEEDED(oSupportErrInfo->InterfaceSupportsErrorInfo(__uuidof(_SourceT)))
          && SUCCEEDED(GetErrorInfo(0, &oErrInfo))
          && oErrInfo) {
          _bstr_t bsDescription;
          GUID gErr;
          DWORD dwHelpCtx;
          _bstr_t bsHelpFile;
          _bstr_t bsSource;

          oErrInfo->GetDescription(bsDescription.GetAddress());
          oErrInfo->GetGUID(&gErr);
          oErrInfo->GetHelpContext(&dwHelpCtx);
          oErrInfo->GetHelpFile(bsHelpFile.GetAddress());
          oErrInfo->GetSource(bsSource.GetAddress());
          _what = xtd::format(L"%s : %s %s", (const wchar_t*)bsDescription, (const wchar_t*)bsSource, Convert<GUID>::To<std::wstring>(gErr).c_str());
          if (!!bsHelpFile) {
            _what += xtd::format(L"See : %s(%u)", (const wchar_t*)bsHelpFile, dwHelpCtx);
          }
        }
      }
		};
	}
}

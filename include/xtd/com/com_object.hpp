/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/xtd.hpp>
#include <windows.h>

#include <atomic>

namespace xtd{
	namespace com {
		template <typename _DeclT, LPCTSTR _Name, typename ... _Interfaces> struct Object;

		template <typename _DeclT, LPCTSTR _Name> struct Object <_DeclT, _Name > : IUnknown{
			using com_object_type = Object < _DeclT, _Name > ;
			virtual ~Object() { GlobalDLLLock()--; }
			Object() : _ThisRefCnt() { GlobalDLLLock()++; };

			virtual ULONG __stdcall AddRef()override {
				return ++_ThisRefCnt;
			}
			virtual ULONG __stdcall Release() override {
				auto iRet = --_ThisRefCnt;
				if (0 == iRet) delete this;
				return iRet;
			}
			virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override {
				*ppv = nullptr;
				if (IsEqualGUID(riid, __uuidof(_DeclT))) {
					*ppv = this; AddRef(); return S_OK;
				}
				else if (IsEqualGUID(riid, IID_IUnknown)) {
					*ppv = static_cast<IUnknown*>(this); AddRef(); return S_OK;
				}
				LOG(_T("Interface IID %s requested for %s"), Convert<GUID>::To<tstring>(riid).c_str(), Convert<char*>::To<tstring>(typeid(_DeclT).name()));
				return E_NOTIMPL;
			}
			static void Register(){}
			static void Unregister(){}
		protected:
			std::atomic<int> _ThisRefCnt;
		};

		template <typename _DeclT, LPCTSTR _Name, typename _HeadT, typename ... _TailT>
		struct Object <_DeclT, _Name, _HeadT, _TailT... > : _HeadT, Object < _DeclT, _Name, _TailT... > {
			using _super_t = Object < _DeclT, _Name, _TailT... > ;
			using com_object_type = Object < _DeclT, _Name, _HeadT > ;

			Object() : _super_t() {}
			~Object() {}

			virtual ULONG __stdcall AddRef() override {
				return ++_ThisRefCnt;
			}
			virtual ULONG __stdcall Release() override {
				auto iRet = --_ThisRefCnt;
				if (0 == iRet) delete this;
				return iRet;
			}
			virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override {
				HRESULT hr = E_NOTIMPL;
				*ppv = nullptr;
				if (IsEqualGUID(riid, __uuidof(_DeclT))) {
					*ppv = this; hr = S_OK; AddRef();
				}
				else if (IsEqualGUID(riid, __uuidof(_HeadT))) {
					*ppv = static_cast<_HeadT*>(this); hr = S_OK; AddRef();
				}
				else {
					hr = Object<_DeclT, _Name, _TailT...>::QueryInterface(riid, ppv);
				}
				return hr;
			}
		};
	}
}

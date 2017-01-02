/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

namespace xtd {
	namespace com {
		template <typename _ObjectT> struct ClassFactory : com::Object < ClassFactory<_ObjectT>, nullptr, IClassFactory > {
			using _super_t = com::Object < ClassFactory<_ObjectT>, nullptr, IClassFactory > ;

			virtual HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) final {
				DUMP(riid);
				if (pUnkOuter) return CLASS_E_NOAGGREGATION;
				if (!ppvObject) return E_POINTER;
				_ObjectT * pObj = new _ObjectT;
				if (!pObj) return E_OUTOFMEMORY;
				auto hr = pObj->QueryInterface(riid, ppvObject);
				if (S_OK != hr) delete pObj;
				return hr;
			}

			virtual HRESULT __stdcall LockServer(BOOL fLock) final {
				fLock ? GlobalDLLLock()++ : GlobalDLLLock()--;
				return S_OK;
			}
		};
	}
}

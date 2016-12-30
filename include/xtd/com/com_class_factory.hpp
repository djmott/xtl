#ifndef __COMCLASSFACTORY_HPP_INCLUDED__
#define __COMCLASSFACTORY_HPP_INCLUDED__

namespace xtd {
	namespace COM {
		template <typename _ObjectT> struct ClassFactory : COM::Object < ClassFactory<_ObjectT>, nullptr, IClassFactory > {
			using _super_t = COM::Object < ClassFactory<_ObjectT>, nullptr, IClassFactory > ;

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

#endif // __COMCLASSFACTORY_HPP_INCLUDED__
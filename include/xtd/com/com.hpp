#if !defined(__COM_HPP_INCLUDED__)
#define __COM_HPP_INCLUDED__

#ifndef __DEBUG_COM__
#define __DEBUG_COM__ 0
#endif

#include "COMException.hpp"
#include "COMPointer.hpp"
#include "COMDataObject.hpp"
#include "COMObject.hpp"
#include "COMClassFactory.hpp"
#include "COMDLL.hpp"

namespace xtd {
	namespace COM {
		static Concurrent::Atomic<int>& GlobalDLLLock() {
			static Concurrent::Atomic<int> _DllLock;
			return _DllLock;
		}

		template <COINIT _CoInit>
		struct Initializer {
			~Initializer() { if (S_OK == _hInit) CoUninitialize(); _hInit = S_FALSE; }
			static Initializer& Get() {
				static Initializer _Initalizer;
				return _Initalizer;
			}
			struct StaticInitializer {
				StaticInitializer() { Get(); }
			};
		private:
			Initializer() : _hInit(S_FALSE) {
				GlobalDLLLock();
				_hInit = CoInitializeEx(0, _CoInit);
			}
			HRESULT _hInit;
		};
	}
}

template <typename _SourceT>
inline xtd::COM::Exception::Exception(LPCTSTR File, int Line, LPCTSTR Function, LPCTSTR Code, HRESULT ErrorCode, _SourceT& oSource) : xtd::Windows::Exception(File, Line, Function, Code, ErrorCode) {
	COM::Pointer<ISupportErrorInfo> oSupportErrInfo;
	COM::Pointer<IErrorInfo> oErrInfo;
	if (oSource	&& SUCCEEDED(oSource->QueryInterface(__uuidof(ISupportErrorInfo), (void**)&oSupportErrInfo))
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
		_Message = xtd::format(L"%s : %s %s", (const wchar_t*)bsDescription, (const wchar_t*)bsSource, Convert<GUID>::To<std::wstring>(gErr).c_str());
		if (!!bsHelpFile) {
			_Message += xtd::format(L"See : %s(%u)", (const wchar_t*)bsHelpFile, dwHelpCtx);
		}
	}
	else {
		auto sOLEError = GetModuleErrorMessage(L"OleAut32.dll", ErrorCode);
		if (sOLEError.size()) _Message = sOLEError;
	}
}

#pragma NOTE("COM initialized during static init")
EXTERN_C SELECTANY xtd::COM::Initializer<__COM_INITIALIZER__>::StaticInitializer __ComInitializer;
LINKER_INCLUDE(__ComInitializer)

typedef HRESULT(*DllRegisterServerFunction)();
typedef HRESULT(*DllUnregisterServerFunction)();

#define DLL_REGISTER_SERVER(_fn) DLL_REGISTER_SERVER2(_fn, _CONCAT(DllRegSrvFn, __COUNTER__ ))
#define DLL_UNREGISTER_SERVER(_fn) DLL_UNREGISTER_SERVER2(_fn, _CONCAT(DllUnegSrvFn, __COUNTER__ ))

#define DLL_REGISTER_SERVER2(_fn, _name) EXTERN_C SELECTANY __declspec(allocate(".COM$DRM")) DllRegisterServerFunction _name = _fn; LINKER_INCLUDE( _name )
#define DLL_UNREGISTER_SERVER2(_fn, _name) EXTERN_C SELECTANY __declspec(allocate(".COM$DUM")) DllUnregisterServerFunction _name = _fn; LINKER_INCLUDE( _name )

#define DECLARE_COM_DLL(_dllobj) DECLARE_COM_DLL2(_dllobj, _CONCAT(_DllGetClassObject, __COUNTER__ ) , _CONCAT(_DllCanUnloadNow, __COUNTER__ ) , _CONCAT(_DllRegSrv, __COUNTER__ ), _CONCAT(_DllUnRegSrv,__COUNTER__ ) )
#define DECLARE_COM_DLL2(_dllobj, _GetClassObj, _CanUnload, _RegSrv, _UnregSrv) DECLARE_COM_DLL3(_dllobj::Initializer, _GetClassObj, _CanUnload, _RegSrv, _UnregSrv)
#define DECLARE_COM_DLL3(_dllobj, _GetClassObj, _CanUnload, _RegSrv, _UnregSrv) \
	EXTERN_C SELECTANY __declspec(allocate(".COM$GCO")) DllGetClassObjectFunction _GetClassObj = &_dllobj::DllGetClassObject; LINKER_INCLUDE( _GetClassObj ) \
	EXTERN_C SELECTANY __declspec(allocate(".COM$CUN")) DllCanUnloadNowFunction _CanUnload = &_dllobj::DllCanUnloadNow; LINKER_INCLUDE( _CanUnload ) \
	EXTERN_C SELECTANY __declspec(allocate(".COM$DRM")) DllRegisterServerFunction _RegSrv = &_dllobj::DllRegisterServer; LINKER_INCLUDE( _RegSrv ) \
	EXTERN_C SELECTANY __declspec(allocate(".COM$DUM")) DllUnregisterServerFunction _UnregSrv = &_dllobj::DllUnregisterServer; LINKER_INCLUDE( _UnregSrv )

//DllGetClassObject
#pragma section(".COM$GCA", long, read)
#pragma section(".COM$GCO", long, read)
#pragma section(".COM$GCZ", long, read)
typedef HRESULT(*DllGetClassObjectFunction)(REFCLSID rclsid, REFIID riid, LPVOID *ppv);

//DllCanUnloadNow sections
#pragma section(".COM$CUA", long, read)
#pragma section(".COM$CUN", long, read)
#pragma section(".COM$CUZ", long, read)
typedef HRESULT(*DllCanUnloadNowFunction)(void);

//DllRegisterServer sections
#pragma section(".COM$DRA", long, read)
#pragma section(".COM$DRM", long, read)
#pragma section(".COM$DRZ", long, read)

//DllUnregisterServer sections
#pragma section(".COM$DUA", long, read)
#pragma section(".COM$DUM", long, read)
#pragma section(".COM$DUZ", long, read)

#pragma comment(linker, "/merge:.COM=.const")

EXTERN_C SELECTANY __declspec(allocate(".COM$GCA"))	DllGetClassObjectFunction GetClassObjectBegin = nullptr;
EXTERN_C SELECTANY __declspec(allocate(".COM$GCZ"))	DllGetClassObjectFunction GetClassObjectEnd = nullptr;

EXTERN_C SELECTANY __declspec(allocate(".COM$CUA"))	DllCanUnloadNowFunction DllUnloadNowBegin = nullptr;
EXTERN_C SELECTANY __declspec(allocate(".COM$CUZ"))	DllCanUnloadNowFunction DllUnloadNowEnd = nullptr;

EXTERN_C SELECTANY __declspec(allocate(".COM$DRA"))	DllRegisterServerFunction DllRegisterBegin = nullptr;
EXTERN_C SELECTANY __declspec(allocate(".COM$DRZ"))	DllRegisterServerFunction DllRegisterEnd = nullptr;

EXTERN_C SELECTANY __declspec(allocate(".COM$DUA"))	DllUnregisterServerFunction DllUnregisterBegin = nullptr;
EXTERN_C SELECTANY __declspec(allocate(".COM$DUZ"))	DllUnregisterServerFunction DllUnregisterEnd = nullptr;

LINKER_INCLUDE(DllUnloadNowBegin);
LINKER_INCLUDE(DllUnloadNowEnd);

LINKER_INCLUDE(GetClassObjectBegin);
LINKER_INCLUDE(GetClassObjectEnd);

LINKER_INCLUDE(DllRegisterBegin);
LINKER_INCLUDE(DllRegisterEnd);

LINKER_INCLUDE(DllUnregisterBegin);
LINKER_INCLUDE(DllUnregisterEnd);

#pragma warning(suppress: 28251)
EXTERN_C HRESULT  __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv) {
	HRESULT hRet = CLASS_E_CLASSNOTAVAILABLE;
	for (DllGetClassObjectFunction * pFn = &GetClassObjectBegin; pFn < &GetClassObjectEnd; ++pFn) {
		if (pFn && *pFn) {
			if (S_OK != (hRet = (*pFn)(rclsid, riid, ppv))) break;
		}
	}
	return hRet;
}
LINKER_EXPORT(DllGetClassObject);

EXTERN_C  HRESULT __stdcall DllCanUnloadNow() {
	HRESULT hRet = S_OK;
	for (DllCanUnloadNowFunction * pFn = &DllUnloadNowBegin; pFn < &DllUnloadNowEnd; ++pFn) {
		if (pFn && *pFn) {
			if (S_OK != (hRet = (*pFn)())) break;
		}
	}
	return hRet;
}
LINKER_EXPORT(DllCanUnloadNow);

EXTERN_C DLLEXPORT HRESULT __stdcall DllRegisterServer() {
	HRESULT hRet = S_OK;
	std::for_each(&DllRegisterBegin, &DllRegisterEnd, [&hRet](DllUnregisterServerFunction pFN)->void { if (pFN && S_OK == hRet) { hRet = (pFN)(); }});
	return hRet;
}
LINKER_EXPORT(DllRegisterServer);

EXTERN_C DLLEXPORT HRESULT __stdcall DllUnregisterServer() {
	HRESULT hRet = S_OK;
	std::for_each(&DllUnregisterBegin, &DllUnregisterEnd, [&hRet](DllRegisterServerFunction pFN)->void {if (pFN && S_OK == hRet) { hRet = (pFN)(); }});
	return hRet;
}
LINKER_EXPORT(DllUnregisterServer);

#endif //__COM_HPP_INCLUDED__
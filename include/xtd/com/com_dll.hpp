/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/xstring.hpp>
#include <xtd/com/com_object.hpp>

namespace xtd {
	namespace com {
		enum class ThreadingModel {
			Single,
			Apartment,
			Free,
			Both,
		};

		template <ThreadingModel> struct ThreadingModelName;
		template <> struct ThreadingModelName < ThreadingModel::Single > { static tstring Value() { return __("Single"); } };
		template <> struct ThreadingModelName < ThreadingModel::Apartment > { static tstring Value() { return __("Apartment"); } };
		template <> struct ThreadingModelName < ThreadingModel::Free > { static tstring Value() { return __("Free"); } };
		template <> struct ThreadingModelName < ThreadingModel::Both > { static tstring Value() { return __("Both"); } };

		template <typename _DLLDecl, ThreadingModel _ThreadingModel, LPCTSTR _Name, typename ... _Objects> struct DLL;

		template <typename _DLLDecl, ThreadingModel _ThreadingModel, LPCTSTR _Name> struct DLL < _DLLDecl, _ThreadingModel, _Name > {

			virtual HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv) {
				DUMP(rclsid);
				DUMP(riid);
				*ppv = nullptr;
				return CLASS_E_CLASSNOTAVAILABLE;
			}

			virtual HRESULT DllCanUnloadNow() { return (0 == GlobalDLLLock().load() ? S_OK : S_FALSE); }
			virtual HRESULT DllRegisterServer() { _DLLDecl::Register(); return S_OK; }
			virtual HRESULT DllUnregisterServer() { _DLLDecl::Unregister(); return S_OK; }

			struct Initializer {
				static HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv) { return Get().DllGetClassObject(rclsid, riid, ppv); }
				static HRESULT DllCanUnloadNow() { return Get().DllCanUnloadNow(); }
				static HRESULT DllRegisterServer() { return Get().DllRegisterServer(); }
				static HRESULT DllUnregisterServer() { return Get().DllUnregisterServer(); }
			};

			static DLL& Get() { static DLL oDLL; return oDLL; }
			DLL() {}

			static void Register(){}
			static void Unregister(){}

		private:
			DLL(const DLL&) = delete;
			DLL& operator=(const DLL&) = delete;
		};

		template <typename _DLLDecl, ThreadingModel _ThreadingModel, LPCTSTR _DLLName, typename _HeadT, typename ..._TailT>
		struct DLL<_DLLDecl, _ThreadingModel, _DLLName, _HeadT, _TailT...> : DLL < _DLLDecl, _ThreadingModel, _DLLName, typename _HeadT::com_object_type, _TailT... > {};

		template <typename _DLLDecl, ThreadingModel _ThreadingModel, LPCTSTR _DLLName, typename _COMObjDecl, LPCTSTR _ObjName, typename ... _ObjInterfaces, typename ... _TailT>
		struct DLL<_DLLDecl, _ThreadingModel, _DLLName, Object<_COMObjDecl, _ObjName, _ObjInterfaces...>, _TailT...> : DLL < _DLLDecl, _ThreadingModel, _DLLName, _TailT... > {
			using _super_t = DLL < _DLLDecl, _ThreadingModel, _DLLName, _TailT... > ;

			virtual HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv) {
				try {
					DUMP(rclsid);
					DUMP(riid);
					*ppv = nullptr;
					if (IsEqualGUID(rclsid, __uuidof(_COMObjDecl))) {
						auto pFactory = new com::ClassFactory < _COMObjDecl > ;
						if (!pFactory) return E_OUTOFMEMORY;
						pFactory->AddRef();
						auto hRet = pFactory->QueryInterface(riid, ppv);
						pFactory->Release();
						return hRet;
					}
					return _super_t::DllGetClassObject(rclsid, riid, ppv);
				}
				catch (const Windows::Exception& ex) {
					CRITICAL(_T("A Windows exception 0x%08x occurred: %hs"), ex.ErrorCode(), ex.what());
					return ex.ErrorCode();
				}
			}

			virtual HRESULT DllRegisterServer() {
				try {
					auto sCLSID = Convert<GUID>::To<tstring>(__uuidof(_COMObjDecl));
					tstring sProgID = sCLSID;
					if (_DLLName) {
						sProgID = _DLLName;
						if (_ObjName) {
							sProgID += _T(".");
							sProgID += _ObjName;
						}
					}

					auto oProgKey = Windows::Registry::ClassesRoot::Get().CreateSubkey(format(_T("%s\\CLSID"), sProgID.c_str()));
					oProgKey.SetValue(_T(""), sCLSID);

					auto oCLSIDKey = Windows::Registry::ClassesRoot::Get().CreateSubkey(format(_T("CLSID\\%s"), sCLSID.c_str()));
					oCLSIDKey.SetValue(_T(""), sProgID);

					auto oInprocServerKey = oCLSIDKey.CreateSubkey(_T("InProcServer32"));
					oInprocServerKey.SetValue(_T(""), static_cast<xtd::tstring>(Windows::Path::GetBinaryPath()));
					oInprocServerKey.SetValue(_T("ThreadingModel"), ThreadingModelName<_ThreadingModel>::Value());
					_super_t::DllRegisterServer();
					_COMObjDecl::Register();
					return S_OK;
				}
				catch (const Windows::Exception& ex) {
					CRITICAL(_T("A Windows exception 0x%08x occurred: %hs"), ex.ErrorCode(), ex.what());
					return ex.ErrorCode();
				}
			}

			virtual HRESULT DllUnregisterServer() {
				try {
					auto sCLSID = Convert<GUID>::To<tstring>(__uuidof(_COMObjDecl));
					tstring sProgID = sCLSID;
					if (_DLLName) {
						sProgID = _DLLName;
						if (_ObjName) {
							sProgID += _T(".");
							sProgID += _ObjName;
						}
					}
					Windows::Registry::ClassesRoot::Get().DeleteTree(format(_T("%s"), sProgID.c_str()));
					Windows::Registry::ClassesRoot::Get().DeleteTree(format(_T("CLSID\\%s"), sCLSID.c_str()));
					_super_t::DllUnregisterServer();
					_COMObjDecl::Unregister();
					return S_OK;
				}
				catch (const Windows::Exception& ex) {
					CRITICAL(_T("A Windows exception 0x%08x occurred: %hs"), ex.ErrorCode(), ex.what());
					return ex.ErrorCode();
				}
			}

			struct Initializer {
				static HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv) { return Get().DllGetClassObject(rclsid, riid, ppv); }
				static HRESULT DllCanUnloadNow() { return Get().DllCanUnloadNow(); }
				static HRESULT DllRegisterServer() { return Get().DllRegisterServer(); }
				static HRESULT DllUnregisterServer() { return Get().DllUnregisterServer(); }
			};
			static _DLLDecl& Get() { static _DLLDecl oDecl; return oDecl; }
			DLL() {}

		private:
			DLL(const DLL&) = delete;
			DLL& operator=(const DLL&) = delete;
		};
	}
}

/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/xtd.hpp>

#include <xtd/unique_id.hpp>

namespace xtd {

	namespace com {

    MULTIPLE GUID NULL_GUID{ 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

		template <typename _ObjectT, const GUID & _pIID = __uuidof(_ObjectT), const GUID & _pCLSID = NULL_GUID> struct pointer {
			using pointer_type = _ObjectT *;
			using object_type = _ObjectT;

			~pointer() { if (_ptr) Release(); _ptr = nullptr; }
			pointer(pointer_type ptr) : _ptr(ptr) { _ptr->AddRef(); }
			pointer(pointer_type ptr, bool IncRef) : _ptr(ptr) { if (IncRef && _ptr) _ptr->AddRef(); }
			pointer(const pointer& src) : _ptr(src._ptr) { if (_ptr) _ptr->AddRef(); }
			pointer(pointer&& src) : _ptr(src._ptr) { src._ptr = nullptr; }
			pointer() : _ptr(nullptr) {}

			pointer& operator=(const pointer& src) {
				pointer(src).Swap(*this);
				return *this;
			}
			pointer& operator=(pointer&& src) {
				pointer(std::move(src)).Swap(*this);
				return *this;
			}

			void Swap(pointer& other){
				std::swap(_ptr, other._ptr);
			}

			static const GUID& IID() { return _pIID; }
			static const GUID& CLSID() { return _pCLSID; }

			operator bool() const { return (_ptr ? true : false); }

			pointer_type operator->() { return _ptr; }
			const pointer_type operator->() const { return _ptr; }

			pointer_type & operator*() { return _ptr; }
			const pointer_type & operator*() const { return _ptr; }

			pointer_type * operator &() { return &_ptr; }
			const pointer_type * operator &() const { return &_ptr; }

			void Release() { if (!_ptr) throw std::invalid_argument("COM pointer is empty"); _ptr->Release(); _ptr = nullptr; };
			void AddRef() { if (!_ptr) throw std::invalid_argument("COM pointer is empty"); _ptr->AddRef(); };

			HRESULT CreateInstance(const GUID & clsid, DWORD ctx = CLSCTX_ALL) {
				return CoCreateInstance(clsid, nullptr, ctx, _pIID, reinterpret_cast<void**>(&_ptr));
			}

			HRESULT CreateInstance(DWORD ctx = CLSCTX_ALL) {
				return CreateInstance(_pCLSID, ctx);
			}

			static pointer Create(const GUID & clsid, DWORD ctx = CLSCTX_ALL) {
				pointer oRet;
				HR_THROW_ERR(oRet, oRet.CreateInstance(clsid, ctx));
				return std::move(oRet);
			}

			static pointer Create(DWORD ctx = CLSCTX_ALL) {
				return std::move(Create(_pCLSID, ctx));
			}

			template <typename _OtherT, const GUID& otheriid, const GUID& otherclsid> HRESULT QueryInterface(pointer<_OtherT, otheriid, otherclsid>& objptr) const {
				return _ptr->QueryInterface(otheriid, (void**)&objptr);
			}

			template <typename _OtherT> _OtherT QueryInterface() const {
				_OtherT oRet;
				HR_THROW_ERR(*this, _ptr->QueryInterface(__uuidof(_OtherT::object_type), (void**)&oRet));
				return std::move(oRet);
			}

			pointer_type detach() { auto pRet = _ptr; _ptr = nullptr; return pRet; }
			pointer_type _ptr;
		};
	}
}


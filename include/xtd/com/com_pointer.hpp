#ifndef __XTD_COM_POINTER_HPP_INCLUDED__
#define __XTD_COM_POINTER_HPP_INCLUDED__

namespace xtd {
	namespace COM {
		SELECTANY GUID NULL_GUID = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

		template <typename _ObjectT, const GUID & _pIID = __uuidof(_ObjectT), const GUID & _pCLSID = NULL_GUID> struct Pointer {
			using pointer_type = _ObjectT *;
			using object_type = _ObjectT;

			~Pointer() { if (_ptr) Release(); _ptr = nullptr; }
			Pointer(pointer_type ptr) : _ptr(ptr) { _ptr->AddRef(); }
			Pointer(pointer_type ptr, bool IncRef) : _ptr(ptr) { if (IncRef && _ptr) _ptr->AddRef(); }
			Pointer(const Pointer& src) : _ptr(src._ptr) { if (_ptr) _ptr->AddRef(); }
			Pointer(Pointer&& src) : _ptr(src._ptr) { src._ptr = nullptr; }
			Pointer() : _ptr(nullptr) {}

			Pointer& operator=(const Pointer& src) {
				Pointer(src).Swap(*this);
				return *this;
			}
			Pointer& operator=(Pointer&& src) {
				Pointer(std::move(src)).Swap(*this);
				return *this;
			}

			void Swap(Pointer& other){
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
				return CoCreateInstance(clsid, nullptr, ctx, _pIID, (void**)&_ptr);
			}

			HRESULT CreateInstance(DWORD ctx = CLSCTX_ALL) {
				return CreateInstance(_pCLSID, ctx);
			}

			static Pointer Create(const GUID & clsid, DWORD ctx = CLSCTX_ALL) {
				Pointer oRet;
				HR_THROW_ERR(oRet, oRet.CreateInstance(clsid, ctx));
				return std::move(oRet);
			}

			static Pointer Create(DWORD ctx = CLSCTX_ALL) {
				return std::move(Create(_pCLSID, ctx));
			}

			template <typename _OtherT, const GUID& otheriid, const GUID& otherclsid> HRESULT QueryInterface(Pointer<_OtherT, otheriid, otherclsid>& objptr) const {
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

#endif //__XTD_COM_POINTER_HPP_INCLUDED__

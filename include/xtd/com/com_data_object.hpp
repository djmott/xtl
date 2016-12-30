#ifndef __XTD_COM_DATA_OBJECT_HPP_INCLUDED__
#define __XTD_COM_DATA_OBJECT_HPP_INCLUDED__

#if 0

namespace xtd {
	namespace COM {
		struct StorageMedium : STGMEDIUM {
			struct Pointer {
				StorageMedium * operator->() { return _ptr; }
				const StorageMedium * operator->() const { return _ptr; }
				StorageMedium & operator*() { return *_ptr; }
				const StorageMedium & operator*() const { return *_ptr; }
				StorageMedium ** operator&() { return &_ptr; }
				const StorageMedium *const* operator&() const { return &_ptr; }

				Pointer() : _ptr(nullptr) { }
				Pointer(StorageMedium * ptr) : _ptr(ptr) {}
				Pointer(Pointer&& src) : _ptr(std::move(src._ptr)) { src._ptr = nullptr; }
				~Pointer() { if (_ptr) ReleaseStgMedium(_ptr); }
				StorageMedium * _ptr;
			};
		};

		struct FormatEtc : FORMATETC {
			FormatEtc() { memset(this, 0, sizeof(FORMATETC)); }
			FormatEtc(CLIPFORMAT Format, DVASPECT Aspect, LONG iSplitIndex, TYMED StorageMedium) { cfFormat = Format; ptd = nullptr; dwAspect = Aspect; lindex = iSplitIndex; tymed = StorageMedium; }
		};

		struct FormatEnumerator : IEnumFORMATETC {
			struct Pointer : COM::Pointer < FormatEnumerator, IID_IEnumFORMATETC > {};
		};

		struct NOVTABLE DataObject : IDataObject {
			using Pointer = COM::Pointer < DataObject, IID_IDataObject > ;
			FormatEnumerator * foo();

			static Pointer From(IDataObject* pSrc) { return Pointer(static_cast<DataObject*>(pSrc)); }

			// 			FormatEnumerator::Pointer GetFormats() {
			// 				FormatEnumerator::Pointer pRet;
			// 				EnumFormatEtc(DATADIR_GET, pRet.get());
			// 				return std::move(pRet);
			// 			}
			//
			// 			FormatEnumerator::Pointer SetFormats() {
			// 				FormatEnumerator::Pointer pRet;
			// 				EnumFormatEtc(DATADIR_SET, pRet.get());
			// 				return std::move(pRet);
			// 			}

			StorageMedium::Pointer GetData(const FormatEtc&) {}
			HRESULT QueryGetData(FormatEtc& oFormat) { return IDataObject::QueryGetData(&oFormat); }
		};
	}
}

#endif

#endif //__XTD_COM_DATA_OBJECT_HPP_INCLUDED__
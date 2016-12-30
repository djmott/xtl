#ifndef __XTD_COM_EXCEPTION_HPP_INCLUDED__
#define __XTD_COM_EXCEPTION_HPP_INCLUDED__

#define HR_THROW_ERR(_obj, _exp) xtd::COM::Exception::HRThrowErr( __F_L_F__, _TQUOTE( _exp ) , ( _exp ) , _obj )

#define _HR(_obj, _fn) HR_THROW_ERR(_obj, _obj->_fn)
#define _HR2(_obj, _fn) HR_THROW_ERR(_obj, _fn)

namespace xtd {
	namespace COM {
		struct Exception : xtd::Windows::Exception {
			template <typename _SourceT>
			static HRESULT HRThrowErr(LPCTSTR File, int Line, LPCTSTR Function, LPCTSTR Code, HRESULT ErrorCode, _SourceT& oSource) {
				if (!FAILED(ErrorCode)) return ErrorCode;
				throw Exception(File, Line, Function, Code, ErrorCode, oSource);
			}

			template <typename _SourceT>
			Exception(LPCTSTR File, int Line, LPCTSTR Function, LPCTSTR Code, HRESULT ErrorCode, _SourceT& oSource);
		};
	}
}

#endif //__XTD_COM_EXCEPTION_HPP_INCLUDED__
#pragma once

namespace xtd {

	namespace _ {
		template <typename _ty> struct value_proxy;
		template <> struct value_proxy<ustring> {
			static void set(HKEY hKey, const tstring& name, const ustring& newval) {
				windows_exception::throw_lasterr_if(RegSetValueExW(hKey, name.to<ustring>().c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(newval.c_str()), sizeof(TCHAR) * static_cast<DWORD>(1 + newval.size())),
					[](LONG l) { return ERROR_SUCCESS != l; });
			}
		};

		template <> struct value_proxy<const wchar_t*> : value_proxy<ustring> {};

		template <> struct value_proxy<std::string> {
			static void set(HKEY hKey, const tstring& name, const std::string& newval) {
				windows_exception::throw_lasterr_if(RegSetValueExA(hKey, name.to<cstring>().c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(newval.c_str()), static_cast<DWORD>(1 + newval.size())),
					[](LONG l) { return ERROR_SUCCESS != l; });
			}
		};

		template <> struct value_proxy<int> {
			static void set(HKEY hKey, const tstring& name, const int& newval) {
				windows_exception::throw_lasterr_if(RegSetValueExW(hKey, name.to<ustring>().c_str(), 0, REG_DWORD, reinterpret_cast<const BYTE*>(&newval), sizeof(int)),
					[](LONG l) { return ERROR_SUCCESS != l; });
			}
		};
	}

	struct registry : std::enable_shared_from_this<registry>{
		using ptr = std::shared_ptr<registry>;
		using weak_ptr = std::weak_ptr<registry>;

		static ptr hklm() {
			static ptr oRet(new registry(HKEY_LOCAL_MACHINE));
			return oRet;
		}

		struct value_proxy {
			void Delete() const {
				assert(_parent);
				windows_exception::throw_lasterr_if(::RegDeleteValue(*_parent, _name.c_str()), [](LONG l) {return ERROR_SUCCESS != l; });
			}
			template <typename _ty> void operator=(add_clref<_ty> newval) { _::value_proxy<_ty>::set(*_parent, _name, newval); }
			template <typename _ty> void operator=(_ty newval) { _::value_proxy<_ty>::set(*_parent, _name, newval); }
			template <typename _ty> operator _ty();
			value_proxy() = delete;
			value_proxy(const value_proxy& src)  : _parent(src._parent), _name(src._name){}
			value_proxy(value_proxy&& src) : _parent(std::move(src._parent)), _name(std::move(src._name)) {}
		private:
			friend struct registry;
			value_proxy(ptr oParent, const tstring& sName) : _parent(oParent), _name(sName) {}
			template <typename _ty> void set_value(add_clref<_ty>);
			ptr _parent;
			tstring _name;
		};

		operator HKEY() const { return _key; }

		ptr CreateKey(const tstring& path) {
			HKEY hKey;
			windows_exception::throw_err(RegCreateKeyEx(_key, path.c_str(), 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &hKey, nullptr));
			return ptr(new registry(shared_from_this(), hKey));
		}
		
		value_proxy value(const tstring& Name) { return value_proxy { shared_from_this(), Name }; }

		bool KeyExists(const tstring& name) const {
			tstring sKeyName(255, 0);
			for (DWORD i = 0, lRet = RegEnumKey(_key, i, &sKeyName[0], static_cast<DWORD>(sKeyName.size()));
				ERROR_SUCCESS == lRet;
				lRet = RegEnumKey(_key, ++i, &sKeyName[0], static_cast<DWORD>(sKeyName.size())))
			{
				if (!name.compare(sKeyName.c_str())) return true;
			}
			return false;
		}

		bool ValueExists(const tstring& name) const {
			DWORD dwNameLen = 0;
			windows_exception::throw_err(RegQueryInfoKey(_key, nullptr, nullptr, 0, nullptr, nullptr, nullptr, nullptr, &dwNameLen, nullptr, nullptr, nullptr));
			LONG lRet = ERROR_SUCCESS;
			for (DWORD i = 0; ERROR_SUCCESS == lRet; ++i) {
				tstring sValueName(dwNameLen, 0);
				auto iLen = dwNameLen;
				iLen++;
				windows_exception::throw_err(lRet = RegEnumValue(_key, i, &sValueName[0], &iLen, 0, nullptr, nullptr, nullptr));
				if (!name.compare(sValueName.c_str())) return true;
			}
			return false;
		}

		void DeleteKey(const tstring& name) const { windows_exception::throw_err(RegDeleteKey(_key, name.c_str())); }

		void DeleteValue(const tstring& name) const { windows_exception::throw_err(RegDeleteValue(_key, name.c_str())); }
	private:
		registry(weak_ptr oParent, HKEY hKey) : _parent(oParent), _key(hKey){}
		registry(HKEY hKey) : _key(hKey){}
		weak_ptr _parent;
		HKEY _key;
	};

}
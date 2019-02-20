#pragma once

#include <xtd/xtd.hpp>

#include <memory>
#include <vector>

#include <xtd/xstring.hpp>
#include <xtd/exception.hpp>


namespace xtd {
	namespace windows {

		struct service_info {
			using ptr = std::shared_ptr<service_info>;
			using vector = std::vector<ptr>;
			service_info(){}
			~service_info(){}
		};

		struct service_control_manager final {
			~service_control_manager() { if (_hSCM) CloseServiceHandle(_hSCM); }
			service_control_manager(const xtd::tstring& MachineName = "", const xtd::tstring& DatabaseName = SERVICES_ACTIVE_DATABASE, DWORD Access = SC_MANAGER_ALL_ACCESS)
				: _hSCM(xtd::windows::exception::throw_if(OpenSCManager(MachineName.c_str(), DatabaseName.c_str(), Access), [](SC_HANDLE h) { return NULL == h; }))
			{}
			service_control_manager(service_control_manager&& src) : _hSCM(std::move(src._hSCM)){}
			service_control_manager& operator=(service_control_manager&& src) {
				std::swap(_hSCM, src._hSCM);
				return *this;
			}
			service_control_manager() = delete;
			service_control_manager(const service_control_manager&) = delete;
			service_control_manager& operator=(const service_control_manager&) = delete;
			operator SC_HANDLE() const { return _hSCM; }

		private:
			SC_HANDLE _hSCM;

		};
	}
}
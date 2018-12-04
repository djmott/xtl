#pragma once
#include <xtd/xtd.hpp>
#include <xtd/exception.hpp>
#include <xtd/source_location.hpp>

namespace xtd {
  namespace windows {
    struct pipe {
      using shared_ptr = std::shared_ptr<pipe>;
      using handle_type = std::unique_ptr<void, BOOL (__stdcall*)(HANDLE)>;
      ~pipe() = default;
      pipe() = delete;
      pipe(const pipe&) = delete;
      pipe& operator=(const pipe&) = delete;
      pipe(pipe&& src) : _hRead{std::move(src._hRead)}, _hWrite{std::move(src._hWrite)}{}
      pipe& operator=(pipe&& src) {
        std::swap(_hRead, src._hRead);
        std::swap(_hWrite, src._hWrite);
        return *this;
      }
      static shared_ptr create(DWORD buffer_size=0) {
        HANDLE hRead, hWrite;
        windows::exception::throw_if(CreatePipe(&hRead, &hWrite, nullptr, buffer_size), [](BOOL b) { return !b; });
        handle_type oRead(hRead,::CloseHandle);
        handle_type oWrite(hWrite, ::CloseHandle);
        return shared_ptr(new pipe(std::move(oRead), std::move(oWrite)));
      }
      template <typename _ty>
      void read(std::vector<_ty>& oRet) const {
        DWORD dwRead;
        windows::exception::throw_if(ReadFile(_hRead.get(), &oRet[0], static_cast<DWORD>(oRet.size() * sizeof(_ty)), &dwRead, nullptr), [&](BOOL b) {return !b || dwRead < (oRet.size()*sizeof(_ty)); });
      }
      void read(std::string& data, DWORD len) const {
        DWORD dwRead;
        if (data.size() < len) data.resize(len);
        windows::exception::throw_if(ReadFile(_hRead.get(), &data[0], len, &dwRead, nullptr), [&](BOOL b) {return !b || dwRead < len; });
      }
      template <typename _ty>
      void write(const std::vector<_ty>& data) const {
        DWORD dwWritten;
        windows::exception::throw_if(WriteFile(_hWrite.get(), &data[0], static_cast<DWORD>(data.size() * sizeof(_ty)), &dwWritten, nullptr), [&](BOOL b) { return !b || dwWritten < (data.size() * sizeof(_ty)); });
      }
      void write(const std::string& data) const {
        DWORD dwWritten;
        windows::exception::throw_if(WriteFile(_hWrite.get(), &data[0], static_cast<DWORD>(data.size()), &dwWritten, nullptr), [&](BOOL b) { return !b || dwWritten < data.size(); });
      }
      template <typename _ty> void write(const _ty& data) const {
        DWORD dwWritten;
        windows::exception::throw_if(WriteFile(_hWrite.get(), &data, static_cast<DWORD>(sizeof(_ty)), &dwWritten, nullptr), [&](BOOL b) { return !b || dwWritten < sizeof(_ty); });
      }
      size_t bytes_available() const {
        DWORD avail;
        windows::exception::throw_if(PeekNamedPipe(_hRead.get(), nullptr, 0, nullptr, &avail, nullptr), [](BOOL b) {return !b; });
        return avail;
      }
      template <typename _ty>
      bool peek(_ty& ret) const {
        DWORD dwRead, dwAvail;
        windows::exception::throw_if(PeekNamedPipe(_hRead.get(), &ret, sizeof(_ty), &dwRead, &dwAvail, nullptr), [](BOOL b) {return !b; });
        return dwRead==sizeof(_ty);
      }
    private:
      pipe(handle_type&& oRead, handle_type&& oWrite) : _hRead{std::move(oRead)}, _hWrite{std::move(oWrite)}{}
      handle_type _hRead;
      handle_type _hWrite;
    };
  }
}

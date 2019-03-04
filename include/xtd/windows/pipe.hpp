/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once
#include <xtd/xtd.hpp>

#include <windows.h>

#include <memory>
#include <string>
#include <vector>

#include <xtd/exception.hpp>

namespace xtd {
  namespace windows {
    struct pipe {
      using shared_ptr = std::shared_ptr<pipe>;
      using handle_type = std::unique_ptr<void, BOOL (__stdcall*)(HANDLE)>;
      ~pipe() = default;
      pipe() = delete;
      pipe(const pipe&) = delete;
      pipe& operator=(const pipe&) = delete;
      pipe(pipe&& src) : _hPipe{std::move(src._hPipe)}{}
      pipe& operator=(pipe&& src) {
        std::swap(_hPipe, src._hPipe);
        return *this;
      }
      explicit pipe(const std::string& name) : _hPipe(nullptr,::CloseHandle){
        std::string sPipe = "\\\\.\\pipe\\";
        sPipe += name;
        auto hPipe = windows::exception::throw_if(CreateNamedPipe(sPipe.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS, 4, 1024, 1024, NMPWAIT_USE_DEFAULT_WAIT, nullptr), [](HANDLE h) { return INVALID_HANDLE_VALUE == h; });
        _hPipe =  handle_type(hPipe, ::CloseHandle);
      }
      static shared_ptr create(const std::string& name) {
        return shared_ptr(new pipe(name));
      }
      static shared_ptr create(DWORD buffer_size=0) {
        HANDLE hRead, hWrite;
        windows::exception::throw_if(CreatePipe(&hRead, &hWrite, nullptr, buffer_size), [](BOOL b) { return !b; });
        handle_type oPipe(hRead,::CloseHandle);
        return shared_ptr(new pipe(std::move(oPipe)));
      }
      template <typename _ty>
      void read(std::vector<_ty>& oRet) const {
        DWORD dwRead;
        windows::exception::throw_if(ReadFile(_hPipe.get(), &oRet[0], static_cast<DWORD>(oRet.size() * sizeof(_ty)), &dwRead, nullptr), [&](BOOL b) {return !b || dwRead < (oRet.size()*sizeof(_ty)); });
      }
      void read(std::string& data, DWORD len) const {
        DWORD dwRead;
        if (data.size() < len) data.resize(len);
        windows::exception::throw_if(ReadFile(_hPipe.get(), &data[0], len, &dwRead, nullptr), [&](BOOL b) {return !b || dwRead < len; });
      }
      template <typename _ty>
      void write(const std::vector<_ty>& data) const {
        DWORD dwWritten;
        windows::exception::throw_if(WriteFile(_hPipe.get(), &data[0], static_cast<DWORD>(data.size() * sizeof(_ty)), &dwWritten, nullptr), [&](BOOL b) { return !b || dwWritten < (data.size() * sizeof(_ty)); });
      }
      void write(const std::string& data) const {
        DWORD dwWritten;
        windows::exception::throw_if(WriteFile(_hPipe.get(), &data[0], static_cast<DWORD>(data.size()), &dwWritten, nullptr), [&](BOOL b) { return !b || dwWritten < data.size(); });
      }
      template <typename _ty> void write(const _ty& data) const {
        DWORD dwWritten;
        windows::exception::throw_if(WriteFile(_hPipe.get(), &data, static_cast<DWORD>(sizeof(_ty)), &dwWritten, nullptr), [&](BOOL b) { return !b || dwWritten < sizeof(_ty); });
      }
      size_t bytes_available() const {
        DWORD avail;
        windows::exception::throw_if(PeekNamedPipe(_hPipe.get(), nullptr, 0, nullptr, &avail, nullptr), [](BOOL b) {return !b; });
        return avail;
      }
      template <typename _ty>
      bool peek(_ty& ret) const {
        DWORD dwRead, dwAvail;
        windows::exception::throw_if(PeekNamedPipe(_hPipe.get(), &ret, sizeof(_ty), &dwRead, &dwAvail, nullptr), [](BOOL b) {return !b; });
        return dwRead==sizeof(_ty);
      }
    protected:
      pipe(handle_type&& oPipe) : _hPipe{std::move(oPipe)}{}
      handle_type _hPipe;
    };

  }
}

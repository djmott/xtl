#pragma once

#include <xtd/xtd.hpp>
#include <xtd/windows/exception.hpp>

#include <cassert>
#include <vector>

namespace xtd {
  namespace windows {

    struct input_record : INPUT_RECORD {
      using vector = std::vector<input_record>;
    };


   
    struct console {

      using ptr = std::shared_ptr<console>;

      virtual ~console() noexcept { ::FreeConsole(); }

      static ptr attach() {
        static auto iSuccess = xtd::windows::exception::throw_false(::AttachConsole(::GetCurrentProcessId()));
        return _get();
      }

      static ptr alloc() {
        static auto iSuccess = xtd::windows::exception::throw_false(::AllocConsole());
        return _get();
      }

    private:

      static ptr _get() {
        static ptr oConsole(new console);
        return oConsole;
      }

      console() {}


      //input_record::vector peek_input_records() const {}
    protected:
      HANDLE _stdin;
      HANDLE _stdout;
      HANDLE _stderr;
    };


  }
}
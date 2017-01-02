/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

namespace xtd {
  namespace com {

    template <COINIT _CoInit>
    struct Initializer {
      ~Initializer() { if (S_OK == _hInit) CoUninitialize(); _hInit = S_FALSE; }
      static Initializer& Get() {
        static Initializer _Initalizer;
        return _Initalizer;
      }
      struct StaticInitializer {
        StaticInitializer() { Get(); }
      };
    private:
      Initializer() : _hInit(S_FALSE) {
        GlobalDLLLock();
        _hInit = CoInitializeEx(0, _CoInit);
      }
      HRESULT _hInit;
    };

  }
}
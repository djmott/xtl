
#pragma once

#include <memory>

namespace xtd {

  namespace _ {

  }

  template <typename _ty, typename ... _init_arg_ts> struct singleton
    static std::weak_ptr<_ty> get() {
      static auto oRet = std::make_shared<_ty>(std::forward<_init_arg_ts>(args)...);
      return oRet;
    }
  };

}
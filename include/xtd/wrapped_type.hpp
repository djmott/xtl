/** @file
* wraps a type in a class for compiler type deductive distinction
* @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

namespace xtd {

#define WRAPPED(_type) xtd::wrapped<_type, __COUNTER__>

  template <typename _ty, size_t> class wrapped {
    _ty _inner;
  public:
    using value_type = _ty;

    template <typename ... _arg_ts> wrapped(_arg_ts...oArgs) : _inner(std::forward<_arg_ts>(oArgs)...) {}

    wrapped(wrapped&& src) : _inner(std::move(src._inner)) {}

    wrapped(const wrapped& src) : _inner(src._inner) {}

    template <typename _ArgT> wrapped& operator=(_ArgT&& oArg) {
      _inner = std::move(oArg);
      return *this;
    }

    wrapped& operator=(wrapped&& src) {
      _inner = std::move(src._inner);
      return *this;
    }

    wrapped& operator=(const wrapped& src) {
      _inner = src._inner;
      return *this;
    }

    value_type& operator*() { return _inner; }
    const value_type& operator*() const { return _inner; }

    value_type* operator->() { return &_inner; }
    const value_type* operator->() const { return &_inner; }

    operator value_type() const { return _inner; }
    operator value_type&() { return _inner; }
    operator const value_type&() const { return _inner; }

    value_type& operator()() { return _inner; }
    const value_type& operator()() const { return _inner; }

  };
}
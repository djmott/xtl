/** @file
  multi-type variant using type-erasure
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#pragma once

namespace xtd {

  /** The var class implements a minimal type erasure idiom.
   *  A var is a container-wrapper for arbitrary value types similar to a ~variant~ or ~any~ type.
   */
  class var {
  public:
    ///Default constructs a var object with an underlying empty object
    var() : _inner(new empty) {}

    var(const var& src) : _inner(src._inner->clone()) {}

    explicit template <typename _Ty> var(_Ty src) : _inner(new inner<_Ty>(src)) {}

    template <typename _Ty> var& operator = (_Ty src) {
      var oTmp(std::forward<_Ty>(src));
      std::swap(oTmp, *this);
      return *this;
    }

    var& operator=(const var& src) {
      if (&src == this) return *this;
      _inner.reset(src._inner->clone());
      return *this;
    }

    const std::type_info& get_type() const { return _inner->get_type(); }

    bool is_pod() const { return _inner->is_pod(); }

    size_t size() const { return _inner->size(); }

    template <typename _Ty> _Ty& as() {
      return *dynamic_cast<inner<_Ty>&>(*_inner);
    }

    template <typename _Ty> const _Ty& as() const {
      return *dynamic_cast<inner<_Ty>&>(*_inner);
    }

    class inner_base {
    public:
      using ptr = std::unique_ptr < inner_base >;
      virtual ~inner_base() = default;
      virtual inner_base * clone() const = 0;
      virtual const std::type_info& get_type() const = 0;
      virtual bool is_pod() const = 0;
      virtual size_t size() const = 0;
      inner_base& operator=(const inner_base&) = delete;
      inner_base(const inner_base&) = delete;
      inner_base() = default;
    };

    class empty : public inner_base{
    public:
      ~empty() override = default;
      inner_base * clone() const override { return new empty; }
      const std::type_info& get_type() const override { return typeid(empty); }
      bool is_pod() const override { throw std::runtime_error("reference to uninitialized variable"); }
      size_t size() const override { throw std::runtime_error("reference to uninitialized variable"); }
    };

    template <typename _Ty> class inner : public inner_base {
    public:
      explicit inner(_Ty newval) : _value(newval) {}
      ~inner() override = default;
      inner(const inner&)=delete;
      inner() = delete;
      inner& operator=(const inner&) = delete;
      inner_base * clone() const override { return new inner(_value); }
      const std::type_info& get_type() const override { return typeid(_Ty); }
      _Ty & operator * () { return _value; }
      const _Ty & operator * () const { return _value; }
      bool is_pod() const override { return std::is_pod<_Ty>::value; }
      size_t size() const override { return sizeof(_Ty); }
    private:
      _Ty _value;
    };

    inner_base::ptr _inner;
  };

  template <typename _ChT>
  class var::inner<xtd::xstring<_ChT>> : public var::inner_base {
  public:
    explicit inner(xtd::xstring<_ChT> newval) : _value(std::move(newval)) {}
    ~inner() override = default;
    inner(const inner&) = delete;
    inner() = delete;
    inner& operator=(const inner&) = delete;
    inner_base * clone() const override { return new inner(_value); }
    const std::type_info& get_type() const override { return typeid(xtd::xstring<_ChT>); }
    xtd::xstring<_ChT> & operator * () { return _value; }
    const std::basic_string<_ChT> & operator * () const { return _value; }
    bool is_pod() const override { return false; }
    size_t size() const override { return _value.size(); }
  private:
    xtd::xstring<_ChT> _value;
  };

}

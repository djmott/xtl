/** @file
  @brief Multi-type variant using type-erasure
  
  Provides a type-erased container similar to std::variant or std::any that can hold
  values of arbitrary types at runtime.
  
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/xtd.hpp>

#include <memory>

#include <xtd/string.hpp>

namespace xtd {

  /** @brief Type-erased value container
   * 
   * The var class implements a minimal type erasure idiom. A var is a container-wrapper 
   * for arbitrary value types similar to std::variant or std::any. It allows storing 
   * values of different types at runtime and retrieving them with type safety.
   * 
   * @note Unlike std::any, var uses dynamic_cast for type checking which may have
   * different performance characteristics.
   */
  class var {
  public:
    /** @brief Default constructs a var object with an underlying empty object
     * 
     * Creates a var instance that does not contain any value. Attempting to access
     * the value will result in an exception.
     */
    var() : _inner(new empty) {}

    var(const var& src) : _inner(src._inner->clone()) {}

    template <typename _ty> var(_ty src) : _inner(new inner<_ty>(src)) {}

    template <typename _ty> var& operator = (_ty src) {
      var oTmp(std::forward<_ty>(src));
      std::swap(oTmp, *this);
      return *this;
    }

    var& operator=(const var& src) {
      if (&src == this) return *this;
      _inner.reset(src._inner->clone());
      return *this;
    }

    /** @brief Gets the type information of the stored value
     * @return Reference to std::type_info for the stored type
     */
    const std::type_info& get_type() const { return _inner->get_type(); }

    /** @brief Checks if the stored value is a POD (Plain Old Data) type
     * @return true if the stored type is trivial and standard layout
     * @throws std::runtime_error if var is empty
     */
    bool is_pod() const { return _inner->is_pod(); }

    /** @brief Gets the size of the stored value
     * @return Size in bytes of the stored type
     * @throws std::runtime_error if var is empty
     */
    size_t size() const { return _inner->size(); }

    /** @brief Retrieves the stored value with type checking
     * @tparam _ty The expected type of the stored value
     * @return Reference to the stored value of type _ty
     * @throws std::bad_cast if the stored type does not match _ty
     */
    template <typename _ty> _ty& as() {
      return *dynamic_cast<inner<_ty>&>(*_inner);
    }

    /** @brief Retrieves the stored value with type checking (const version)
     * @tparam _ty The expected type of the stored value
     * @return Const reference to the stored value of type _ty
     * @throws std::bad_cast if the stored type does not match _ty
     */
    template <typename _ty> const _ty& as() const {
      return *dynamic_cast<inner<_ty>&>(*_inner);
    }

    /** @brief Base class for type-erased value storage
     * @internal
     */
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

    /** @brief Represents an empty var (no stored value)
     * @internal
     */
    class empty : public inner_base{
    public:
      ~empty() override = default;
      inner_base * clone() const override { return new empty; }
      const std::type_info& get_type() const override { return typeid(empty); }
      bool is_pod() const override { throw std::runtime_error("reference to uninitialized variable"); }
      size_t size() const override { throw std::runtime_error("reference to uninitialized variable"); }
    };

    /** @brief Type-specific storage for a value in var
     * @tparam _ty The type of value to store
     * @internal
     */
    template <typename _ty> class inner : public inner_base {
    public:
      explicit inner(_ty newval) : _value(newval) {}
      ~inner() override = default;
      inner(const inner&)=delete;
      inner() = delete;
      inner& operator=(const inner&) = delete;
      inner_base * clone() const override { return new inner(_value); }
      const std::type_info& get_type() const override { return typeid(_ty); }
      _ty & operator * () { return _value; }
      const _ty & operator * () const { return _value; }
      bool is_pod() const override { return std::is_trivial_v<_ty> && std::is_standard_layout_v<_ty>; }
      size_t size() const override { return sizeof(_ty); }
    private:
      _ty _value;
    };

    inner_base::ptr _inner;
  };

  template <typename _ch_t>
  class var::inner<xtd::xstring<_ch_t>> : public var::inner_base {
  public:
    explicit inner(xtd::xstring<_ch_t> newval) : _value(std::move(newval)) {}
    ~inner() override = default;
    inner(const inner&) = delete;
    inner() = delete;
    inner& operator=(const inner&) = delete;
    inner_base * clone() const override { return new inner(_value); }
    const std::type_info& get_type() const override { return typeid(xtd::xstring<_ch_t>); }
    xtd::xstring<_ch_t> & operator * () { return _value; }
    const std::basic_string<_ch_t> & operator * () const { return _value; }
    bool is_pod() const override { return false; }
    size_t size() const override { return _value.size(); }
  private:
    xtd::xstring<_ch_t> _value;
  };

}

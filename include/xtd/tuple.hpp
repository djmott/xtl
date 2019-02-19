/** @file
Tuple manipulation
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once
#include <xtd/xtd.hpp>
#include <memory>
#include <cstdint>
namespace xtd {

  /*
  @defgroup Tuple
  @{
  Generic tuple declaration
  */
  template <typename...> class tuple;
  /// @}

#if (!DOXY_INVOKED)
  namespace _ {
//tuple_item
    template <typename _Ty, typename ...> class tuple_item;
    template <typename _Ty> class tuple_item<_Ty>{};

    template <typename _Ty, typename ... _Ts> class tuple_item<_Ty, _Ty, _Ts...> {
    public:
      static _Ty& get(tuple<_Ty, _Ts...>& oTuple) { return oTuple.Value; }
    };
    template <typename _Ty, typename _HeadT, typename ... _TailT> class tuple_item<_Ty, _HeadT, _TailT...> {
    public:
      static _Ty& get(tuple<_HeadT, _TailT...>& oTuple) { return tuple_item<_Ty, _TailT...>::get(static_cast<tuple<_TailT...>&>(oTuple)); }
    };

//tuple_index
    template <int, typename ...> class tuple_index;
    template <typename _HeadT, typename..._TailT> class tuple_index<0, _HeadT, _TailT...> {
    public:
      using value_type = _HeadT;
      static value_type& get(tuple<_HeadT, _TailT...>& oTuple) { return oTuple.Value; }
    };
    template <int index, typename _HeadT, typename..._TailT> class tuple_index<index, _HeadT, _TailT...> {
    public:
      using value_type = typename tuple_index<index-1, _TailT...>::value_type;
      static value_type& get(tuple<_HeadT, _TailT...>& oTuple) { return tuple_index<index - 1, _TailT...>::get(static_cast<tuple<_TailT...>&>(oTuple)); }
    };

//tuple_append
    template <typename, typename> class tuple_append;
    template <typename _Ty, typename ... _TupleTs> class tuple_append<_Ty, xtd::tuple<_TupleTs...>> {
    public:
      using tuple_type = xtd::tuple<_TupleTs..., _Ty>;
    };
    template <typename ... _AppendTs, typename ... _TupleTs> class tuple_append<xtd::tuple<_AppendTs...>, xtd::tuple<_TupleTs...>> {
    public:
      using tuple_type = xtd::tuple<_TupleTs..., _AppendTs...>;
    };

//tuple_prepend
    template <typename, typename> class tuple_prepend;
    template <typename _Ty, typename ... _TupleTs> class tuple_prepend<_Ty, xtd::tuple<_TupleTs...>> {
    public:
      using tuple_type = xtd::tuple<_Ty, _TupleTs...>;
    };
    template <typename ... _PrependTs, typename ... _TupleTs> class tuple_prepend<xtd::tuple<_PrependTs...>, xtd::tuple<_TupleTs...>> {
    public:
      using tuple_type = xtd::tuple<_PrependTs..., _TupleTs...>;
    };

//tuple_index_of
    template <typename, typename...> class tuple_index_of;
    template <typename _Ty, typename..._TailT> class tuple_index_of<_Ty, _Ty, _TailT...> {
    public:
      static constexpr int value = 0;
    };
    template <typename _Ty, typename _HeadT, typename..._TailT> class tuple_index_of<_Ty, _HeadT, _TailT...> {
    public:
      static constexpr int value = 1 + tuple_index_of<_Ty, _TailT...>::value;
    };
  }
#endif

  /*
  @addtogroup Tuple
  @{
  Generic tuple declaration
  */
  /// Tuple specialization with no elements
  template <> class tuple<> {
  public:
    static constexpr size_t Length = 0;
    tuple(){}
    tuple(const tuple&){}
    tuple(tuple&&){}
  };

  /// Tuple specialization with one or more elements
  template <typename _HeadT, typename ... _TailT> class tuple<_HeadT, _TailT...> : public tuple<_TailT...>{
  public:
    using _self_t = tuple<_HeadT, _TailT...>;
    using _super_t = tuple<_TailT...>;
    using value_type = _HeadT;
    static constexpr size_t Length = 1 + _super_t::Length;

    /** Gets an item from the tuple by type
    @tparam _Ty the type to retrieve
    @returns the first type of _Ty in the tuple
    */
    template <typename _Ty> _Ty& Item() { return _::tuple_item<_Ty, _HeadT, _TailT...>::get(*this); }
    /// Gets an item from the tuple by index
    template <int index> typename _::tuple_index<index, _HeadT, _TailT...>::value_type& Index() { return _::tuple_index<index, _HeadT, _TailT...>::get(*this); }
    /// Appends a type to the tuple
    template <typename _Ty> using Append = typename _::tuple_append<_Ty, tuple<_HeadT, _TailT...>>::tuple_type;
    /// Prepends a type to the tuple
    template <typename _Ty> using Prepend = typename _::tuple_prepend <_Ty, tuple<_HeadT, _TailT...>>::tuple_type;
    /// Gets the index of a specified type
    template <typename _Ty> using index_of = _::tuple_index_of<_Ty, _HeadT, _TailT...>;

    tuple() : _super_t() {}
    tuple(tuple&& src) : _super_t(std::move(src)), Value(std::move(src.value)) {}
    tuple(const tuple& src) : _super_t(src), Value(src.Value) {}

    value_type Value;
  };
///@}
}



/** @file
manipulate and generate tuples
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

namespace xtd {

  template <typename...> struct tuple;


  namespace _ {
//tuple_item
    template <typename _Ty, typename ...> struct tuple_item;
    template <typename _Ty> struct tuple_item<_Ty>{
// 			static _Ty& get(tuple<>&);
    };

    template <typename _Ty, typename ... _Ts> struct tuple_item<_Ty, _Ty, _Ts...> {
      static _Ty& get(tuple<_Ty, _Ts...>& oTuple) { return oTuple.Value; }
    };
    template <typename _Ty, typename _HeadT, typename ... _TailT> struct tuple_item<_Ty, _HeadT, _TailT...> {
      static _Ty& get(tuple<_HeadT, _TailT...>& oTuple) { return tuple_item<_Ty, _TailT...>::get(static_cast<tuple<_TailT...>&>(oTuple)); }
    };

//tuple_index
    template <int, typename ...> struct tuple_index;
    template <typename _HeadT, typename..._TailT> struct tuple_index<0, _HeadT, _TailT...> {
      using value_type = _HeadT;
      static value_type& get(tuple<_HeadT, _TailT...>& oTuple) { return oTuple.Value; }
    };
    template <int index, typename _HeadT, typename..._TailT> struct tuple_index<index, _HeadT, _TailT...> {
      using value_type = typename tuple_index<index-1, _TailT...>::value_type;
      static value_type& get(tuple<_HeadT, _TailT...>& oTuple) { return tuple_index<index - 1, _TailT...>::get(static_cast<tuple<_TailT...>&>(oTuple)); }
    };

//tuple_append
    template <typename, typename> struct tuple_append;
    template <typename _Ty, typename ... _TupleTs> struct tuple_append<_Ty, xtd::tuple<_TupleTs...>> {
      using tuple_type = xtd::tuple<_TupleTs..., _Ty>;
    };
    template <typename ... _AppendTs, typename ... _TupleTs> struct tuple_append<xtd::tuple<_AppendTs...>, xtd::tuple<_TupleTs...>> {
      using tuple_type = xtd::tuple<_TupleTs..., _AppendTs...>;
    };

//tuple_prepend
    template <typename, typename> struct tuple_prepend;
    template <typename _Ty, typename ... _TupleTs> struct tuple_prepend<_Ty, xtd::tuple<_TupleTs...>> {
      using tuple_type = xtd::tuple<_Ty, _TupleTs...>;
    };
    template <typename ... _PrependTs, typename ... _TupleTs> struct tuple_prepend<xtd::tuple<_PrependTs...>, xtd::tuple<_TupleTs...>> {
      using tuple_type = xtd::tuple<_PrependTs..., _TupleTs...>;
    };

//tuple_index_of
    template <typename, typename...> struct tuple_index_of;
    template <typename _Ty, typename..._TailT> struct tuple_index_of<_Ty, _Ty, _TailT...> {
      static const int value = 0;
    };
    template <typename _Ty, typename _HeadT, typename..._TailT> struct tuple_index_of<_Ty, _HeadT, _TailT...> {
      static const int value = 1 + tuple_index_of<_Ty, _TailT...>::value;
    };
  }

  template <> struct tuple<> {
    static const size_t Length = 0;
    tuple(){}
    tuple(const tuple&){}
    tuple(tuple&&){}
  };

  template <typename _HeadT, typename ... _TailT> struct tuple<_HeadT, _TailT...> : tuple<_TailT...>{
    using _self_t = tuple<_HeadT, _TailT...>;
    using _super_t = tuple<_TailT...>;
    using value_type = _HeadT;
    static const size_t Length = 1 + _super_t::Length;

    template <typename _Ty> _Ty& Item() { return _::tuple_item<_Ty, _HeadT, _TailT...>::get(*this); }

    template <int index> typename _::tuple_index<index, _HeadT, _TailT...>::value_type& Index() { return _::tuple_index<index, _HeadT, _TailT...>::get(*this); }

    template <typename _Ty> using Append = typename _::tuple_append<_Ty, tuple<_HeadT, _TailT...>>::tuple_type;
    template <typename _Ty> using Prepend = typename _::tuple_prepend <_Ty, tuple<_HeadT, _TailT...>>::tuple_type;

    template <typename _Ty> using index_of = _::tuple_index_of<_Ty, _HeadT, _TailT...>;

    tuple() : _super_t() {}
    tuple(tuple&& src) : _super_t(std::move(src)), Value(std::move(src.value)) {}
    tuple(const tuple& src) : _super_t(src), Value(src.Value) {}

    value_type Value;

  };


}

#if !defined(__TEST_TUPLE__)
#define __TEST_TUPLE__ 0
#endif

#if __TEST_TUPLE__

int main() {
using t1 = xtd::tuple<int, short>;
using t2 = t1::Append<char*>;
DUMP(typeid(t2));
XTD_ASSERT(typeid(t2) == typeid(xtd::tuple<int, short, char*>));
using t3 = xtd::tuple<long, double>;
using t4 = t3::Prepend<float>;
DUMP(typeid(t4));
XTD_ASSERT(typeid(t4) == typeid(xtd::tuple<float, long, double>));
using t5 = t4::Append<t2>;
DUMP(typeid(t5));
XTD_ASSERT(typeid(t5) == typeid(xtd::tuple<float, long, double, int, short, char *>));

}


#endif
/** @file
concurrently push and pop items from a FIFO queue
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#if 0
#pragma once

namespace xtd{
  namespace concurrent{
    template <typename _Ty> class queue{
    public:
      using value_type = _Ty;

      queue() : _head(nullptr), _tail(nullptr){}
      queue(const queue&) = delete;
      queue& operator=(const queue&) = delete;
      ~queue(){
        while(auto oNode = _head.load()){
          _head.store(oNode->_next);
          delete oNode;
        }
      }

      void push(const value_type& src){
        auto oNode = new node(src);
        do{
          oNode->_next = _head.load();
        }while (!_head.compare_exchange_strong(&oNode, oNode->_next));
        node::pointer pNull = nullptr;
        _tail.compare_exchange_strong(&pNull, oNode);
      }
      bool try_pop(value_type& ret){
        do{
          auto oNode = _head.load();
          if (!oNode) return false;

        }while(!_head.)
      }
      value_type pop(){
        auto oNode = _head.load();
        if (!oNode){

        }
      }

    protected:

      struct node{
        using pointer = node *;
        using atomic_ptr = std::atomic<pointer>;
        value_type _value;
        node * _next;
        node(const value_type& data) : _data(data), _next(nullptr){}
        node(value_type&& data) : _data(std::move(data)), _next(nullptr){}
      };

      node::pointer pop_head(){
        auto ori = _head.load();
        if (ori == _tail.load()){
          return nullptr;
        }
        _head.store(ori->_next);
        return ori;
      }

      node::atomic_ptr _head;
      node::atomic_ptr _tail;
    };
  }
}

#endif

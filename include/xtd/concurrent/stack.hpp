/** @file
concurrently push and pop items in a FILO stack
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once
#include <xtd/concurrent/concurrent.hpp>

#include <atomic>

namespace xtd{
 
  namespace concurrent{
    /** @addtogroup Concurrent
    @{*/
    /** A lock-free LIFO stack
    multiple threads can push and pop items concurrently
    @tparam _value_t type of value contained in the stack. Must be copy constructible.
    */
    template <typename _value_t, typename _wait_policy_t = null_wait_policy> class stack{
    public:

      using value_type = _value_t;
      using wait_policy_type = _wait_policy_t;

      stack(wait_policy_type oWait = wait_policy_type()) : _root(nullptr), _wait_policy(oWait){}

      ~stack(){
        while (_root.load()){
          auto pNode = _root.load();
          if (_root.compare_exchange_strong(pNode, pNode->_next)){
            delete pNode;
          }
          _wait_policy();
        }
      }
      stack(stack&& src) : _root(src._root.load()){
        src._root.store(nullptr);
      }
      stack& operator=(stack&& src){
        if (&src == this) return *this;
        std::swap(_root, src._root);
      }

      stack(const stack&) = delete;

      stack& operator=(const stack&) = delete;

      bool try_pop(value_type& oRet){        
        auto oTmp = _root.load();
        if (!oTmp) return false;
        if (!_root.compare_exchange_strong(oTmp, oTmp->_next)){
          return false;
        }
        oRet = oTmp->_value;
        delete oTmp;
        return true;
      }
      
      void push(const value_type& value){
        typename node::pointer pNode = new node(nullptr, value);
        forever{
          pNode->_next = _root.load();
          if (_root.compare_exchange_strong(pNode->_next, pNode)){
            break;
          }
          _wait_policy();
        }
      }

      value_type pop(){
        value_type oRet;
        while (!try_pop(oRet)){
          _wait_policy();
        }
        return oRet;
      }

    private:
      struct node{
        using pointer = node *;
        using atomic_ptr = std::atomic<pointer>;
        node() = delete;
        node(const node&) = delete;
        node(node * pNext, const value_type& value) : _next(pNext), _value(value){}
        node(node&&)=delete;
        node& operator=(const node&) = delete;
        node& operator=(node&&)=delete;
        node * _next;
        value_type _value;
      };
      typename node::atomic_ptr _root;
      _wait_policy_t _wait_policy;
    };
    ///@}
  }
}

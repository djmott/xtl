/** @file
concurrently push and pop items in a FILO stack
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once
#include <xtd/xtd.hpp>

namespace xtd{
 
  namespace concurrent{
    /** @addtogroup Concurrent
    @{*/
    /** A lock-free LIFO stack
    multiple threads can push and pop items concurrently
    @tparam _ValueT type of value contained in the stack. Must be copy constructible.
    */
    template <typename _ValueT, typename _WaitPolicyT = null_wait_policy> class stack{
    public:

      using value_type = _ValueT;
      using wait_policy_type = _WaitPolicyT;

      stack(wait_policy_type oWait = wait_policy_type()) : _Root(nullptr), _WaitPolicy(oWait){}

      ~stack(){
        while (_Root.load()){
          auto pNode = _Root.load();
          if (_Root.compare_exchange_strong(pNode, pNode->_Next)){
            delete pNode;
          }
          _WaitPolicy();
        }
      }
      stack(stack&& src) : _Root(src._Root.load()){
        src._Root.store(nullptr);
      }
      stack& operator=(stack&& src){
        if (&src == this) return *this;
        std::swap(_Root, src._Root);

      }

      stack(const stack&) = delete;
      stack& operator=(const stack&) = delete;

      bool try_pop(value_type& oRet){        
        auto oTmp = _Root.load();
        if (!oTmp) return false;
        if (!_Root.compare_exchange_strong(oTmp, oTmp->_Next)){
          return false;
        }
        oRet = oTmp->_Value;
        delete oTmp;
        return true;
      }
      
      void push(const value_type& value){
        typename node::pointer pNode = new node;
        pNode->_Value = value;
        forever{
          pNode->_Next = _Root.load();
          if (_Root.compare_exchange_strong(pNode->_Next, pNode)){
            break;
          }
          _WaitPolicy();
        }
      }

      value_type pop(){
        value_type oRet;
        while (!try_pop(oRet)){
          _WaitPolicy();
        }
        return oRet;
      }

    private:
      struct node{
        using pointer = node *;
        using atomic_ptr = std::atomic<pointer>;
        node * _Next;
        value_type _Value;
      };
      typename node::atomic_ptr _Root;
      _WaitPolicyT _WaitPolicy;
    };
    ///@}
  }
}

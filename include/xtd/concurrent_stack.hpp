/** @file
concurrently push and pop items in a FILO stack
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

namespace xtd{
 
  namespace concurrent{
    /** @addtogroup Concurrent
    @{*/
    /** A lock-free LIFO stack
    multiple threads can push and pop items concurrently
    @tparam _ValueT type of value contained in the stack. Must be copy constructible.
    */
    template <typename _ValueT> class stack{
    public:
      using value_type = _ValueT;

      stack() : _Root(nullptr){}
      ~stack(){
        while (_Root.load()){
          auto pNode = _Root.load();
          if (_Root.compare_exchange_strong(pNode, pNode->_Next)){
            delete pNode;
          }
        }
      }
      stack(stack&& src) : _Root(src._Root.load()){
        src._Root.store(nullptr);
      }
      stack& operator=(stack&& src){
        if (&src == this) return *this;
        node * pNull = nullptr;
        node * pSrcRoot;
        std::swap(_Root, src._Root);

      }

      stack(const stack&) = delete;
      stack& operator=(const stack&) = delete;
      

    protected:
      struct node{
        node * _Next;
        value_type _Value;
      };
      std::atomic<node*> _Root;
    };
    ///@}
  }
}

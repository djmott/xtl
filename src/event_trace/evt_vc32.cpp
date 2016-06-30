/** @file
 * \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <intrin.h>
#pragma intrinsic(_ReturnAddress)



extern "C"{

  void __xtd_EventEnter(void * addr);
  void __xtd_EventLeave(void * addr);

  void __declspec(naked) _cdecl _penter(){
    __asm {
      PUSH EBP
      MOV EBP, ESP
      SUB ESP, __LOCAL_SIZE
      PUSHAD
    }
    __xtd_EventEnter((void*)((uint32_t)_ReturnAddress() - 5));
    __asm {
      POPAD
      MOV ESP, EBP
      POP EBP
      RET
    }
  }

  void __declspec(naked) _cdecl _pexit(){
    __asm {
      PUSH EBP
      MOV EBP, ESP
      SUB ESP, __LOCAL_SIZE
      PUSHAD
    }
    __xtd_EventLeave((void*)((unsigned long long)_ReturnAddress() - 5));
    __asm {
      POPAD
      MOV ESP, EBP
      POP EBP
      RET
    }
  }
}

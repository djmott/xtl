; @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
extrn __xtd_EventEnter:proc
extrn __xtd_EventLeave:proc

.code

_penter proc 
  PUSH RBP
  MOV RBP, RSP
  PUSH RAX
  PUSH RCX
  PUSH RDX
  PUSH R8
  PUSH R9
  PUSH R10
  PUSH R11

  SUB  RSP,20H

  MOV  RCX,RBP
  SUB  RCX,5

  CALL __xtd_EventEnter

  ADD  RSP,20H

  POP R11
  POP R10
  POP R9
  POP R8
  POP RDX
  POP RCX
  POP RAX
  POP RBP
  RET
_penter ENDP


_pexit PROC
  PUSH RBP
  MOV RBP, RSP
  PUSH RAX
  PUSH RCX
  PUSH RDX
  PUSH R8
  PUSH R9
  PUSH R10
  PUSH R11

  ; RESERVE SPACE FOR 4 REGISTERS [ RCX,RDX,R8 AND R9 ] 32 BYTES
  SUB  RSP,20H 
  
  ; GET THE RETURN ADDRESS OF THE FUNCTION
  MOV  RCX,RSP
  MOV  RCX,QWORD PTR[RCX+58H]

  CALL __xtd_EventLeave

  ;RELEASE THE SPACE RESERVED FOR THE REGISTERSK BY ADDING 32 BYTES
  ADD RSP,20H

  POP R11
  POP R10
  POP R9
  POP R8
  POP RDX
  POP RCX
  POP RAX
  POP RBP
  RET
_pexit ENDP


END

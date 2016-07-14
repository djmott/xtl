/** @file
data conversion utilities
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#pragma once

namespace xtd{

  /** @addtogroup Annotation
  @{*/
  template <typename _Ty>
  struct nibble_mask{
    static const uint64_t lonibble_mask = 0x0f;
    static const uint64_t hinibble_mask = 0xf0;
    static constexpr uint8_t lonibble(typename std::enable_if<std::is_pod<_Ty>::value, const _Ty&>::type src){
      return *reinterpret_cast<const uint8_t*>(&src) & lonibble_mask;
    }
    static constexpr uint8_t hinibble(typename std::enable_if<std::is_pod<_Ty>::value, const _Ty&>::type src){
      return (*reinterpret_cast<const uint8_t*>(&src) & hinibble_mask) >> 4;
    }
  };
  template <typename _Ty>
  struct byte_mask : nibble_mask<_Ty>{
    static const uint64_t lobyte_mask = 0xff;
    static const uint64_t hibyte_mask = 0xff00;
    static constexpr uint8_t lobyte(typename std::enable_if<std::is_pod<_Ty>::value, const _Ty&>::type src){
      return reinterpret_cast<const uint8_t*>(&src)[0];
    }
    static constexpr uint8_t hibyte(typename std::enable_if<std::is_pod<_Ty>::value, const _Ty&>::type src){
      return reinterpret_cast<const uint8_t*>(&src)[1];
    }
  };
  template <typename _Ty>
  struct word_mask : byte_mask<_Ty>{
    static const uint64_t loword_mask = 0xffff;
    static const uint64_t hiword_mask = 0xffff0000;
    static constexpr uint8_t loword(typename std::enable_if<std::is_pod<_Ty>::value, const _Ty&>::type src){
      return reinterpret_cast<const uint16_t*>(&src)[0];
    }
    static constexpr uint8_t hinibble(typename std::enable_if<std::is_pod<_Ty>::value, const _Ty&>::type src){
      return reinterpret_cast<const uint16_t*>(&src)[1];
    }
  };
  template <typename _Ty>
  struct int_mask : word_mask<_Ty>{
    static const uint64_t loint_mask = 0xffffffff;
    static const uint64_t hiint_mask = 0xffffffff00000000;
    static constexpr uint8_t loint(typename std::enable_if<std::is_pod<_Ty>::value, const _Ty&>::type src){
      return reinterpret_cast<const uint32_t*>(&src)[0];
    }
    static constexpr uint8_t hiint(typename std::enable_if<std::is_pod<_Ty>::value, const _Ty&>::type src){
      return reinterpret_cast<const uint32_t*>(&src)[1];
    }
  };

  template <typename _Ty> struct mask
    : std::conditional< sizeof(_Ty) >= sizeof(int64_t), int_mask<_Ty>,
      std::conditional<sizeof(_Ty) >= sizeof(uint32_t), word_mask<_Ty>,
        std::conditional<sizeof(_Ty) >= sizeof(uint16_t), byte_mask<_Ty>, nibble_mask<_Ty>>>>::type
  {
  };

  template <typename _Ty> struct BitsPer{
    static const int value = sizeof(_Ty) * 8;
  };
  ///@}
}

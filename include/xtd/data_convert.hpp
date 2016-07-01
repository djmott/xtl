/** @file
 data conversion utilities
  \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#pragma once

namespace xtd{

  struct Mask{
    static const uint64_t lonibble = 0x0f;
    static const uint64_t hinibble = 0xf0;
  };

  /**
   \fn  template <typename _Ty> static constexpr uint8_t lonibble(_Ty src)
  
   \brief lonibble returns the low order nibble of a POD variable
  
 
   \tparam  _Ty type of the ty.
   @param src Source for the.
  
   \return  The low order nibble of src
   */

  template <typename _Ty>
  static constexpr uint8_t lonibble(_Ty src){
    static_assert(std::is_pod<_Ty>::value, "invalid parameter");
    return *reinterpret_cast<const uint8_t*>(&src) & Mask::lonibble;
  }

  /**
 \struct  BitsPer

 \brief The bits per.

 \tparam  _Ty type of the ty.
 */

  template <typename _Ty> struct BitsPer{
    /** \brief The value. */
    static const int value = sizeof(_Ty) * 8;
  };

  /**
   \fn  template <typename _Ty> static constexpr uint8_t hinibble(_Ty src)
  
   \brief hinibble returns the high order nibble of a POD variable

   \tparam  _Ty type of the ty.
   @param src Source for the.
  
   \return  An uint8_t.
   */

  template <typename _Ty>
  static constexpr uint8_t hinibble(_Ty src){
    static_assert(std::is_pod<_Ty>::value, "invalid parameter");
    return (*reinterpret_cast<const uint8_t *>(&src) & Mask::hinibble) >> (BitsPer<char>::value/2);
  }

  /**
   \fn  template <typename _Ty> static constexpr uint8_t lobyte(_Ty src)
  
   \brief Lobytes the given source.

   \tparam  _Ty type of the ty.
   @param src Source for the.
  
   \return  An uint8_t.
   */

  template <typename _Ty>
  static constexpr uint8_t lobyte(_Ty src){
    static_assert(std::is_pod<_Ty>::value, "invalid parameter");
    return *reinterpret_cast<const uint8_t *>(&src);
  }

  template <typename _Ty>
  static inline constexpr uint8_t hibyte(_Ty src){
    static_assert(std::is_pod<_Ty>::value, "invalid parameter");
    return reinterpret_cast<const uint8_t *>(&src)[1];
  }

  /**
   \fn  template <typename _Ty> constexpr uint16_t loword(_Ty src)
  
   \brief Lowords the given source.
  

   \tparam  _Ty type of the ty.
   @param src Source for the.
  
   \return  An uint16_t.
   */

  template <typename _Ty>
  constexpr uint16_t loword(_Ty src){
    static_assert(std::is_pod<_Ty>::value, "invalid parameter");
    return reinterpret_cast<const uint16_t *>(&src)[0];
  }

  /**
   \fn  template <typename _Ty> constexpr uint16_t hiword(_Ty src)
  
   \brief Hiwords the given source.
  
   \author  David
   \date  6/11/2016
  
   \tparam  _Ty type of the ty.
   @param src Source for the.
  
   \return  An uint16_t.
   */

  template <typename _Ty>
  constexpr uint16_t hiword(_Ty src){
    static_assert(std::is_pod<_Ty>::value, "invalid parameter");
    return reinterpret_cast<const uint16_t *>(&src)[1];
  }

  /**
   \fn  template <typename _Ty> constexpr uint32_t lodword(_Ty src)
  
   \brief Lodwords the given source.
  
   \author  David
   \date  6/11/2016
  
   \tparam  _Ty type of the ty.
   @param src Source for the.
  
   \return  An uint32_t.
   */

  template <typename _Ty>
  constexpr uint32_t lodword(_Ty src){
    static_assert(std::is_pod<_Ty>::value, "invalid parameter");
    return reinterpret_cast<const uint32_t *>(&src)[0];
  }

  /**
   \fn  template <typename _Ty> constexpr uint32_t hidword(const _Ty& src)
  
   \brief Hidwords the given source.
  
   \author  David
   \date  6/11/2016
  
   \tparam  _Ty type of the ty.
   @param src Source for the.
  
   \return  An uint32_t.
   */

  template <typename _Ty>
  constexpr uint32_t hidword(const _Ty& src){
    static_assert(std::is_pod<_Ty>::value, "invalid parameter");
    return reinterpret_cast<const uint32_t *>(&src)[1];
  }


}

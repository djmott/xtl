/**@file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#if !defined(__XTD_RPC_HPP_INCLUDED__)
#error Dont include this file directly. Instead include rpc.hpp
#endif

namespace xtd{
    namespace rpc{

      namespace _ {
        template <typename _ty, typename> struct marshaler;
      }

      struct payload : std::vector<uint8_t> {
        payload() = default;
        ~payload() = default;
        payload(const payload&) = default;
        payload(payload&&) = default;
        payload& operator=(const payload&) = default;
        payload& operator=(payload&&) = default;

        void marshal() {}

        template <typename _head_t, typename ... _tail_t> void marshal(_head_t&& oHead, _tail_t&&...oTail) {
          _::marshaler<_head_t, _head_t>::marshal(*this, std::forward<_head_t>(oHead));
          marshal(std::forward<_tail_t>(oTail)...);
        }

        template <typename _ty> _ty unmarshal() { return _::marshaler<_ty, _ty>::unmarshal(*this); }
      };

      namespace _ {

        template <typename _ty> struct marshaler<_ty, typename std::enable_if<std::is_pod<_ty>::value, _ty>::type> {
          static void marshal(payload& oPayload, const _ty& val) {
            const auto * pval= reinterpret_cast<const uint8_t*>(&val);
            std::copy(pval, pval + sizeof(_ty), std::back_inserter(oPayload));
          }
          static _ty unmarshal(payload& oPayload) {
            auto oRet = *reinterpret_cast<_ty*>(&oPayload.at(oPayload.size() - sizeof(_ty) - 1));
            oPayload.erase(oPayload.begin() + (oPayload.size() - sizeof(_ty)), oPayload.end());
            return oRet;
          }
        };

        template <> struct marshaler<std::string, std::string> {
          static void marshal(payload& oPayload, const std::string& val) {
            std::copy(val.begin(), val.end(), std::back_inserter(oPayload));
            marshaler<std::string::size_type, std::string::size_type>::marshal(oPayload, val.size());
          }
          static std::string unmarshal(payload& oPayload) {
            auto iSize = marshaler<std::string::size_type, std::string::size_type>::unmarshal(oPayload);
            std::string sRet;
            sRet.reserve(iSize);
            std::copy(oPayload.begin() + (oPayload.size() - iSize - 1), oPayload.end(), std::back_inserter(sRet));
            return sRet;
          }
        };
        template <typename _ty> struct marshaler<std::vector<_ty>, std::vector<_ty>> {
          static void marshal(payload& oPayload, const std::vector<_ty>& val) {
            for (const auto & oItem : val) marshaler<_ty, _ty>::marshal(oPayload);
            marshaler<std::string::size_type, std::string::size_type>::marshal(oPayload, val.size());
          }
          static std::vector<_ty> unmarshal(payload& oPayload) {
            auto iSize = marshaler<std::std::vector<_ty>::size_type, std::std::vector<_ty>::size_type>::unmarshal(oPayload);
            std::vector<_ty> oRet;
            for (; iSize; --iSize) oRet.push_back(marshaler<_ty, _ty>::unmarshal(oPayload));
            return oRet;
          }
        };

      }

    }
}
/**@file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
    namespace rpc{
        
    /*
     * payload
     */
    using payload_t = std::vector<uint8_t>;

    template <bool _skip_in_only, typename ...> struct marshaler;

    template <bool _skip_in_only> struct marshaler<_skip_in_only> {
      static void marshal(...) {}
      static void unmarshal(...) {}
    };
    //skip const references
    template <typename _head_t, typename ... _tail_ts> struct marshaler<true, const _head_t&, _tail_ts...> {
      static void marshal(payload_t& oPayload, const _head_t&, _tail_ts&&...oTail) {
        marshaler<true, _tail_ts...>::marshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
      static void unmarshal(payload_t& oPayload, const _head_t&, _tail_ts&&...oTail) {
        marshaler<true, _tail_ts...>::unmarshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
    };

    //skip byval
    template <typename _head_t, typename ... _tail_ts> struct marshaler<true, _head_t, _tail_ts...> {
      static void marshal(payload_t& oPayload, const _head_t&, _tail_ts&&...oTail) {
        marshaler<true, _tail_ts...>::marshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
      static void unmarshal(payload_t& oPayload, const _head_t&, _tail_ts&&...oTail) {
        marshaler<true, _tail_ts...>::unmarshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
    };

    //POD reference
    template <typename _head_t, typename ... _tail_ts> struct marshaler<false, _head_t&, _tail_ts...> {
      static void marshal(payload_t& oPayload, const _head_t& oHead, _tail_ts&&...oTail) {
        static_assert(std::is_pod<_head_t>::value, "Invalid POD type for marshal");
        auto ptr = reinterpret_cast<const uint8_t*>(&oHead);
        oPayload.insert(oPayload.end(), ptr, ptr + sizeof(_head_t));
        marshaler<false, _tail_ts...>::marshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
      static void unmarshal(payload_t& oPayload, _head_t& oHead, _tail_ts&&...oTail) {
        static_assert(std::is_pod<_head_t>::value, "Invalid POD type for unmarshal");
        oHead = *reinterpret_cast<const _head_t*>(&oPayload.at(0));
        oPayload.erase(oPayload.begin(), oPayload.begin() + sizeof(_head_t));
        marshaler<false, _tail_ts...>::unmarshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
    };


    //POD byval - delegate to POD reference
    template <bool _skip_in_only, typename _head_t, typename ... _tail_ts> struct marshaler<_skip_in_only, _head_t, _tail_ts...> {
      static void marshal(payload_t& oPayload, const _head_t& oHead, _tail_ts&&...oTail) {
        marshaler<_skip_in_only, _head_t&, _tail_ts...>::marshal(oPayload, oHead, std::forward<_tail_ts>(oTail)...);
      }
      static void unmarshal(payload_t& oPayload, _head_t& oHead, _tail_ts&&...oTail) {
        marshaler<false, _head_t&, _tail_ts...>::unmarshal(oPayload, oHead, std::forward<_tail_ts>(oTail)...);
      }
    };


    //string&
    template <typename ... _tail_ts> struct marshaler<false, std::string&, _tail_ts...> {
      static void marshal(payload_t& oPayload, const std::string& oHead, _tail_ts&&...oTail) {
        marshaler<false, size_t>::marshal(oPayload, oHead.size());
        std::copy(oHead.begin(), oHead.end(), std::back_inserter(oPayload));
        marshaler<false, _tail_ts...>::marshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
      static void unmarshal(payload_t& oPayload, std::string& oHead, _tail_ts&&...oTail) {
        size_t len;
        marshaler<false, size_t>::unmarshal(oPayload, len);
        oHead.reserve(len);
        oHead.resize(0, 0);
        std::copy(oPayload.begin(), oPayload.begin() + len, std::back_inserter(oHead));
        oPayload.erase(oPayload.begin(), oPayload.begin() + len);
        marshaler<false, _tail_ts...>::unmarshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
    };
    //vector
    template <typename _item_t, typename ... _tail_ts> struct marshaler<false, std::vector<_item_t>&, _tail_ts...> {
      static void marshal(payload_t& oPayload, const std::vector<_item_t>& oHead, _tail_ts&&...oTail) {
        marshaler<false, size_t>::marshal(oPayload, oHead.size());
        for (const auto & oItem : oHead) marshaler<false, _item_t&>::marshal(oPayload, oItem);
        marshaler<false, _tail_ts...>::marshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
      static void unmarshal(payload_t& oPayload, std::vector<_item_t>& oHead, _tail_ts&&...oTail) {
        size_t len;
        marshaler<false, size_t>::unmarshal(oPayload, len);
        oHead.reserve(len);
        oHead.clear();
        for (; len > 0; --len) {
          _item_t oItem;
          marshaler<false, _item_t&>::unmarshal(oPayload, oItem);
          oHead.push_back(oItem);
        }
        marshaler<false, _tail_ts...>::unmarshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
    };

    struct payload : std::vector<uint8_t> {
      using _super_t = std::vector<uint8_t>;
      payload() : vector(sizeof(size_t), 0) {}
      template <typename _ty> _ty peek() const {
        static_assert(std::is_pod<_ty>::value, "Invalid POD type for peek");
        return *reinterpret_cast<const _ty*>(&at(0));
      }
      void embed_length() {
        auto * iLen = reinterpret_cast<size_t*>(&at(0));
        *iLen = size();
      }
    };


    }
}
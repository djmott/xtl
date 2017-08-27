/** @file
  global unique identifier / universal unique identifier data type
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#pragma once
#include <xtd/xtd.hpp>
#include <xtd/string.hpp>
#include <ios>
#include <fstream>
#include <cstring>
#include "exception.hpp"

#if (XTD_COMPILER_MSVC & XTD_COMPILER)
  #pragma comment(lib, "rpcrt4")
#endif

#if (XTD_HAS_UUID)
  #include <uuid/uuid.h>
#endif

namespace xtd{
#if (XTD_OS_UNIX & XTD_OS)
  #if (XTD_HAS_UUID)
    class unique_id{
    public:
      unique_id(){
        uuid_generate(_uuid);
      }
      unique_id(const unique_id& src){
        uuid_copy(_uuid, src._uuid);
      }
      explicit unique_id(const uuid_t& src){
        uuid_copy(_uuid, src);
      }
      unique_id& operator=(const unique_id& src){
        uuid_copy(_uuid, src._uuid);
        return *this;
      }
      unique_id& operator=(const uuid_t& src){
        uuid_copy(_uuid, src);
        return *this;
      }
      bool operator<(const unique_id& rhs) const{
        return -1 == uuid_compare(_uuid, rhs._uuid);
      }
      bool operator<(const uuid_t& rhs) const{
        return -1 == uuid_compare(_uuid, rhs);
      }
      static const unique_id& nullid(){
        static uuid_t null_id = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        static unique_id oRet(null_id);
        return oRet;
      }
    private:
      template <typename> friend class _::xstring_format;
      uuid_t _uuid;
    };

  #else

  struct unique_id{
    using uuid_t = uint8_t[16];
    uuid_t _uuid;
  public:
    unique_id(){
      std::ifstream oFile;
      oFile.open("/proc/sys/kernel/random/uuid", std::ios::binary);
      if (oFile.bad()) throw std::runtime_error("Failure generating uuid");
      std::string sTemp;
      std::getline(oFile, sTemp);
      //289248a1-379e-4775-bcf5-c6348cf5272b
      sscanf(sTemp.c_str(), "%08hX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
             (uint32_t*)&_uuid[0], (uint16_t*)&_uuid[4], (uint16_t*)&_uuid[6],
             &_uuid[8], &_uuid[9], &_uuid[10], &_uuid[11], &_uuid[12], &_uuid[13], &_uuid[14], &_uuid[15]);

    }
    unique_id(const uuid_t& oID) {
      memcpy(_uuid, &oID, sizeof(uuid_t));
    }
    unique_id(const unique_id& src){
      memcpy(_uuid, src._uuid, sizeof(uuid_t));
    }
    unique_id& operator=(const unique_id& src){
      memcpy(_uuid, src._uuid, sizeof(uuid_t));
      return *this;
    }
    bool operator<(const unique_id& rhs) const{
      return -1 == memcmp(_uuid, rhs._uuid, sizeof(uuid_t));
    }
    static const unique_id& nullid(){
      static uuid_t null_id = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
      static unique_id oRet(null_id);
      return oRet;
    }
  };

  #endif
  namespace _{
    template <> class xstring_format<char, const unique_id&>{
    public:
      inline static string format(const unique_id& value){
        xtd::string oRet(36, 0);
        sprintf(&oRet[0], "%08hX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
                *(uint32_t*)&value._uuid[0], *(uint16_t*)&value._uuid[4], *(uint16_t*)&value._uuid[6],
                value._uuid[8], value._uuid[9], value._uuid[10], value._uuid[11],
                value._uuid[12], value._uuid[13], value._uuid[14], value._uuid[15]);
        return oRet;
      }
    };
  }

#elif (XTD_OS_WINDOWS & XTD_OS)

  class unique_id : uuid_t{
    template <typename, typename> friend class _::xstring_format;
  public:
    unique_id(){
      xtd::exception::throw_if(UuidCreate(this), [](RPC_STATUS x){return (RPC_S_OK != x && RPC_S_UUID_LOCAL_ONLY != x && RPC_S_UUID_NO_ADDRESS != x); });
    }
    unique_id(const unique_id& src){
      std::memcpy(this, &src, sizeof(unique_id));
    }
    unique_id(const uuid_t& src){
      std::memcpy(this, &src, sizeof(uuid_t));
    }
    unique_id& operator=(const unique_id& src){
      if (this != &src){
        std::memcpy(this, &src, sizeof(unique_id));
      }
      return *this;
    }
    unique_id& operator=(const uuid_t& src){
      if (this != &src){
        std::memcpy(this, &src, sizeof(uuid_t));
      }
      return *this;
    }
    bool operator<(const uuid_t& rhs) const{
      return ((Data1 < rhs.Data1) ||
        (Data2 < rhs.Data2) ||
        (Data3 < rhs.Data3) ||
        (Data4[0] < rhs.Data4[0]) ||
        (Data4[1] < rhs.Data4[1]) ||
        (Data4[2] < rhs.Data4[2]) ||
        (Data4[3] < rhs.Data4[3]) ||
        (Data4[4] < rhs.Data4[4]) ||
        (Data4[5] < rhs.Data4[5]) ||
        (Data4[6] < rhs.Data4[6]) ||
        (Data4[7] < rhs.Data4[7]));
    }
    bool operator<(const unique_id& rhs) const{
      return *this < static_cast<const uuid_t&>(rhs);
    }
    static unique_id nullid(){
      uuid_t oRet{ 0, 0, 0,{ 0, 0, 0, 0, 0, 0, 0, 0 } };
      return oRet;
    }
  };

  namespace _{
    template <> class xstring_format<char, const unique_id&>{
    public:
      inline static string format(const unique_id& value){
        xtd::string oRet(36, 0);
        sprintf(&oRet[0], "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
                value.Data1, value.Data2, value.Data3,
                value.Data4[0], value.Data4[1], value.Data4[2], value.Data4[3],
                value.Data4[4], value.Data4[5], value.Data4[6], value.Data4[7]);
        return oRet;
      }
    };
  }

#endif



}

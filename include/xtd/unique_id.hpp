/** @file
  global unique identifier / universal unique identifier (GUID/UUID)
  \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#pragma once

#if (XTD_COMPILER_MSVC & XTD_COMPILER)
  #pragma comment(lib, "rpcrt4")
#endif

namespace xtd{

#if (XTD_HAS_UUID)

  class unique_id{
  public:
    unique_id(){
      uuid_generate(_uuid);
    }
    unique_id(const unique_id& src){
      uuid_copy(_uuid, src._uuid);
    }
    unique_id(const uuid_t& src){
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
    static unique_id nullid(){
      unique_id oRet({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
      return oRet;
    }
  private:
    uuid_t _uuid;
  };

#elif ((XTD_OS_WINDOWS & XTD_OS) || (XTD_OS_MINGW & XTD_OS))

  class unique_id : uuid_t{
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
      uuid_t oRet = { 0, 0, 0,{ 0, 0, 0, 0, 0, 0, 0, 0 } };
      return oRet;
    }
  };

#endif
}

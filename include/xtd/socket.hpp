/** @file
general purpose socket communication
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once
#include <xtd/xtd.hpp>

#if (XTD_OS_WINDOWS & XTD_OS)
  #include <Ws2ipdef.h>
  #include <ws2tcpip.h>
  static_assert(_WIN32_WINNT >= 0x500, "unsupported target Windows version");
  #define poll WSAPoll
#endif

#if (XTD_COMPILER_MINGW & XTD_COMPILER)
  #include <mswsock.h>
#endif

#if (XTD_OS_UNIX & XTD_OS)
  #include <sys/socket.h>
  #include <sys/ioctl.h>
  #include <netinet/in.h>
  #include <netinet/udp.h>
  #include <netinet/tcp.h>
  #include <arpa/inet.h>
  #include <poll.h>
  #include <unistd.h>
#endif

#include <type_traits>
#include <memory>

#include <xtd/exception.hpp>
#include <xtd/string.hpp>
#include <xtd/meta.hpp>
#include <xtd/callback.hpp>

#if (XTD_COMPILER_MSVC & XTD_COMPILER)
  #pragma comment(lib, "ws2_32")
#endif

namespace xtd{

    namespace socket{
    /// @addtogroup Sockets
    /// @{
#if (!DOXY_INVOKED)
#if (XTD_COMPILER_MINGW & XTD_COMPILER)
    using POLLFD=WSAPOLLFD;
#elif (XTD_OS_UNIX & XTD_OS)
  using SOCKET = int;
  #define closesocket close
  #define ioctlsocket ioctl
#elif (XTD_OS_WINDOWS & XTD_OS)
    using POLLFD = pollfd;
#endif
#endif
    ///Represents an socket error
    class exception : public xtd::crt_exception{
    public:
      /// constructors
      /// @{
      exception(const source_location& loc, const xtd::string& swhat) : crt_exception(loc, swhat){}
      exception(const exception& ex) : crt_exception(ex){}
      explicit exception(exception&& ex) : crt_exception(std::move(ex)){}
      /// @}

      template <typename _ReturnT, typename _ExpressionT>
      inline static _ReturnT _throw_if(const xtd::source_location& source, _ReturnT ret, _ExpressionT exp, const char* expstr){
        if (exp(ret)){
          throw exception(source, expstr);
        }
        return ret;
      }

    };

#if (!DOXY_INVOKED)
    namespace _{
      template <typename _Ty, int level, int optname> class socket_option{
      public:
        using value_type = _Ty;
        static value_type get(SOCKET s){
          value_type iRet;
#if  (XTD_COMPILER_MSVC & XTD_COMPILER)
          int iSize = sizeof(value_type);
#else
          std::remove_pointer<typename xtd::template get_parameter<4, decltype(getsockopt)>::type>::type iSize = sizeof(value_type);
#endif
          socket::exception::throw_if(getsockopt(s, level, optname, reinterpret_cast<char*>(&iRet), &iSize), [](int i){ return (i<0); });
          return iRet;
        }
        static void set(SOCKET s, value_type newval){
          socket::exception::throw_if(setsockopt(s, level, optname, reinterpret_cast<char*>(&newval), sizeof(newval)), [](int i){ return (i<0); });
        }
      };
      template <int level, int optname> class socket_option<std::string, level, optname>{
      public:
        using value_type = std::string;
        static value_type get(SOCKET s){
          value_type iRet;
          int iSize = 0;
          socket::exception::throw_if(getsockopt(s, level, optname, reinterpret_cast<char*>(&iRet), &iSize), [](int i){ return (i<0); });
          return iRet;
        }
        static void set(SOCKET s, value_type newval){
          socket::exception::throw_if(setsockopt(s, level, optname, reinterpret_cast<char*>(&newval), sizeof(newval)), [](int i){ return (i<0); });
        }
      };


    }
#endif

    ///IPv4 address wrapper around sockaddr_in
    class ipv4address : public sockaddr_in{
    public:
      /// ipv4 address familt
      static const int address_family = AF_INET;
      /**
       * constructor
       * @param sIP IP address
       * @param iPort port
       */
      ipv4address(const char * sIP, uint16_t iPort){
        sin_family = AF_INET;
        sin_addr.s_addr = inet_addr(sIP);
        sin_port = htons(iPort);
      }
      ipv4address(const ipv4address& src){
        memcpy(this, &src, sizeof(ipv4address));
      }
      ipv4address& operator=(const ipv4address& src){
        if (&src != this) memcpy(this, &src, sizeof(ipv4address));
        return *this;
      }
    };

    ///IPv6 address wrapper around sockaddr_in6
    class ipv6address : public sockaddr_in6{
    public:
      /// ipv6 address family
      static const int address_family = AF_INET6;
      TODO("Implement proper ipv6 address");
    };

    /// socket communication styles
    enum class socket_type{
      stream = SOCK_STREAM, ///< stream reliable FIFO stream on a remote socket
      datagram = SOCK_DGRAM, ///< datagram unreliable connectionless broadcast
      raw = SOCK_RAW, ///< raw interface level data
    };

    /// IP based protocols
    enum class socket_protocol{      
      ip = IPPROTO_IP, ///< ip Internet Protocol v4      
      icmp = IPPROTO_ICMP,///< icmp Internet Control Message Protocol      
      tcp = IPPROTO_TCP,///< tcp Transmission Control Protocol     
      udp = IPPROTO_UDP, ///< udp User Datagram Protocol
      ipv6 = IPPROTO_IPV6, ///< ipv6 Internet Protocol v6
    };
    NOTE("add more socket protocols as needed defined in netinet/in.h")


    TODO("Refactor out socket::serializer into something more universal")
    ///Serializes data on a socket
    template <typename> class serializer;

#if (XTD_OS_WINDOWS & XTD_OS)
    /** Initializes WinSock
    Winsock requires a call to WSAStartup before any other calls to the winsock library.
    winsock_initializer is a global static that ensures winsock is initialized and cleaned up properly
    */
    class winsock_initializer sealed{
    public:
      //singleton getter method
      static winsock_initializer &get(){
        static winsock_initializer oInit;
        return oInit;
      }
      winsock_initializer(const winsock_initializer&) = delete;
      winsock_initializer& operator=(const winsock_initializer&) = delete;
    private:
      winsock_initializer(){
        WSADATA oData;
        crt_exception::throw_if(WSAStartup(0x202, &oData), [](int err){ return 0 != err; });
        if (oData.wVersion != 0x202) throw crt_exception(here(), "Invalid winsock version");
      }

      ~winsock_initializer(){ WSACleanup(); }
    };
#endif


#if (!DOXY_INVOKED)
    template<typename _AddressT, socket_type, socket_protocol, template<class> class ... _Policies> class socket_base;


    template<typename _AddressT, socket_type _ST, socket_protocol _PR, template<class> class _HeadT, template<class> class ..._TailT>
    class socket_base<_AddressT, _ST, _PR, _HeadT, _TailT...> : public _HeadT<socket_base<_AddressT, _ST, _PR, _TailT...> >{
    public:
      template<typename ... _ArgsT> explicit socket_base(_ArgsT &&...oArgs) : _HeadT<socket_base<_AddressT, _ST, _PR, _TailT...> >(std::forward<_ArgsT>(oArgs)...){}
    };
#endif

    /** Base class of the various socket specializations
    Hierarchy generation TMP pattern to create all the socket object types with the various behavioral policies.
     */
    template<typename _AddressT, socket_type _SocketT, socket_protocol _Protocol>
    class socket_base<_AddressT, _SocketT, _Protocol>{
    public:

      /// typedefs
      /// @{
      using unique_ptr = std::unique_ptr<socket_base>;
      using shared_ptr = std::shared_ptr<socket_base>;
      using address_type = _AddressT;
      /// @}

      /// constants
      /// @{
      static constexpr socket_type type = _SocketT;
      static constexpr socket_protocol protocol = _Protocol;
      /// @}

      /// dtor
      ~socket_base(){ close(); }

      /// constructors
      /// @{
      socket_base() : _Socket(0){
#if (XTD_OS_WINDOWS & XTD_OS)
        winsock_initializer::get();
#endif
        _Socket = xtd::crt_exception::throw_if(::socket(address_type::address_family, (int)type, (int)protocol), [](SOCKET s){ return static_cast<SOCKET>(-1) == s; });
      }

      explicit socket_base(socket_base&& src) : _Socket(src._Socket){
        src._Socket = 0;
      }


      explicit socket_base(SOCKET newval) : _Socket(newval){}

      socket_base(const socket_base&) = delete;
      /// @}

      /// copy-assignment not permitted
      socket_base& operator=(const socket_base&) = delete;

      /**
       * move assignment
       * @param src source socket
       * @return *this
       */
      socket_base& operator=(socket_base&& src){
        std::swap(_Socket, src._Socket);
        return *this;
      }

      /**
       * cast to native socket
       * @return native socket value
       */
      operator SOCKET() const{ return _Socket; }


      /**
       * writes data to the connected socket
       * @param data the data to write
       */
      template <typename _Ty> void write(const _Ty& data){
        serializer<_Ty>::write(*this, data);
      }

      /**
       * reads data from the connected socket
       * @param data returned data
       */
      template <typename _Ty> void read(_Ty& data){
        serializer<_Ty>::read(*this, data);
      }

      /**
       * reads data from the connected socket
       * @return the data of type _Ty from the socket
       */
      template <typename _Ty> _Ty read(){
        _Ty data;
        serializer<_Ty>::read(*this, data);
        return data;
      }

      /**
       * Closes the open socket
       */
      void close(){
        if (!_Socket){
          return;
        }
        ::closesocket(_Socket);
        _Socket = -1;
      }

      /** sets the blocking mode of the socket
       * @param blocking true to set to blocking mode
       */
      void set_blocking(bool blocking){
        u_long val = (blocking ? 0 : 1);
        xtd::crt_exception::throw_if(ioctlsocket(_Socket, FIONBIO, &val), [](int i){ return i < 0; });
      }

      /** gets the number of bytes waiting in the read buffer
       */
      u_long bytes_available(){
        u_long iRet=0;
        xtd::crt_exception::throw_if(ioctlsocket(_Socket, FIONREAD, &iRet), [](int i){ return i < 0; });
        return iRet;
      }

      /**
       * test if the socket is valid
       */
      bool is_valid() const{ return -1 != _Socket; }

    protected:
      /// OS/CRT inner SOCKET that is being managed by this wrapper
      SOCKET _Socket;

    };


    ///Polling behavior policy
    template <typename _SuperT>
    class polling_socket : public _SuperT{
    public:
      /// callback event fires when data is ready to read data
      callback<void()> read_event;
      /// callback event fires when socket is ready to write data
      callback<void()> write_event;
      /// callback event fires when socket becomes disconnected
      callback<void()> disconnect_event;
      /// callback event fires when an error occurs
      callback<void()> error_event;
      /// begins polling the socket for events for a period of Timeout
      void poll(int Timeout){
        struct pollfd oPoll;
        oPoll.events = POLLIN | POLLOUT;
        oPoll.fd = _SuperT::_Socket;
        oPoll.revents = 0;
        if (0 == xtd::crt_exception::throw_if(::poll(&oPoll, 1, Timeout), [](int i){ return i < 0; })){
          return;
        }
        if (oPoll.revents & POLLERR){
          error_event();
        }
        if (oPoll.revents & POLLHUP){
          disconnect_event();
        }
        if (oPoll.revents & POLLIN){
          read_event();
        }
        if (oPoll.revents & POLLOUT){
          write_event();
        }
      }
      ///ctor
      template<typename ... _ArgTs>
      explicit polling_socket(_ArgTs &&...oArgs) : _SuperT(std::forward<_ArgTs>(oArgs)...){}

#if (XTD_OS_WINDOWS & XTD_OS)
      int poll(POLLFD *ufds, unsigned int nfds, int timeout){ return ::WSAPoll(ufds, nfds, timeout); }
#endif
    };


    ///Server side binding behavior
    template <typename _SuperT>
    class bindable_socket : public _SuperT{
    public:

      /// ctor
      template<typename ... _ArgTs>
      explicit bindable_socket(_ArgTs &&...oArgs) : _SuperT(std::forward<_ArgTs>(oArgs)...){}

      /// binds the socket to an address and port
      void bind(const typename _SuperT::address_type& addr){
        exception::throw_if(::bind(_SuperT::_Socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(typename _SuperT::address_type)), [](int i){ return i < 0; });
      }
    };


    ///Client side connecting behavior
    template <typename _SuperT>
    class connectable_socket : public _SuperT{
    public:

      /// ctor
      template<typename ... _ArgTs>
      explicit connectable_socket(_ArgTs &&...oArgs) : _SuperT(std::forward<_ArgTs>(oArgs)...){}

      /// initiates connection to a socket
      void connect(const typename _SuperT::address_type& addr){
        exception::throw_if(::connect(_SuperT::_Socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(typename _SuperT::address_type)), [](int i){ return i < 0; });
      }
    };


    ///Server side listening behavior
    template <typename _SuperT>
    class listening_socket : public _SuperT{
    public:

      /// ctor
      template<typename ... _ArgTs>
      explicit listening_socket(_ArgTs &&...oArgs) : _SuperT(std::forward<_ArgTs>(oArgs)...){}

      /// begins listening on the socket
      void listen(int Backlog = SOMAXCONN){
        exception::throw_if(::listen(_SuperT::_Socket, Backlog), [](int i){ return i < 0; });
      }

      /// accepts an incoming connection request
      template <typename _ReturnT>
      _ReturnT accept(){
        return _ReturnT(exception::throw_if(::accept(_SuperT::_Socket, nullptr, nullptr), [](SOCKET s){ return (s <= 0); }));
      }
    };

    /// Socket properties
    template <typename _SuperT>
    class socket_options : public _SuperT{
    public:

      /// ctor
      template<typename ... _ArgTs>
      explicit socket_options(_ArgTs&&...oArgs) : _SuperT(std::forward<_ArgTs>(oArgs)...){}

      /// gets the SO_KEEPALIVE property
      bool keep_alive() const{ return (_::socket_option<int, SOL_SOCKET, SO_KEEPALIVE>::get(_SuperT::_Socket) ? true : false); }
      /// sets the SO_KEEPALIVE property
      void keep_alive(bool newval){ _::socket_option<int, SOL_SOCKET, SO_KEEPALIVE>::set(_SuperT::_Socket, newval); }
      TODO("Add more SOL_SOCKET options");
    };

    /// IP based socket properties
    template <typename _SuperT>
    class ip_options : public _SuperT{
    public:

      /// ctor
      template<typename ... _ArgTs>
      explicit ip_options(_ArgTs&&...oArgs) : _SuperT(std::forward<_ArgTs>(oArgs)...){}
#if (XTD_OS_WINDOWS & XTD_OS)
      /// gets the IP_DONTFRAGMENT property
      bool dont_fragment() const{ return (_::socket_option<int, IPPROTO_IP, IP_DONTFRAGMENT>::get(_SuperT::_Socket) ? true : false); }
      /// sets the IP_DONTFRAGMENT property
      void dont_fragment(bool newval){ _::socket_option<int, IPPROTO_IP, IP_DONTFRAGMENT>::set(_SuperT::_Socket, newval); }
#endif
      TODO("Add more IPPROTO_IP options");
    };

    /// TCP based socket properties
    template <typename _SuperT>
    class tcp_options : public _SuperT{
    public:

      /// ctor
      template<typename ... _ArgTs>
      explicit tcp_options(_ArgTs&&...oArgs) : _SuperT(std::forward<_ArgTs>(oArgs)...){}

      /// gets the TCP_NODELAY property
      bool no_delay() const{ return (_::socket_option<int, IPPROTO_TCP, TCP_NODELAY>::get(_SuperT::_Socket) ? true : false); }
      /// sets the TCP_NODELAY property
      void no_delay(bool newval){ _::socket_option<int, IPPROTO_TCP, TCP_NODELAY>::set(_SuperT::_Socket, newval); }
      TODO("Add more IPPROTO_TCP options");
    };

    /// UDP socket properties
    template <typename _SuperT>
    class udp_options : public _SuperT{
    public:

      /// ctor
      template<typename ... _ArgTs>
      explicit udp_options(_ArgTs&&...oArgs) : _SuperT(std::forward<_ArgTs>(oArgs)...){}

#if (XTD_OS_WINDOWS & XTD_OS)
      /// gets the UDP_NOCHECKSUM property
      bool no_checksum() const{ return (_::socket_option<int, IPPROTO_UDP, UDP_NOCHECKSUM>::get(_SuperT::_Socket) ? true : false); }
      /// sets the UDP_NOCHECKSUM property
      void no_checksum(bool newval){ _::socket_option<int, IPPROTO_UDP, UDP_NOCHECKSUM>::set(_SuperT::_Socket, newval); }
#endif
      TODO("Add more IPPROTO_UDP options");
    };


    ///Async IO select behavior
    template <typename _SuperT>
    class selectable_socket : public _SuperT{
    public:
      /// ctor
      template<typename ... _ArgTs>
      explicit selectable_socket(_ArgTs&&...oArgs) : _SuperT(std::forward<_ArgTs>(oArgs)...){}

      /// callback event fired when data is ready to be read
      xtd::callback<void()> onRead;
      /// callback event fired when socket is ready to write
      xtd::callback<void()> onWrite;
      /// callback event fired when a socket error occurs
      xtd::callback<void()> onError;

      /// begin the select to wait for an event or timeout
      bool select(int WaitMS){
        timeval tv;
        fd_set fdRead;
        fd_set fdWrite;
        fd_set fdErr;
        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdErr);
        FD_SET((SOCKET)*this, &fdRead);
        FD_SET((SOCKET)*this, &fdWrite);
        FD_SET((SOCKET)*this, &fdErr);
        tv.tv_sec = WaitMS / 1000;
        WaitMS %= 1000;
        tv.tv_usec = WaitMS / 1000;
#if (XTD_COMPILER_MINGW & XTD_COMPILER)
        auto iRet = xtd::crt_exception::throw_if(::select(1 + (SOCKET)*this, &fdRead, &fdWrite, &fdErr, reinterpret_cast<PTIMEVAL>(&tv)), [](int i){return i < 0; });
#else
        auto iRet = xtd::crt_exception::throw_if(::select(1 + (SOCKET)*this, &fdRead, &fdWrite, &fdErr, &tv), [](int i){return i < 0; });
#endif
        if (0 == iRet){
          return false;
        }
        if (FD_ISSET((SOCKET)*this, &fdErr)){
          onError();
        }
        if (FD_ISSET((SOCKET)*this, &fdRead)){
          onRead();
        }
        if (FD_ISSET((SOCKET)*this, &fdWrite)){
          onWrite();
        }
        return true;
      }
    };

#if (!DOXY_INVOKED)
    TODO("Get rid of these")
    template <typename _Ty>
    class serializer{
    public:

      template <typename _SocketT>
      static void write(_SocketT& oSocket, const _Ty& src){
        static_assert(std::is_pod<_Ty>::value, "no acceptable specialization for type");
        exception::throw_if(::send(oSocket, reinterpret_cast<const char*>(&src), sizeof(_Ty), 0), [](int i){ return i <= 0; });
      }

      template <typename _SocketT>
      static void read(_SocketT& oSocket, _Ty& src){
        static_assert(std::is_pod<_Ty>::value, "no acceptable specialization for type");
        exception::throw_if(::recv(oSocket, reinterpret_cast<char*>(&src), sizeof(_Ty), 0), [](int i){ return i <= 0; });
      }
    };


    template <typename _Ty>
    class NON_POD_Vector_Serializer{
    public:

      template <typename _SocketT>
      static void write(_SocketT& oSocket, const std::vector<_Ty>& src){
        serializer<typename std::vector<_Ty>::size_type>::write(oSocket, src.size());
        for (const auto & oItem : src){
          serializer<_Ty>::write(oSocket, oItem);
        }
      }

      template <typename _SocketT>
      static void read(_SocketT& oSocket, std::vector<_Ty>& src){
        typename std::vector<_Ty>::size_type count;
        serializer<typename std::vector<_Ty>::size_type>::read(oSocket, count);
        for (; count; --count){
          _Ty newval;
          serializer<_Ty>::read(oSocket, newval);
          src.push_back(newval);
        }
      }
    };


    template <typename _Ty>
    class POD_Vector_Serializer{
    public:

      template <typename _SocketT >
      static void write(_SocketT& oSocket, const std::vector<_Ty>& src){
        serializer<typename std::vector<_Ty>::size_type>::write(oSocket, src.size());
        send(oSocket, reinterpret_cast<const char*>(&src[0]), (int)(sizeof(_Ty) * src.size()), 0);
      }

      template <typename _SocketT>
      static void read(_SocketT& oSocket, std::vector<_Ty>& src){
        typename std::vector<_Ty>::size_type count;
        serializer<typename std::vector<_Ty>::size_type>::read(oSocket, count);
        src.resize(count);
        recv(oSocket, reinterpret_cast<char*>(&src[0]), (int)(sizeof(_Ty) * count), 0);
      }

    };


    template <typename _Ty>
    class serializer<std::vector<_Ty>> : public std::conditional<std::is_pod<_Ty>::value, POD_Vector_Serializer<_Ty>, NON_POD_Vector_Serializer<_Ty>>::type{};
#endif
    /// General purpose IPV4 client and server socket type
    using ipv4_tcp_stream = socket_base<ipv4address, socket_type::stream, socket_protocol::tcp, socket_options, ip_options, tcp_options, connectable_socket, bindable_socket, listening_socket, selectable_socket>;
    /// General purpose UDP socket type
    using ipv4_udp_socket = socket_base<ipv4address, socket_type::datagram, socket_protocol::udp, socket_options, ip_options, udp_options>;
    ///@}

  }

}

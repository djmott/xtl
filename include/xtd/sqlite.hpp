/** @file
  sqlite interface
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <memory>

#include <xtd/exception.hpp>
#include <xtd/string.hpp>
#include <xtd/filesystem.hpp>
#include <xtd/tuple.hpp>

#include <sqlite3.h>


#if defined(throw_if)
  #pragma push_macro("throw_if")
  #undef throw_if
#endif

#define throw_if(db, test, exp) _throw_if(db, here(), test, exp, #test)

namespace xtd{
  namespace sqlite{



    /** 
    c++ exception wrapper around sqlite error
    */
    class exception : public xtd::exception{
    public:
      template <typename _DatabaseT, typename _ExpressionT>
      inline static int _throw_if(_DatabaseT db, const xtd::source_location& source, int ret, _ExpressionT exp, const char* expstr){
        if (exp(ret)){
          throw exception(db, source, ret, expstr);
        }
        return ret;

      }


      /// constructors
      /// @{
      template <typename _DatabaseT>
      exception(_DatabaseT db, const source_location& Source, int ErrCode, const std::string& What) : xtd::exception(Source, What){

        _errnum = ErrCode;

        _what += " : ";

        _what += sqlite3_errstr(sqlite3_extended_errcode(db));

      }
      exception(const exception& ex) : xtd::exception(ex){}
      exception(exception&& ex) : xtd::exception(std::move(ex)){}
      /// @}
      /**
       * Error number returned by sqlite
       * @return the sqlite3 error that caused the exception
       */
      int error_code() const { return _errnum; }
    };

    namespace _{

      template <typename _Ty> struct sqlite_field_loader;

      template <> struct sqlite_field_loader<double>{
        template <int Index> static double get(sqlite3_stmt* st){ return sqlite3_column_double(st, Index); }
      };

      template <> struct sqlite_field_loader<int>{
        template <int Index> static int get(sqlite3_stmt* st){ return sqlite3_column_int(st, Index); }
      };

      template <> struct sqlite_field_loader<sqlite3_int64>{
        template <int Index> static sqlite3_int64 get(sqlite3_stmt* st){ return sqlite3_column_int64(st, Index); }
      };

      template <> struct sqlite_field_loader<char*>{
        template <int Index> static const char * get(sqlite3_stmt* st){ return reinterpret_cast<const char *>(sqlite3_column_text(st, Index)); }
      };

      template <> struct sqlite_field_loader<std::vector<uint8_t>>{
        template <int Index> static std::vector<uint8_t>& get(sqlite3_stmt* st){ 
          return reinterpret_cast<const char *>(sqlite3_column_text(st, Index)); 
        }
      };

      template <> struct sqlite_field_loader<xtd::string>{
        template <int Index> static const char * get(sqlite3_stmt* st){
          return reinterpret_cast<const char *>(sqlite3_column_text(st, Index));
        }
      };

    }
    /**
    result_set
    */
    template <typename ...> class result_set;

    template <> class result_set<>{
    protected:
      friend class database;
      sqlite3 * _database;
      sqlite3_stmt * _pStatement;
      result_set(sqlite3 * pDB, sqlite3_stmt * pStatement) : _database(pDB), _pStatement(pStatement){}
    public:
      bool next(){ return (SQLITE_DONE != exception::throw_if(_database, sqlite3_step(_pStatement), [](int i){ return !(i == SQLITE_OK || i == SQLITE_DONE || i==SQLITE_ROW);  })); }
      ~result_set(){ if (_pStatement) sqlite3_finalize(_pStatement); }
      
      result_set(const result_set&) = delete;
      result_set& operator=(const result_set&) = delete;
      result_set(result_set&& src) : _database(nullptr), _pStatement(nullptr){
        std::swap(_database, src._database);
        std::swap(_pStatement, src._pStatement);
      }
      result_set& operator=(result_set&& src){
        std::swap(_database, src._database);
        std::swap(_pStatement, src._pStatement);
        return *this;
      }
    };

    template <typename _HeadT, typename ... _TailT> class result_set<_HeadT, _TailT...> : public result_set<_TailT...>{
    protected:
      friend class database;
      using _super_t = result_set<_TailT...>;
      result_set(sqlite3 * pDB, sqlite3_stmt * pStatement) : _super_t(pDB, pStatement){}
    public:
      using value_type = _HeadT;
      using pointer = std::shared_ptr < result_set > ;
      template <int _index> using type_at = typename xtd::_::tuple_index<_index, _HeadT, _TailT...>::value_type;
      template <int _index> type_at<_index> get(){
        return _::sqlite_field_loader<type_at<_index>>::template get<_index>(_super_t::_pStatement);
      }

      result_set(const result_set&) = delete;
      result_set& operator=(const result_set&) = delete;
      result_set(result_set&& src) : _super_t(std::move(src)){}
      result_set& operator=(result_set&& src){
        _super_t::operator=(std::move(src));
        return *this;
      }

    };

    namespace _{
      template <typename> struct sqlite_field_binder;

      template <> struct sqlite_field_binder<double>{
        template <int Index> static void set(sqlite3 * pDB, sqlite3_stmt* st, const double& val){ exception::throw_if(pDB, sqlite3_bind_double(st, Index, val), [](int i){return SQLITE_OK != i; }); }
      };

      template <> struct sqlite_field_binder<int>{
        template <int Index> static void set(sqlite3 * pDB, sqlite3_stmt* st, const int& val){ exception::throw_if(pDB, sqlite3_bind_int(st, Index, val), [](int i){return SQLITE_OK != i; }); }
      };

      template <> struct sqlite_field_binder<sqlite3_int64>{
        template <int Index> static void set(sqlite3 * pDB, sqlite3_stmt* st, const sqlite3_int64& val){ exception::throw_if(pDB, sqlite3_bind_int64(st, Index, val), [](int i){return SQLITE_OK != i; }); }
      };

      template <> struct sqlite_field_binder<std::vector<uint8_t>>{
        template <int Index> static void set(sqlite3 * pDB, sqlite3_stmt* st, const std::vector<uint8_t>& val){ exception::throw_if(pDB, sqlite3_bind_blob(st, Index, &val[0], val.size(), SQLITE_TRANSIENT), [](int i){return SQLITE_OK != i; }); }
      };

      template <> struct sqlite_field_binder<xtd::string>{
        template <int Index> static void set(sqlite3 * pDB, sqlite3_stmt* st, const xtd::string& val){ exception::throw_if(pDB, sqlite3_bind_text(st, Index, val.c_str(), val.size(), SQLITE_TRANSIENT), [](int i){return SQLITE_OK != i; }); }
      };

      template <int, typename ...> struct sqlite_command_params;

      template <int idx> struct sqlite_command_params<idx>{
        static void set(sqlite3 *, sqlite3_stmt *){}
      };

      template <int idx, typename _HeadT, typename ... _TailT> struct sqlite_command_params<idx, _HeadT, _TailT...>{
        static void set(sqlite3 * pDB, sqlite3_stmt * st, const _HeadT& oHead, _TailT&&...oTail){
          sqlite_field_binder<_HeadT>::set<idx>(pDB, st, oHead);
          sqlite_command_params<1 + idx, _TailT...>::set(pDB, st, std::forward<_TailT>(oTail)...);
        }
      };
    }
    /**
    command
    */
    template <typename ... _ArgTs> class command{
      friend class database;
      sqlite3 * _pDatabase;
      sqlite3_stmt * _pStatement;
      command(sqlite3 * pDB, sqlite3_stmt * pStatement) : _pStatement(nullptr), _pDatabase(pDB){}
    public:
      using pointer = std::shared_ptr<command>;
      ~command(){ if (_pStatement) sqlite3_finalize(_pStatement); }
      command() = delete;
      command(const command&) = delete;
      command& operator=(const command&) = delete;
      command(command&& src) : _pDatabase(src._pDatabase), _pStatement(src._pStatement){
        src._pStatement = nullptr;
      }
      command& operator=(command&& src){
        std::swap(_pDatabase, src._pDatabase);
        std::swap(_pStatement, src._pStatement);
        return *this;
      }
      int operator()(_ArgTs&&...oArgs){
        _::sqlite_command_params<1, _ArgTs...>::set(_pDatabase, _pStatement, std::forward<_ArgTs>(oArgs)...);
        exception::throw_if(_pDatabase, sqlite3_step(_pStatement), [](int i){ return !(i == SQLITE_OK || i == SQLITE_DONE || i == SQLITE_ROW);  });
        exception::throw_if(_pDatabase, sqlite3_reset(_pStatement), [](int i){return SQLITE_OK != i; });
        return sqlite3_changes(_pDatabase);
      }
    };



    template <typename _DatabaseT> class transaction{
      friend class database;
      _DatabaseT * _database;
      bool _finished;
      transaction(_DatabaseT * oDB) : _database(oDB), _finished(false){
        if (_database) _database->execute("begin transaction;");
      }
    public:

      transaction() = delete;
      transaction(const transaction&) = delete;
      transaction& operator=(const transaction& src) = delete;

      transaction(transaction&& src) {
        std::swap(_database, src._database);
        std::swap(_finished, src._finished);
      }
      transaction& operator=(transaction&& src){
        std::swap(_database, src._database);
        std::swap(_finished, src._finished);
        return *this;
      }
      ~transaction(){
        if (!_finished){
          try{ rollback(); }
          catch(...){}
        }
      }

      void commit(){
        if (!_finished && _database){
          _database->execute("commit transaction;");
        }
        _finished = true;
      }

      void rollback(){
        if (!_finished && _database){
          _database->execute("rollback transaction;");
        }
        _finished = true;
      }

    };

    /**
    sqlite3 database
    */
    class database : public std::enable_shared_from_this<database>{
      sqlite3 * _pDB;
      explicit database(const xtd::filesystem::path& oPath, int flags){
        exception::throw_if(_pDB, sqlite3_open_v2(oPath.string().c_str(), &_pDB, flags, nullptr), [](int i){ return SQLITE_OK != i; });
      }
    public:
      using pointer = std::shared_ptr<database>;
      using transaction_type = transaction<database>;

      enum open_flags : int{
        NoMutex = SQLITE_OPEN_NOMUTEX,
        FullMutex = SQLITE_OPEN_FULLMUTEX,
        SharedCache = SQLITE_OPEN_SHAREDCACHE,
        PrivateCache = SQLITE_OPEN_PRIVATECACHE,
        ReadOnly = SQLITE_OPEN_READONLY,
        ReadWrite = SQLITE_OPEN_READWRITE,
        OpenCreate = SQLITE_OPEN_CREATE,
        OpenURI = SQLITE_OPEN_URI,
        MemoryDB = SQLITE_OPEN_MEMORY,
      };

      ~database(){ if (_pDB) sqlite3_close(_pDB); }

      static pointer open_database(const xtd::filesystem::path& oPath, int flags = ReadWrite | OpenCreate){
        return pointer(new database(oPath, flags));
      }

      template <typename ... _ArgTs>
      typename command<_ArgTs...>::pointer prepare(const xtd::string& sSQL){
        command<_ArgTs...> oRet(_pDB, nullptr);
        exception::throw_if(_pDB, sqlite3_prepare_v2(_pDB, sSQL.c_str(), (int)(1 + sSQL.size()), &oRet._pStatement, nullptr), [](int i){ return SQLITE_OK != i; });
        return std::make_shared<command<_ArgTs...>>(std::move(oRet));
      }

      transaction_type begin_transaction(){
        return transaction_type(this);
      }

      int execute(const xtd::string& sSQL){
        auto oCmd = prepare<>(sSQL);
        return (*oCmd)();
      }

      template <typename ... _Types>
      typename result_set<_Types...>::pointer execute_reader(const xtd::string& sSQL){
        result_set<_Types...> oRet(_pDB, nullptr);
        exception::throw_if(_pDB, sqlite3_prepare_v2(_pDB, sSQL.c_str(), (int)(1 + sSQL.size()), &oRet._pStatement, nullptr), [](int i){ return SQLITE_OK != i; });
        return std::make_shared<result_set<_Types...>>(std::move(oRet));
      }


    };



  }
}

#pragma pop_macro("throw_if")

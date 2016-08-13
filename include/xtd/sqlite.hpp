/** @file
  sqlite interface
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>


#include <xtd/exception.hpp>
#include <xtd/string.hpp>
#include <xtd/filesystem.hpp>

#include <sqlite3.h>

#include <memory>

#if defined(throw_err)
  #pragma push_macro("throw_err")
  #undef throw_err
#endif

#define throw_err(test) _throw_err(here(), test, #test)

namespace xtd{
  namespace sqlite{



    /** 
    c++ exception wrapper around sqlite error
    */
    class exception : public xtd::exception{
    public:
      template <typename _ReturnT, typename _ExpressionT>
      inline static _ReturnT _throw_if(const xtd::source_location& source, _ReturnT ret, _ExpressionT exp, const char* expstr){
        if (exp(ret)){
          throw exception(source, expstr);
        }
        return ret;

      }

      inline static void _throw_err(xtd::source_location& where, int test, const char * source){
        if (SQLITE_OK!=test) {
          throw exception(where, test, source);
        }
      }

      /// constructors
      /// @{
      exception(const source_location& Source, int ErrCode, const std::string& What) : xtd::exception(Source, What){

        _errnum = ErrCode;

        _what += " : ";

        _what += sqlite3_errstr(_errnum);

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



    /**
    sqlite3 statement
    */
    class statement{
      sqlite3_stmt * _pStatement;
    public:
      ~statement(){ sqlite3_finalize(_pStatement); }
    };



    /**
    */
    template <typename _Ty, size_t _len = sizeof(_Ty)> struct field_def{
      sqlite3_stmt * _pStatement;
    public:
      field_def(){}
      ~field_def(){}
    };



    /**
    sqlite table_def
    */
    template <typename ... _FieldDefTs> class table_def{
    public:

    };



    /**
    sqlite3 database
    */
    class database : std::enable_shared_from_this<database>{
      sqlite3 * _pDB;
      database(const xtd::filesystem::path& oPath){
        exception::throw_err(sqlite3_open(oPath.string().c_str(), &_pDB));
      }
    public:
      using pointer = std::shared_ptr<database>;

      static pointer open_database(const xtd::filesystem::path& oPath){
        return pointer(new database(oPath));
      }

      ~database(){
        sqlite3_close(_pDB);
      }
    };



  }
}

#pragma pop_macro("throw_err")
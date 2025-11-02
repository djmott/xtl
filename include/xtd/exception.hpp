/** @file
generic and special purpose exceptions
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#pragma once

#include <xtd/xtd.hpp>

#include <exception>
#include <source_location>
#include <xtd/source_location.hpp>
#include <xtd/string.hpp>
/**
@def throw_if(_test, _expression) _throw_if(here(), _test, _expression, #_test)

Simplifies use of exception::_throw_if

@param _test       Expression to test
@param _expression Expression to evaluate the value of _test
*/
#define throw_if(_test, _expression) _throw_if(here(), _test, _expression, #_test)

namespace xtd{

  class unhandled_exception{};

/// Base exception for XTL
    class exception : public std::exception{
    public:
        /// shortcut typedef of the super class
        using _super_t = std::exception;

/** Helper function template to test for an expression and throw an exception
Throws exception if the test expression returns true. _throw_if methods are present in many XTL exception classes. Typically, the throw_if macro will be used as in exception::throw_if to properly fill parameters
@tparam _return_t The return type of the function and type of the expression to evaluate. Implicitly deduced at compile time.
@tparam _expression_t the type of expression tester that evaluates _return_t, normally a lambda.
@param source The location in source code where called.
@param ret The return value of _throw_if and parameter to exp used to test for success or failure
@param exp Expression used to evaluate ret. Expression is in the form of: bool (_return_t). The exception is thrown if this expression evaluates to true.
@param expstr String value passed to the exceptions constructor when thrown
@returns ret if no exception is thrown
*/
        /** @brief Helper function to test and throw exception
         * @tparam _return_t Return type
         * @tparam _expression_t Expression type (typically lambda)
         * @param source Source location (std::source_location or xtd::source_location)
         * @param ret Return value to test
         * @param exp Expression that evaluates ret
         * @param expstr Error message
         * @return ret if no exception thrown
         */
        template <typename _return_t, typename _expression_t>
#if __cpp_lib_source_location >= 201907L
        inline static _return_t _throw_if(const std::source_location& source, _return_t ret, _expression_t exp, const char* expstr){
          if (exp(ret)){
            throw exception(source, expstr);
          }
          return ret;
        }
        template <typename _return_t, typename _expression_t>
#endif
        inline static _return_t _throw_if(const source_location& source, _return_t ret, _expression_t exp, const char* expstr){
          if (exp(ret)){
            throw exception(source, expstr);
          }
          return ret;
        }

/// Constructors @{
#if __cpp_lib_source_location >= 201907L
        /** @brief Constructs exception with std::source_location
         * @param Source Source location from std::source_location::current()
         * @param What Error message
         */
        exception(const std::source_location& Source, const std::string& What)
                :  _super_t()
                , _errnum(errno)
                , _source(_convert_source_location(Source))
                , _what(What){}
        
        /** @brief Constructs exception with std::source_location and error code
         * @param Source Source location
         * @param What Error message
         * @param iErr Error code
         */
        exception(const std::source_location& Source, const std::string& What, uint32_t iErr)
                :  _super_t()
                , _errnum(iErr)
                , _source(_convert_source_location(Source))
                , _what(What){}
#endif
        /** @brief Constructs exception with xtd::source_location
         * @param Source Source location
         * @param What Error message
         */
        exception(const source_location& Source, const std::string& What)
                :  _super_t()
                , _errnum(errno)
                , _source(Source)
                , _what(What){}
        /** @brief Constructs exception with xtd::source_location and error code
         * @param Source Source location
         * @param What Error message
         * @param iErr Error code
         */
        exception(const source_location& Source, const std::string& What, uint32_t iErr)
                :  _super_t()
                , _errnum(iErr)
                , _source(Source)
                , _what(What){}
        exception(const exception& src)
                : _super_t()
                , _errnum(src._errnum)
                , _source(src._source)
                , _what(src._what){}
        exception(exception&& src)
                : _super_t()
                , _errnum(src._errnum)
                , _source(std::move(src._source))
                , _what(std::move(src._what)){}
///}@

        ///explanation of the exception
        const char * what() const
#if ((XTD_COMPILER_CLANG | XTD_COMPILER_GNU ) & XTD_COMPILER)
        noexcept
#endif
        override{
          return _what.c_str();
        }

///location in source that caused the exception
        virtual const source_location& location() const
#if ((XTD_COMPILER_CLANG | XTD_COMPILER_GNU) & XTD_COMPILER)
        noexcept
#endif
        {
          return _source;
        }

    private:
#if __cpp_lib_source_location >= 201907L
        static source_location _convert_source_location(const std::source_location& loc){
          return source_location(loc.file_name(), static_cast<int>(loc.line()));
        }
#endif
    protected:
        //_errnum needs to be in the base class before heap allocations such as std::string in _what
        int _errnum;
        const source_location& _source;
        std::string _what;
    };

/// c++ wrapper around legacy errno based errors from the CRT
    class crt_exception : public xtd::exception{
    public:
        template <typename _return_t, typename _expression_t>
        inline static _return_t _throw_if(const xtd::source_location& source, _return_t ret, _expression_t exp, const char* expstr){
          if (exp(ret)){
            throw crt_exception(source, expstr);
          }
          return ret;
        }

        /// constructors
        /// @{
        crt_exception(const source_location& Source, const std::string& What) : xtd::exception(Source, What){
          if (!_errnum){
            return;
          }
          _what += " : ";

#if (XTD_COMPILER_MSVC & XTD_COMPILER)
          _what.resize(_what.size() + 100);
      strerror_s(&_what[_what.size() - 100], 100, _errnum);
      _what.resize(strlen(_what.c_str()));
#else
          _what += strerror(_errnum);
#endif
        }
        crt_exception(const crt_exception& ex) : xtd::exception(ex){}
        crt_exception(crt_exception&& ex) : xtd::exception(std::move(ex)){}
        /// @}
        /**
         * Error number associated with the CRT exception
         * @return thread local _errnum that caused the exception
         */
        int errnum() const { return _errnum; }
    };



}

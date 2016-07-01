/** @file
 generic and special purpose exceptions
  \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/


#pragma once

/**
 @def throw_if(_test, _expression) _throw_if(here(), _test, _expression, #_test)

 \brief A macro that defines throw if.

 \author  David
 \date  6/11/2016

 @param _test       The test.
 @param _expression The expression.
 */

#define throw_if(_test, _expression) _throw_if(here(), _test, _expression, #_test)

namespace xtd{

  /**
   \struct  exception
  
   \brief An exception.
  
   \author  David
   \date  6/11/2016
   */

  struct exception : std::exception{

    template <typename _ReturnT, typename _ExpressionT>
    inline static _ReturnT _throw_if(const xtd::source_location& source, _ReturnT ret, _ExpressionT exp, const char* expstr){
      if (exp(ret)){
        throw exception(source, expstr);
      }
      return ret;
    }

    using _super_t = std::exception;

    exception(const source_location& Source, const std::string& What)
      : _super_t()
      , _source(Source)
      , _what(What){}
    exception(const exception& src)
      : _super_t()
      , _source(src._source)
      , _what(src._what){}
    exception(exception&& src)
      : _super_t()
      , _source(std::move(src._source))
      , _what(std::move(src._what)){}

    virtual const char * what() const noexcept{ return _what.c_str(); }

    virtual const source_location& location() const noexcept{ return _source; }

  protected:
    /** \brief Source for the. */
    const source_location& _source;
    /** \brief The what. */
    std::string _what;
  };

  /**
   \struct  os_exception
  
   \brief Exception for signalling operating system errors.
  
   \author  David
   \date  6/11/2016
   */

  struct os_exception : xtd::exception{
    template <typename _ReturnT, typename _ExpressionT>
    inline static _ReturnT _throw_if(const xtd::source_location& source, _ReturnT ret, _ExpressionT exp, const char* expstr){
      if (exp(ret)){
        throw exception(source, expstr);
      }
      return ret;
    }

    os_exception(const source_location& Source, const std::string& What) : xtd::exception(Source, What){
      if (!errno){
        return;
      }
      _what += " : ";

    #if (XTD_COMPILER_IS_MSVC)
      _what.resize(_what.size() + 100);
      strerror_s(&_what[_what.size() - 100], 100, errno);
      _what.resize(strlen(_what.c_str()));
    #else
      _what += strerror(errno);
    #endif
    }
    os_exception(const os_exception& ex) : xtd::exception(ex){}
    os_exception(os_exception&& ex) : xtd::exception(std::move(ex)){}
  };
}

/** @file
maintains info about locations within source code
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

namespace xtd{

/// @def here() creates an xtd::source_location at the definition site
#define here() xtd::source_location(__FILE__,__LINE__)

  /** Contains information about the location of source code
  Used in error reporting and logging
  */
  class source_location{
  public:
    source_location(const char * File, int Line) : _file(File), _line(Line){}
    source_location(const source_location& src) : _file(src._file), _line(src._line){}
    source_location& operator=(const source_location& src){
      if (this == &src){
        return *this;
      }
      _file = src._file;
      _line = src._line;
      return *this;
    }
    const char * file() const{ return _file; }

    int line() const{ return _line; }

    bool operator==(const source_location& src) const{
      if (_line != src._line){
        return false;
      }
      return 0==strcmp(_file, src._file);
    }

    bool operator<(const source_location& src) const{
      if (strcmp(_file, src._file) < 0){
        return true;
      }
      return _line < src._line;
    }
  private:
    const char * _file;
    int _line;
  };
}

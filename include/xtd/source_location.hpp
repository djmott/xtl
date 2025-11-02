/** @file
  @brief Source code location information
  
  Provides a class to store and manage information about locations within
  source code, useful for error reporting, logging, and debugging.
  
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once
#include <xtd/xtd.hpp>
#include <source_location>
#include <string>
#include <string.h>
namespace xtd{

/** @def here()
 *  @brief Creates a source_location at the call site using C++20 std::source_location
 *  
 *  Uses std::source_location::current() for better standardization.
 *  Falls back to xtd::source_location for compatibility if needed.
 *  
 *  @code{.cpp}
 *  throw exception(here(), "Something went wrong");
 *  @endcode
 */
#if __cpp_lib_source_location >= 201907L
  #define here() std::source_location::current()
#else
  #define here() xtd::source_location(__FILE__,__LINE__)
#endif

  /** @brief Contains information about a location in source code
   * 
   * Used primarily for error reporting and logging. Stores the file name
   * and line number where an object was created.
   * 
   * @note With C++20+, prefer using std::source_location directly via here() macro.
   *       This class maintains backward compatibility.
   */
  class source_location{
  public:
    /** @brief Constructs a source_location
     * @param File Source file name (typically __FILE__)
     * @param Line Line number (typically __LINE__)
     */
    source_location(const char * File, int Line) : _file(File), _line(Line){}
    
    /** @brief Copy constructor
     * @param src Source location to copy
     */
    source_location(const source_location& src) : _file(src._file), _line(src._line){}
    
    /** @brief Copy assignment operator
     * @param src Source location to copy
     * @return Reference to this object
     */
    source_location& operator=(const source_location& src){
      if (this == &src){
        return *this;
      }
      _file = src._file;
      _line = src._line;
      return *this;
    }
    
    /** @brief Gets the source file name
     * @return Pointer to the file name string
     */
    const char * file() const{ return _file; }
    
    /** @brief Sets the source file name
     * @param newval New file name
     */
    void file(const char * newval) { _file = newval; }

    /** @brief Gets the line number
     * @return Line number in the source file
     */
    int line() const{ return _line; }
    
    /** @brief Sets the line number
     * @param newval New line number
     */
    void line(int newval) { _line = newval; }

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

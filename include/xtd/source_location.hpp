/** @file
  @brief Source code location information
  
  Provides a class to store and manage information about locations within
  source code, useful for error reporting, logging, and debugging.
  
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once
#include <xtd/xtd.hpp>
#include <string.h>
namespace xtd{

/** @def here()
 *  @brief Creates an xtd::source_location at the call site
 *  
 *  Macro that expands to a source_location object with the current file
 *  and line number. Useful for error reporting and logging.
 *  
 *  @code{.cpp}
 *  throw exception(here(), "Something went wrong");
 *  @endcode
 */
#define here() xtd::source_location(__FILE__,__LINE__)

  /** @brief Contains information about a location in source code
   * 
   * Used primarily for error reporting and logging. Stores the file name
   * and line number where an object was created.
   * 
   * @note On C++20, consider using std::source_location for better
   *       standardization, but this class maintains C++17 compatibility.
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

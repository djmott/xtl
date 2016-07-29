/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

namespace xtd{
  namespace nlp{
    class document{
    public:
      document(const xtd::string text) : _text(text){}

      const xtd::string& const text() const{ return _text; }
    protected:
      xtd::string _text;

    };
  }
}
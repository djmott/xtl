#include "xtd/xtd.hpp"
#include <memory>
#include <deque>
#include <string>
#include <cassert>

namespace xtd {
  namespace parseex {

    /** Declares a string terminal with the value equal to the name.
    Must be used at namespace scope
    @param x name and value of the terminal
    */
//#define STRING_(x) namespace _{ char x[] = #x; } using x = xtd::parse::string< decltype(_::x), _::x>;
    /**
    @def STRING(_name, _value)
    Declares a string terminal
    Must be used at namespace scope
    @param _name name of the terminal
    @param _value string value of the terminal. Must be enclosed in double quotes.
    */
//#define STRING(_name, _value) namespace _{ char _name[] = _value; } using _name = xtd::parse::string< decltype(_::_name), _::_name>;


    /**
    @def CHARACTER_(_name, _value)
    Declares a single character terminal.
    Must be used at namespace scope.
    @param _name name of the terminal.
    @param _value value of the terminal. Must be a single character enclosed in single quotes.
    */
#define CHARACTER_(_name, _value) using _name = xtd::parseex::character<_value>

    /**
    @def CHARACTERS_(_name, _first, _last)
    Declares a range of character from _first to _last (inclusive)
    Must be used at namespace scope.
    @param _name name of the terminal.
    @param _first value of the first terminal. Must be a single character enclosed in single quotes.
    */
//#define CHARACTERS_(_name, _first, _last) using _name = xtd::parse::characters<_first, _last>

    /**
    @def REGEX(_name, _value)
    Declares a regular expression terminal.
    Must be used at namespace scope.
    @param _name name of the terminal.
    @param _value regular expression of the terminal.
    */
//#define REGEX(_name, _value) namespace _{ char _name[] = _value; } using _name = xtd::parse::regex< decltype(_::_name), _::_name>;

    template <char> struct character;
    template <typename ...> struct and_;
    template <typename ...> struct or_;
    template <typename> struct zero_or_more_;
    template <typename> struct zero_or_one_;
    template <typename> struct one_or_more_;

    struct rule_base : std::enable_shared_from_this<rule_base>{

      using ptr = std::shared_ptr<rule_base>;
      using weak_ptr = std::weak_ptr<rule_base>;
      using deque = std::deque<ptr>;

      deque::size_type size() const { return _items.size(); }

      ptr& operator[](deque::size_type index) { return _items[index]; }
      const ptr& operator[](deque::size_type index) const { return _items[index]; }

      virtual bool is_a(const std::type_info&) = 0;

      ptr parent() { return _parent.lock(); }
      ptr parent() const { return _parent.lock(); }

    protected:
      template <typename ... _arg_ts> rule_base(_arg_ts...oArgs) : _items{ std::forward(oArgs)... } {}
      deque _items;
      weak_ptr _parent;
    };

    struct error_base {
      using ptr = std::shared_ptr<error_base>;
    };

    template <typename impl_t, typename decl_t> struct rule : rule_base {

      bool is_a(const std::type_info& oType) override {
        if (typeid(impl_t) == oType) return true;
        return (typeid(decl_t) == oType);
      }
    };

    template <char _value> struct character {
      static const char value = _value;
    };


    namespace _ {
      struct context{
        context(std::string::const_iterator oBegin, std::string::const_iterator oEnd) : begin(oBegin), end(oEnd){}
        rule_base::ptr rule;
        std::string::const_iterator begin;
        std::string::const_iterator end;
      };

      template <typename> struct parse_helper;

      template <typename, typename ...> struct unpack;
      template <typename _ty> struct unpack<_ty>{
        template <typename ... _arg_ts> static _ty create(rule_base::deque&, _arg_ts&&...oArgs) {
          return _ty(std::forward<_arg_ts>(oArgs)...);
        }
      };
      template <typename _ty, typename _head_t, typename ... _tail_ts> 
      struct unpack<_ty, _head_t, _tail_ts...> {
        template <typename ... _arg_ts> static _ty create(rule_base::deque& oRules, _arg_ts&&...oArgs) {
          return unpack<_ty, _tail_ts...>::create(oRules, oRules.pop_front(), std::forward<_arg_ts>(oArgs)...);
        }
      };

      template <typename> struct and_parse_helper;
      template <> struct and_parse_helper<and_<>>{
        static bool parse(const context&) { return true; }
      };

      template <typename _head_t, typename ... _tail_t>
      struct and_parse_helper<and_<_head_t, _tail_t...>> {
        static bool parse(context& oOuter) {
          context oContext(oOuter);
          if (!parse_helper<_head_t>::parse(oContext)) {

          }
        }
      };


      template <typename _head_t, typename ... _tail_t> 
      struct parse_helper<and_<_head_t, _tail_t...>> {
        static bool parse(context& oOuter) {
          context oContext(oOuter);
          if (!parse_helper<_head_t>::parse(oContext)) {

          }
        }
      };

      template <char _ch> struct parse_helper<character<_ch>>{
        static bool parse(context& oOuter) {
          assert(false);
          return false;

        }
      };

    }


    template <typename rule_t>
    rule_base::ptr parse(const std::string& str) {
      _::context oContext(str.begin(), str.end());
      return (_::parse_helper<rule_t>::parse(oContext) ?
        oContext.rule :
        rule_base::ptr());
    }

  }
}
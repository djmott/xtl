/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
Simple text parsing library
*/

#pragma once

#include <xtd.hpp>
#include <xtd/meta.hpp>

#include <memory>
#include <vector>
#include <string>
#include <regex>

namespace xtd{

  /** @namespace xtd::parse
  Declarations and definitions of the parse library
  @defgroup Parsing Text parsng library
  @{*/
  namespace parse{


/** Declares a string terminal with the value equal to the name.
 Must be used at namespace scope
 @param x name and value of the terminal
 */
#define STRING_(x) \
      namespace _{ char x[] = #x; } \
      using x = xtd::parse::string< decltype(_::x), _::x>;
/**
 @def STRING(_name, _value)
 Declares a string terminal
 Must be used at namespace scope
 @param _name name of the terminal
 @param _value string value of the terminal. Must be enclosed in double quotes.
 */
#define STRING(_name, _value) \
      namespace _{ char _name[] = _value; } \
      using _name = xtd::parse::string< decltype(_::_name), _::_name>;


/**
 @def CHARACTER_(_name, _value)
 Declares a single character terminal.
 Must be used at namespace scope.
 @param _name name of the terminal.
 @param _value value of the terminal. Must be a single character enclosed in single quotes.
 */
#define CHARACTER_(_name, _value) \
  namespace _{} \
  using _name = xtd::parse::character<_value>

/**
 @def REGEX(_name, _value)
 Declares a regular expression terminal.
 Must be used at namespace scope.
 @param _name name of the terminal.
 @param _value regular expression of the terminal.
 */
#define REGEX(_name, _value) \
      namespace _{ char _name[] = _value; } \
      using _name = xtd::parse::regex< decltype(_::_name), _::_name>;


    /** Base class of both rules and terminals
     Though rules and terminals are technically different they share the rule_base ancestor in XTL to have a cleaner object model and simpler parsing algorithms.
     Its an abstract interface of parsable text or containers of same satisfying various rules (one or more, zero or more, exactly one, etc).
     */
    struct rule_base{
      using pointer_type = std::shared_ptr<rule_base>;
      using vector_type = std::vector<pointer_type>;

      /** Constructor
      @param[in] _ChildRuleTs list of child rules that successfully parsed to satisfy the current parse rule. The child rules are stored in a local variable and accessible via the items() member.
      */
      template <typename ... _ChildRuleTs>
      explicit rule_base(_ChildRuleTs&& ... oChildRules) : _Items{ std::forward<_ChildRuleTs>(oChildRules)... }{}

      /** Determines if the interface is implemented by a concrete type
       @param[in] oType type to test
      @returns true if the implementation's is the specified type
      */
      virtual bool isa(const std::type_info& oType) const = 0;
      /** Gets the type info of the concrete implementation */
      virtual const std::type_info& type() const = 0;
      /** Accessor for child parse elements
      @returns a vector of the parsed child rules and terminals
      */
      const vector_type& items() const{ return _Items; }

    protected:
      vector_type _Items;
    };


    /** Curiously recurring template pattern to simplify creation of rule_base implementations
    Rules and terminals implement the rule_base interface through this pattern. It provides boiler plate pointer_type, isa() and type() members.
    @tparam _DeclT The type declaration. Permits access to the implementation from the rule class template. Declaration pass their name in the form of: class SomeRule : rule<SomeRule>
    @tparam _ImplT The declaration used by the parsing algorithms. The library instantiates type of _DeclT when _ImplT successfully parses.
    @startuml
    rule_base <|-- rule
    @enduml
    */
    template <typename _DeclT, typename _ImplT = _DeclT>
    struct rule : rule_base{
      using decl_type = _DeclT;
      using impl_type = _ImplT;
      using rule_type = rule<_DeclT, _ImplT>;

      template <typename ... _ChildRuleTs>
      explicit rule(_ChildRuleTs&& ... oChildRules) : rule_base(std::forward<_ChildRuleTs>(oChildRules)...){}

      virtual bool isa(const std::type_info& oType) const override{
        return (typeid(rule) == oType) || (typeid(decl_type) == oType) || (typeid(impl_type) == oType) || (typeid(rule_base) == oType);
      }

      virtual const std::type_info& type() const override{
        return typeid(_DeclT);
      }

    };


    /** Represents a parse algorithm where all specified elements parse are contiguously present in the input stream
     All the terminals and rules listed in the parameter pack must parse successfully to satisfy the parse rule
     @tparam ... list of child rules and terminals to parse
    */
    template <typename ...> struct and_;
#if (!DOXY_INVOKED)
    template <> struct and_<> : rule<and_<>>{
      using _super_t = rule<and_<>>;
      template <typename ... _ChildRuleTs>
      explicit and_(_ChildRuleTs&& ... oChildRules) : _super_t(std::forward<_ChildRuleTs>(oChildRules)...){}
    };

    template <typename _HeadT, typename ... _TailT> struct and_<_HeadT, _TailT...> : rule<and_<_HeadT, _TailT...>>{
      using _super_t = rule<and_<_HeadT, _TailT...>>;
      template <typename ... _ChildRuleTs>
      explicit and_(_ChildRuleTs&& ... oChildRules) : _super_t(std::forward<_ChildRuleTs>(oChildRules)...){}

    };
#endif

    /**Represents a parse algorithm where one of the listed elements is present in the input stream
    One of the terminals or rules listed in the parameter pack must parse successfully to satisfy the parse rule
    @tparam ... list of child rules and terminals to parse
    */
    template <typename ...> struct or_;
#if (!DOXY_INVOKED)
    template <> struct or_<> : rule<or_<>>{
      using _super_t = rule<or_<>>;
      template <typename ... _ChildRuleTs>
      explicit or_(_ChildRuleTs&& ... oChildRules) : _super_t(std::forward<_ChildRuleTs>(oChildRules)...){}
    };

    template <typename _HeadT, typename ... _TailT> struct or_<_HeadT, _TailT...> : rule<or_<_HeadT, _TailT...>>{
      using _super_t = rule<or_<_HeadT, _TailT...>>;
      template <typename ... _ChildRuleTs>
      explicit or_(_ChildRuleTs&& ... oChildRules) : _super_t(std::forward<_ChildRuleTs>(oChildRules)...){}
    };
#endif


    /**
    Represents a parse algorithm where where the terminal or rule is repeated one or more times in the input stream
    The specified item must parse successfully at least once to satisfy the parse rule. Consecutive occurrences are attempted and permitted if present.
    @tparam _Ty The rule or terminal to parse
    */
    template <typename _Ty> struct one_or_more_ : rule<one_or_more_<_Ty>>{
      using _super_t = rule<one_or_more_<_Ty>>;
      template <typename ... _ChildRuleTs>
      explicit one_or_more_(_ChildRuleTs&& ... oChildRules) : _super_t(std::forward<_ChildRuleTs>(oChildRules)...){}
    };



    /**
    Represents a parse algorithm where the terminal or rule repeats zero or more times in the input stream
    Always successfully parses whether or not the element parses. Data in the input stream is consumed if present.
    @tparam _Ty the rule or terminal to parse
    */
    template <typename _Ty> struct zero_or_more_ : rule<zero_or_more_<_Ty>>{
      using _super_t = rule<zero_or_more_<_Ty>>;
      template <typename ... _ChildRuleTs>
      explicit zero_or_more_(_ChildRuleTs&& ... oChildRules) : _super_t(std::forward<_ChildRuleTs>(oChildRules)...){}
    };


    /** Represents a parse algorithm where the terminal or rule repeats exactly zero or one time in the input stream
    Always successfully parses whether or not the element parses.
    @tparam _Ty the rule or terminal to parse
    */
    template <typename _Ty> struct zero_or_one_ : rule<zero_or_one_<_Ty>>{
      using _super_t = rule<zero_or_one_<_Ty>>;
      template <typename ... _ChildRuleTs>
      explicit zero_or_one_(_ChildRuleTs&& ... oChildRules) : _super_t(std::forward<_ChildRuleTs>(oChildRules)...){}
    };


    /** list of whitespace characters to ignore in the input stream.
    Whitespace characters are parsed and discarded from the input stream when encountered between rules or terminals.
    A typical whitespace declaration for written language might be: whitespace<'\r', '\n', '\t', ' '>;
    @tparam _Ch... list of characters to ignore. 
    */
    template <char..._Ch> struct whitespace{
      using whitespace_type = whitespace<_Ch...>;
    };


    /** String terminal parsing algorithm.
    This template is infrequently used directly. The STRING and STRING_ macros are provided to declare string terminals.
    */
    template <typename _Ty, _Ty &> struct string;
  #if (!DOXY_INVOKED)
    template <size_t _len, char(&_str)[_len]> struct string<char[_len], _str> : rule<string<char[_len], _str>>{

      using _super_t = rule<string<char[_len], _str>>;

      static constexpr size_t length = _len;

      string() : _super_t(){}

    };
  #endif


    /** Character terminal parsing algorithm.
    This template is infrequently used directly. The CHARACTER macro is provided to declare a character terminal.
    */
    template <char _value> struct character : rule<character<_value>>{};

    /** regular expression parsing algorithm.
    This template is infrequently used directly. The REGEX macro is provided to declare a regular expression terminal.
    */
    template <typename _Ty, _Ty &> struct regex;
  #if (!DOXY_INVOKED)
    template <size_t _len, char(&_str)[_len]> struct regex<char[_len], _str> : rule<regex<char[_len], _str>>{

      using _super_t = rule<regex<char[_len], _str>>;

      static constexpr size_t length = _len;

      explicit regex(const std::string& newval) : _super_t(), _value(newval){}
      const std::string& value() const{ return _value; }

    protected:
      std::string _value;
    };
  #endif

#if (!DOXY_INVOKED)
    /**@internal
    Internal parsing algorithms are specializations of parse_helper that partially specialize the built in rules and terminals
     */

    namespace _{

      template <typename _DeclT, typename _ImplT, bool _IgnoreCase, typename _WhitespaceT> struct parse_helper;
      
      ///case sensitive string
      template <typename _DeclT, size_t _len, char(&_str)[_len], typename _WhitespaceT>
      struct parse_helper<_DeclT, parse::string<char[_len], _str>, false, _WhitespaceT>{
        template <typename _IteratorT> static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end){
          _IteratorT oCurr = begin;

          parse_helper< _WhitespaceT, void, true, void>::parse(oCurr, end);

          rule_base::pointer_type oRet(nullptr);
          for (size_t i = 0; (i < (_len-1)) && (oCurr < end); ++i, ++oCurr){
            if (_str[i] != *oCurr){
              return rule_base::pointer_type(nullptr);
            }
          }

          parse_helper< _WhitespaceT, void, true, void>::parse(oCurr, end);

          begin = oCurr;
          return rule_base::pointer_type(new _DeclT);
        }
      };
      

      ///ignore case string
      template <typename _DeclT, size_t _len, char(&_str)[_len], typename _WhitespaceT>
      struct parse_helper<_DeclT, parse::string<char[_len], _str>, true, _WhitespaceT>{
        
        template <typename _IteratorT> 
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end){
          _IteratorT oCurr = begin;

          parse_helper< _WhitespaceT, void, true, void>::parse(oCurr, end);

          rule_base::pointer_type oRet(nullptr);
          for (size_t i = 0; (i < _len-1) && (oCurr < end); ++i, ++oCurr){
            if (tolower(_str[i]) != tolower(*oCurr)){
              return rule_base::pointer_type(nullptr);
            }
          }
          ///ensure there's an identifiable separation between terminals. this should be done differently
          if (oCurr < end && isalnum(*oCurr) && isalnum(_str[_len - 1])){
            return rule_base::pointer_type(nullptr);
          }
          
          parse_helper< _WhitespaceT, void, true, void>::parse(oCurr, end);

          begin = oCurr;
          return rule_base::pointer_type(new _DeclT);
        }
        
      };

      ///regex
      template <typename _DeclT, size_t _len, char(&_str)[_len], bool _IgnoreCase, typename _WhitespaceT>
      struct parse_helper<_DeclT, parse::regex<char[_len], _str>, _IgnoreCase, _WhitespaceT>{

        template <typename _IteratorT>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end){
          _IteratorT oCurr = begin;

          parse_helper< _WhitespaceT, void, true, void>::parse(oCurr, end);

          static std::regex oRE(_str, std::regex_constants::ECMAScript | std::regex_constants::nosubs | std::regex_constants::optimize | (_IgnoreCase ? std::regex_constants::icase : std::regex_constants::optimize));
          std::match_results<std::string::iterator> oMatch;
          if (!std::regex_search(oCurr, end, oMatch, oRE, std::regex_constants::match_continuous | std::regex_constants::format_first_only)){
            return rule_base::pointer_type(nullptr);
          }
          oCurr += oMatch[0].length();

          ///ensure there's an identifiable separation between terminals. this should be done differently
          if (oCurr < end && isalnum(*oCurr) && isalnum(_str[_len - 1])){
            return rule_base::pointer_type(nullptr);
          }


          parse_helper< _WhitespaceT, void, true, void>::parse(oCurr, end);
          
          begin = oCurr;
          return rule_base::pointer_type(new _DeclT(oMatch[0].str()));

        }

      };

      ///whitespace
      template <bool _IgnoreCase>
      struct parse_helper<whitespace<>, void, _IgnoreCase, void>{
        template <typename _IteratorT>
        static bool parse(_IteratorT&, _IteratorT&){ return false; }
      };

      template <char _HeadCH, char... _TailCH, bool _IgnoreCase>
      struct parse_helper<whitespace<_HeadCH, _TailCH...>, void, _IgnoreCase, void>{
        template <typename _IteratorT>
        static bool parse(_IteratorT& begin, _IteratorT& end){
          _IteratorT oCurr = begin;
          
          for (;oCurr < end;){
            if (*oCurr == _HeadCH){
              oCurr++;
              continue;
            }
            if (parse_helper<whitespace<_TailCH...>, void, _IgnoreCase, void>::parse(oCurr, end)){
              continue;
            }
            break;
          }
          begin == oCurr;
          return false;
        }
      };

      //character
      template <typename _DeclT, char _Ch, typename _WhitespaceT>
      struct parse_helper<_DeclT, character<_Ch>, true, _WhitespaceT>{

        template <typename _IteratorT>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end){
          _IteratorT oCurr = begin;
          parse_helper< _WhitespaceT, void, true, void>::parse(oCurr, end);
          if (lower_case<char, _Ch>::value != tolower(*oCurr)){
            return rule_base::pointer_type(nullptr);
          }
          ++oCurr;
          parse_helper< _WhitespaceT, void, true, void>::parse(oCurr, end);
          begin = oCurr;
          return rule_base::pointer_type(new character<_Ch>);
        }

      };

      template <typename _DeclT, char _Ch, typename _WhitespaceT>
      struct parse_helper<_DeclT, character<_Ch>, false, _WhitespaceT>{

        template <typename _IteratorT>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end){
          _IteratorT oCurr = begin;
          parse_helper< _WhitespaceT, void, true, void>::parse(oCurr, end);
          if (_Ch != oCurr[0]){
            return rule_base::pointer_type(nullptr);
          }
          ++oCurr;
          parse_helper< _WhitespaceT, void, true, void>::parse(oCurr, end);
          begin = oCurr;
          return rule_base::pointer_type(new character<_Ch>);
        }

      };


      //and
      template <typename _DeclT, bool _IgnoreCase, typename _WhitespaceT >
      struct parse_helper < _DeclT, parse::and_<>, _IgnoreCase, _WhitespaceT>{

        template <typename _IteratorT, typename ... _ChildRuleTs>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end, _ChildRuleTs ... oChildRules){
          return rule_base::pointer_type(new _DeclT(std::forward<_ChildRuleTs>(oChildRules)...));
        }

      };

      template <typename _DeclT, typename _HeadT, typename ... _TailT, bool _IgnoreCase, typename _WhitespaceT >
      struct parse_helper < _DeclT, parse::and_<_HeadT, _TailT...>, _IgnoreCase, _WhitespaceT>{

        template <typename _IteratorT, typename ... _ChildRuleTs>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end, _ChildRuleTs ... oChildRules){
          _IteratorT oBegin = begin;
          auto oItem = parse_helper<_HeadT, typename _HeadT::impl_type, _IgnoreCase, _WhitespaceT>::parse(oBegin, end);
          if (!oItem){
            return oItem;
          }
          oItem = parse_helper<_DeclT, parse::and_<_TailT...>, _IgnoreCase, _WhitespaceT>::parse(oBegin, end, std::forward<_ChildRuleTs>(oChildRules)..., oItem);
          if (oItem){
            begin = oBegin;
          }
          return oItem;
        }

      };


      ///or
      template <typename _DeclT, bool _IgnoreCase, typename _WhitespaceT >
      struct parse_helper < _DeclT, parse::or_<>, _IgnoreCase, _WhitespaceT>{

        template <typename _IteratorT, typename ... _ChildRuleTs>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end, _ChildRuleTs ... oChildRules){
          return rule_base::pointer_type(nullptr);
        }

      };

      template <typename _DeclT, typename _HeadT, typename ... _TailT, bool _IgnoreCase, typename _WhitespaceT >
      struct parse_helper < _DeclT, parse::or_<_HeadT, _TailT...>, _IgnoreCase, _WhitespaceT>{

        template <typename _IteratorT, typename ... _ChildRuleTs>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end, _ChildRuleTs ... oChildRules){
          _IteratorT oBegin = begin;
          auto oItem = parse_helper<_HeadT, typename _HeadT::impl_type, _IgnoreCase, _WhitespaceT>::parse(oBegin, end);
          if (oItem){
            begin = oBegin;
            return rule_base::pointer_type(new _DeclT(oItem));
          }
          oItem = parse_helper<_DeclT, parse::or_<_TailT...>, _IgnoreCase, _WhitespaceT>::parse(oBegin, end, std::forward<_ChildRuleTs>(oChildRules)..., oItem);
          if (oItem){
            begin = oBegin;
          }
          return oItem;
        }

      };



      ///zero or more
      template <typename _DeclT, typename _Ty, bool _IgnoreCase, typename _WhitespaceT >
      struct parse_helper < _DeclT, parse::zero_or_more_<_Ty>, _IgnoreCase, _WhitespaceT>{
        
        template <typename _IteratorT, typename ... _ChildRuleTs>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end, _ChildRuleTs ... oChildRules){
          _IteratorT oBegin = begin;
          auto oItem = parse_helper<_Ty, typename _Ty::impl_type, _IgnoreCase, _WhitespaceT>::parse(oBegin, end);
          if (!oItem){
            return rule_base::pointer_type(new _DeclT(std::forward<_ChildRuleTs>(oChildRules)...));
          };
          return parse_helper<_DeclT, parse::zero_or_more_<_Ty>, _IgnoreCase, _WhitespaceT>::parse(oBegin, end, std::forward<_ChildRuleTs>(oChildRules)..., oItem);
        }

      };
    }
    ///@}
#endif
  }

  template <typename _RuleT, bool _IgnoreCase = false, typename _WhitespaceT = xtd::parse::whitespace<>> struct parser {

    template <typename _IteratorT> static parse::rule_base::pointer_type parse(_IteratorT begin, _IteratorT end) {
      _IteratorT oBegin = begin;
      _IteratorT oEnd = end;
      return parse::_::parse_helper<_RuleT, typename _RuleT::impl_type, _IgnoreCase, _WhitespaceT>::parse(oBegin, oEnd);
    }

  };

}


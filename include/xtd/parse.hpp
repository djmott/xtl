/** @file
text parsing and AST generation
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
@example example_parse1.cpp
@example example_parse2.cpp
 */

#pragma once

#include <xtd/xtd.hpp>

#include <memory>
#include <regex>

#include <xtd/meta.hpp>

namespace xtd{

  /// @addtogroup Parsing
  /// @{
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
  using _name = xtd::parse::character<_value>

    /**
 @def CHARACTERS_(_name, _first, _last)
 Declares a range of character from _first to _last (inclusive)
 Must be used at namespace scope.
 @param _name name of the terminal.
 @param _first value of the first terminal. Must be a single character enclosed in single quotes.
 */
#define CHARACTERS_(_name, _first, _last) \
  using _name = xtd::parse::characters<_first, _last>

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
    class rule_base{
    public:
      using pointer_type = std::shared_ptr<rule_base>;
      using vector_type = std::vector<pointer_type>;

      /** Constructor
      @param[in] oChildRules list of child rules that successfully parsed to satisfy the current parse rule. The child rules are stored in a local variable and accessible via the items() member.
      */
      template <typename ... _ChildRuleTs>
      explicit rule_base(_ChildRuleTs&& ... oChildRules) : _Items{ std::forward<_ChildRuleTs>(oChildRules)... }{}

      virtual ~rule_base() = default;
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
    */
    template <typename _DeclT, typename _ImplT = _DeclT>
    class rule : public rule_base{
    public:
      using decl_type = _DeclT;
      using impl_type = _ImplT;
      using rule_type = rule<_DeclT, _ImplT>;

      template <typename ... _ChildRuleTs>
      explicit rule(_ChildRuleTs&& ... oChildRules) : rule_base(std::forward<_ChildRuleTs>(oChildRules)...){}

      virtual ~rule() = default;

      virtual bool isa(const std::type_info& oType) const override{
        return (typeid(rule) == oType) || (typeid(decl_type) == oType) || (typeid(impl_type) == oType) || (typeid(rule_base) == oType);
      }

      virtual const std::type_info& type() const override{
        return typeid(_DeclT);
      }

    };

    /** Represents a parse algorithm where none of the specified elements should parse.
     * Parsing fails if any constituent parsers succeed
     * @tparam ... list of child rules and terminals that should fail to parse for this rule to succeed
     */
    template <typename ...> class not_;

#if (!DOXY_INVOKED)
    template <> class not_<> : public rule<not_<>>{
    public:
      using _super_t = rule<not_<>>;
      template <typename ... _ChildRuleTs>
      explicit not_(_ChildRuleTs&& ... oChildRules) : _super_t(std::forward<_ChildRuleTs>(oChildRules)...){}
    };

    template <typename _HeadT, typename ... _TailT> class not_<_HeadT, _TailT...> : public rule<not_<_HeadT, _TailT...>>{
    public:
      using _super_t = rule<not_<_HeadT, _TailT...>>;
      template <typename ... _ChildRuleTs>
      explicit not_(_ChildRuleTs&& ... oChildRules) : _super_t(std::forward<_ChildRuleTs>(oChildRules)...){}
    };
#endif

    /** Represents a parse algorithm where all specified elements parse are contiguously present in the input stream
     All the terminals and rules listed in the parameter pack must parse successfully to satisfy the parse rule
     @tparam ... list of child rules and terminals to parse
    */
    template <typename ...> class and_;
#if (!DOXY_INVOKED)
    template <> class and_<> : public rule<and_<>>{
    public:
      using _super_t = rule<and_<>>;
      template <typename ... _ChildRuleTs>
      explicit and_(_ChildRuleTs&& ... oChildRules) : _super_t(std::forward<_ChildRuleTs>(oChildRules)...){}
    };

    template <typename _HeadT, typename ... _TailT> class and_<_HeadT, _TailT...> : public rule<and_<_HeadT, _TailT...>>{
    public:
      using _super_t = rule<and_<_HeadT, _TailT...>>;
      template <typename ... _ChildRuleTs>
      explicit and_(_ChildRuleTs&& ... oChildRules) : _super_t(std::forward<_ChildRuleTs>(oChildRules)...){}

    };
#endif

    /**Represents a parse algorithm where one of the listed elements is present in the input stream
    One of the terminals or rules listed in the parameter pack must parse successfully to satisfy the parse rule
    @tparam ... list of child rules and terminals to parse
    */
    template <typename ...> class or_;
#if (!DOXY_INVOKED)
    template <> class or_<> : public rule<or_<>>{
    public:
      using _super_t = rule<or_<>>;
      template <typename ... _ChildRuleTs>
      explicit or_(_ChildRuleTs&& ... oChildRules) : _super_t(std::forward<_ChildRuleTs>(oChildRules)...){}
    };

    template <typename _HeadT, typename ... _TailT> class or_<_HeadT, _TailT...> : public rule<or_<_HeadT, _TailT...>>{
    public:
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
    template <typename _Ty> class one_or_more_ : public rule<one_or_more_<_Ty>>{
    public:
      using _super_t = rule<one_or_more_<_Ty>>;
      template <typename ... _ChildRuleTs>
      explicit one_or_more_(_ChildRuleTs&& ... oChildRules) : _super_t(std::forward<_ChildRuleTs>(oChildRules)...){}
    };



    /**
    Represents a parse algorithm where the terminal or rule repeats zero or more times in the input stream
    Always successfully parses whether or not the element parses. Data in the input stream is consumed if present.
    @tparam _Ty the rule or terminal to parse
    */
    template <typename _Ty> class zero_or_more_ : public rule<zero_or_more_<_Ty>>{
    public:
      using _super_t = rule<zero_or_more_<_Ty>>;
      template <typename ... _ChildRuleTs>
      explicit zero_or_more_(_ChildRuleTs&& ... oChildRules) : _super_t(std::forward<_ChildRuleTs>(oChildRules)...){}
    };


    /** Represents a parse algorithm where the terminal or rule repeats exactly zero or one time in the input stream
    Always successfully parses whether or not the element parses.
    @tparam _Ty the rule or terminal to parse
    */
    template <typename _Ty> class zero_or_one_ : public rule<zero_or_one_<_Ty>>{
    public:
      using _super_t = rule<zero_or_one_<_Ty>>;
      template <typename ... _ChildRuleTs>
      explicit zero_or_one_(_ChildRuleTs&& ... oChildRules) : _super_t(std::forward<_ChildRuleTs>(oChildRules)...){}
    };


    /** list of whitespace characters to ignore in the input stream.
    Whitespace characters are parsed and discarded from the input stream when encountered between rules or terminals.
    A typical whitespace declaration for written language might be: whitespace<'\\r', '\\n', '\\t', ' '>;
    @tparam _Ch... list of characters to ignore.
    */
    template <char..._Ch> class whitespace{
    public:
      using whitespace_type = whitespace<_Ch...>;
    };


    /** String terminal parsing algorithm.
    This template is infrequently used directly. The STRING and STRING_ macros are provided to declare string terminals.
    */
    template <typename _Ty, _Ty &> class string;
#if (!DOXY_INVOKED)
    template <size_t _len, char(&_str)[_len]> class string<char[_len], _str> : public rule<string<char[_len], _str>>{
    public:
      using _super_t = rule<string<char[_len], _str>>;
      static constexpr size_t length = _len;
      string() = default;
    };
#endif


    /** Character terminal parsing algorithm.
    This template is infrequently used directly. The CHARACTER macro is provided to declare a character terminal.
    */
    template <char _value> class character : public rule<character<_value>>{};

    /** Character range terminal parsing algorithm.
    This template is infrequently used directly. The CHARACTERS macro is provided to declare a character terminal.
    */
    template <char _first, char _last> class characters : public rule<characters<_first, _last>>{
    public:
      using _super_t = rule<characters<_first, _last>>;
      explicit characters(char ch) : _super_t(), _ch(ch){}
      char value() const { return _ch; }
    protected:
      char _ch;
    };

    /** regular expression parsing algorithm.
    This template is infrequently used directly. The REGEX macro is provided to declare a regular expression terminal.
    */
    template <typename _Ty, _Ty &> class regex;
#if (!DOXY_INVOKED)
    template <size_t _len, char(&_str)[_len]> class regex<char[_len], _str> : public rule<regex<char[_len], _str>>{
    public:
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

      template <typename _DeclT, typename _ImplT, bool _IgnoreCase, typename _WhitespaceT> class parse_helper;

      ///case sensitive string
      template <typename _DeclT, size_t _len, char(&_str)[_len], typename _WhitespaceT>
      class parse_helper<_DeclT, parse::string<char[_len], _str>, false, _WhitespaceT>{
      public:
        template <typename _IteratorT> static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end){
          if (begin >= end) return rule_base::pointer_type(nullptr);
          _IteratorT oCurr = begin;

          parse_helper< _WhitespaceT, void, true, void>::parse(oCurr, end);

          rule_base::pointer_type oRet(nullptr);
          for (size_t i = 0; (i < (_len-1)) && (oCurr < end); ++i, ++oCurr){
            if (_str[i] != *oCurr){
              return rule_base::pointer_type(nullptr);
            }
          }

          /*
          Problem: The string comparison algorithms compare character by character and skip any leading or trailing whitespace between terminals.
            Rules 'ABC' + 'XYZ' maybe defined expecting the two terminals be separated by whitespace but this algorithm could parse the string 'ABCXYZ' as two separate terminals.
            There's a number of traditional approaches to solving this such as tokenizing before parsing or creating parse tables.
            A proper handling would compound the complexity of this library beyond it's intended scope, there are plenty of complex parsers around.
            Currently, the last character parsed is checked against the next character in the stream to see if they're of the same 'class' and fail if so.
            This isn't ideal because it maybe perfectly valid in some grammars to expect 'ABCXYZ' to appear in the input stream yet successfully parse into independent terminals.
            This library assumes contiguous alpha-numeric terminals constitute a single terminal so the input stream of 'ABCXYZ' will fail to parse without grammar definition trickery
          */
          if (oCurr < end && isalnum(*oCurr) && isalnum(_str[_len - 2])){
            return rule_base::pointer_type(nullptr);
          }
          parse_helper< _WhitespaceT, void, true, void>::parse(oCurr, end);

          begin = oCurr;
          return rule_base::pointer_type(new _DeclT);
        }
      };


      ///ignore case string
      template <typename _DeclT, size_t _len, char(&_str)[_len], typename _WhitespaceT>
      class parse_helper<_DeclT, parse::string<char[_len], _str>, true, _WhitespaceT>{
      public:

        template <typename _IteratorT>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end){
          if (begin >= end) return rule_base::pointer_type(nullptr);
          _IteratorT oCurr = begin;

          parse_helper< _WhitespaceT, void, true, void>::parse(oCurr, end);

          rule_base::pointer_type oRet(nullptr);
          for (size_t i = 0; (i < _len-1) && (oCurr < end); ++i, ++oCurr){
            if (tolower(_str[i]) != tolower(*oCurr)){
              return rule_base::pointer_type(nullptr);
            }
          }
          ///ensure there's an identifiable separation between terminals. this should be done differently
          if (oCurr < end && isalnum(*oCurr) && isalnum(_str[_len - 2])){
            return rule_base::pointer_type(nullptr);
          }

          parse_helper< _WhitespaceT, void, true, void>::parse(oCurr, end);

          begin = oCurr;
          return rule_base::pointer_type(new _DeclT);
        }

      };

      ///regex
      template <typename _DeclT, size_t _len, char(&_str)[_len], bool _IgnoreCase, typename _WhitespaceT>
      class parse_helper<_DeclT, parse::regex<char[_len], _str>, _IgnoreCase, _WhitespaceT>{
      public:

        template <typename _IteratorT>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end){
          if (begin >= end) return rule_base::pointer_type(nullptr);
          _IteratorT oCurr = begin;

          parse_helper< _WhitespaceT, void, true, void>::parse(oCurr, end);

          std::regex_constants::syntax_option_type iFlags = std::regex_constants::optimize;
          if (_IgnoreCase) iFlags |=  std::regex_constants::icase;

          static std::regex oRE(_str, iFlags);
          std::match_results<std::string::iterator> oMatch;
          if (!std::regex_search(oCurr, end, oMatch, oRE, std::regex_constants::match_continuous)){
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
      class parse_helper<whitespace<>, void, _IgnoreCase, void>{
      public:
        template <typename _IteratorT>
        static bool parse(_IteratorT&, _IteratorT&){ return false; }
      };

      template <char _HeadCH, char... _TailCH, bool _IgnoreCase>
      class parse_helper<whitespace<_HeadCH, _TailCH...>, void, _IgnoreCase, void>{
      public:
        template <typename _IteratorT>
        static bool parse(_IteratorT& begin, _IteratorT& end){
          _IteratorT oCurr = begin;

          while (oCurr < end){
            if (*oCurr == _HeadCH){
              oCurr++;
            }else if (parse_helper<whitespace<_TailCH...>, void, _IgnoreCase, void>::parse(oCurr, end)){
              //do nothing
            }else{
              break;
            }
          }
          begin = oCurr;
          return false;
        }
      };

      //character
      template <typename _DeclT, char _Ch, typename _WhitespaceT>
      class parse_helper<_DeclT, character<_Ch>, true, _WhitespaceT>{
      public:

        template <typename _IteratorT>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end){
          if (begin >= end) return rule_base::pointer_type(nullptr);
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
      class parse_helper<_DeclT, character<_Ch>, false, _WhitespaceT>{
      public:
        template <typename _IteratorT>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end){
          if (begin >= end) return rule_base::pointer_type(nullptr);
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

      //characters
      template <typename _DeclT, char _First, char _Last, typename _WhitespaceT>
      class parse_helper<_DeclT, characters<_First, _Last>, true, _WhitespaceT>{
      public:
        template <typename _IteratorT>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end) {
          if (begin >= end) return rule_base::pointer_type(nullptr);
          _IteratorT oCurr = begin;
          parse_helper<_WhitespaceT, void, true, void>::parse(oCurr, end);
          for (char c = _First; c <= _Last; ++c) {
            if (tolower(c) == tolower(*oCurr)) {
              rule_base::pointer_type oRet(new characters<_First, _Last>(*oCurr));
              ++oCurr;
              begin = oCurr;
              return oRet;
            }
          }
          return rule_base::pointer_type(nullptr);
        }
      };
      template <typename _DeclT, char _First, char _Last, typename _WhitespaceT>
      class parse_helper<_DeclT, characters<_First, _Last>, false, _WhitespaceT>{
      public:
        template <typename _IteratorT>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end) {
          if (begin >= end) return rule_base::pointer_type(nullptr);
          _IteratorT oCurr = begin;
          parse_helper<_WhitespaceT, void, true, void>::parse(oCurr, end);
          for (char c = _First; c <= _Last; ++c) {
            if (c == *oCurr) {
              rule_base::pointer_type oRet(new characters<_First, _Last>(*oCurr));
              ++oCurr;
              begin = oCurr;
              return oRet;
            }
          }
          return rule_base::pointer_type(nullptr);
        }
      };


      //not
      template <typename _DeclT, typename ... _ParamTs, bool _IgnoreCase, typename _WhitespaceT >
      class parse_helper < _DeclT, parse::not_<_ParamTs...>, _IgnoreCase, _WhitespaceT> {
      public:
        template <typename _IteratorT, typename ... _ChildRuleTs>
        static rule_base::pointer_type parse(_IteratorT& oBegin, _IteratorT& oEnd){
          _IteratorT begin = oBegin;
          auto oTmp = parse_helper<and_<_ParamTs...>, and_<_ParamTs...>, _IgnoreCase, _WhitespaceT>::parse(begin, oEnd);
          if (oTmp) return rule_base::pointer_type(nullptr);
          return rule_base::pointer_type(new _DeclT());
        }
      };

        //and
      template <typename _DeclT, bool _IgnoreCase, typename _WhitespaceT >
      class parse_helper < _DeclT, parse::and_<>, _IgnoreCase, _WhitespaceT>{
      public:
        template <typename _IteratorT, typename ... _ChildRuleTs>
        static rule_base::pointer_type parse(_IteratorT& , _IteratorT& , _ChildRuleTs&& ... oChildRules){
          return rule_base::pointer_type(new _DeclT(std::forward<_ChildRuleTs>(oChildRules)...));
        }
      };

      template <typename _DeclT, typename _HeadT, typename ... _TailT, bool _IgnoreCase, typename _WhitespaceT >
      class parse_helper < _DeclT, parse::and_<_HeadT, _TailT...>, _IgnoreCase, _WhitespaceT> {
      public:
        template <typename _IteratorT, typename ... _ChildRuleTs>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end, _ChildRuleTs&& ... oChildRules) {
          _IteratorT oBegin = begin;
          auto oItem = parse_helper<_HeadT, typename _HeadT::impl_type, _IgnoreCase, _WhitespaceT>::parse(oBegin, end);
          if (!oItem) {
            return oItem;
          }
          oItem = parse_helper<_DeclT, parse::and_<_TailT...>, _IgnoreCase, _WhitespaceT>::parse(oBegin, end, std::forward<_ChildRuleTs>(oChildRules)..., oItem);
          if (oItem) {
            begin = oBegin;
          }
          return oItem;
        }
      };

      ///zero_or_one_
      template <typename _DeclT, typename _HeadT, bool _IgnoreCase, typename _WhitespaceT >
      class parse_helper < _DeclT, parse::zero_or_one_<_HeadT>, _IgnoreCase, _WhitespaceT> {
      public:

        template <typename _IteratorT>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end) {
          _IteratorT oBegin = begin;
          auto oItem = parse_helper<_HeadT, typename _HeadT::impl_type, _IgnoreCase, _WhitespaceT>::parse(oBegin, end);
          begin = oBegin;
          return rule_base::pointer_type(new _DeclT(oItem));
        }

      };

      ///one_or_more_
      template <typename _DeclT, typename _HeadT, bool _IgnoreCase, typename _WhitespaceT >
      class parse_helper < _DeclT, parse::one_or_more_<_HeadT>, _IgnoreCase, _WhitespaceT> {
      public:

        template <typename _IteratorT>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end) {
          _IteratorT oBegin = begin;
          rule_base::vector_type oChildRules;
          forever{
            auto oItem = parse_helper<_HeadT, typename _HeadT::impl_type, _IgnoreCase, _WhitespaceT>::parse(oBegin, end);
            if (!oItem) break;
            oChildRules.push_back(oItem);
          }
          if (!oChildRules.size()){
            return rule_base::pointer_type(nullptr);
          }
          begin = oBegin;
          return rule_base::pointer_type(new _DeclT(std::move(oChildRules)));
        }

      };

      ///or
      template <typename _DeclT, bool _IgnoreCase, typename _WhitespaceT >
      class parse_helper < _DeclT, parse::or_<>, _IgnoreCase, _WhitespaceT>{
      public:

        template <typename _IteratorT, typename ... _ChildRuleTs>
        static rule_base::pointer_type parse(_IteratorT& , _IteratorT& , _ChildRuleTs ... ){
          return rule_base::pointer_type(nullptr);
        }

      };

      template <typename _DeclT, typename _HeadT, typename ... _TailT, bool _IgnoreCase, typename _WhitespaceT >
      class parse_helper < _DeclT, parse::or_<_HeadT, _TailT...>, _IgnoreCase, _WhitespaceT>{
      public:

        template <typename _IteratorT, typename ... _ChildRuleTs>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end, _ChildRuleTs&& ... oChildRules){
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
      class parse_helper < _DeclT, parse::zero_or_more_<_Ty>, _IgnoreCase, _WhitespaceT>{
      public:

        template <typename _IteratorT, typename ... _ChildRuleTs>
        static rule_base::pointer_type parse(_IteratorT& begin, _IteratorT& end){
          rule_base::vector_type oChildRules;
          _IteratorT oBegin = begin;
          forever{
            auto oItem = parse_helper<_Ty, typename _Ty::impl_type, _IgnoreCase, _WhitespaceT>::parse(oBegin, end);
            if (oItem){
              oChildRules.push_back(oItem);
            }else{
              begin = oBegin;
              return rule_base::pointer_type(new _DeclT(std::move(oChildRules)));
            }
          }
        }
      };
    }
#endif
  }
  /** Main parser class

  The xtd::parser is used to perform the parse and return a constructed AST if the parse succeeds
  @tparam _RuleT The start rule of the grammar
  @tparam _IgnoreCase Specifies whether case should be ignored during the parse
  @tparam _WhitespaceT A specialization of xtd::parse::whitespace that specifies the characters to ignore
  */
  template <typename _RuleT, bool _IgnoreCase = false, typename _WhitespaceT = xtd::parse::whitespace<>> class  parser {
  public:

    /** Parses text
    @param begin the beginning iterator of the text to parse
    @param end the end iterator of the text to parse
    @returns a fully constructed AST of type _RuleT if the parse succeeds or a nullptr if failed
    */
    template <typename _IteratorT> static parse::rule_base::pointer_type parse(_IteratorT begin, _IteratorT end) {
      _IteratorT oBegin = begin;
      _IteratorT oEnd = end;
      return parse::_::parse_helper<_RuleT, typename _RuleT::impl_type, _IgnoreCase, _WhitespaceT>::parse(oBegin, oEnd);
    }

  };
  ///@}

}


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

  template <typename, bool, typename> class parser;


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

    class rule_base;

    template <typename iterator_t>
    struct parse_error{
      using iterator_type = iterator_t;
      using ptr = std::shared_ptr<parse_error>;
      using vector = std::vector<ptr>;
      const std::type_info& failed_rule;
      const iterator_type position;
      parse_error(const std::type_info& oFailedRule, const iterator_type oPosition) : failed_rule(oFailedRule), position(oPosition){}
      parse_error(const parse_error& src) : failed_rule(src.failed_rule), position(src.position){}
      parse_error(parse_error&& src) : failed_rule(std::move(src.failed_rule)), position(std::move(src.position)){}
      parse_error& operator=(const parse_error&) = delete;
      parse_error& operator=(parse_error&& src) = delete;
    };

    template <typename iterator_t>
    struct context{
      using iterator_type = iterator_t;
      iterator_type begin;
      iterator_type end;
      std::shared_ptr<rule_base> start_rule;
      typename parse_error<iterator_t>::vector parse_errors;
      context(const context& src) : begin(src.begin), end(src.end), start_rule(src.start_rule), parse_errors(src.parse_errors){}
      context(context&& src) : begin(std::move(src.begin)), end(std::move(src.end)), start_rule(std::move(src.start_rule)), parse_errors(std::move(src.parse_errors)){}
      context& operator=(const context& src){
        if (this == &src) return *this;
        begin = src.begin;
        end = src.end;
        start_rule = src.start_rule;
        parse_errors = src.parse_errors;
        return *this;
      }
      context& operator=(context&& src){
        begin = std::move(src.begin);
        end = std::move(src.end);
        start_rule = std::move(src.start_rule);
        parse_errors = std::move(src.parse_errors);
        return *this;
      }
      context(iterator_t& oBegin, iterator_t& oEnd) : begin(oBegin), end(oEnd), start_rule(nullptr), parse_errors(){}
    };


    /** Base class of both rules and terminals
     Though rules and terminals are technically different they share the rule_base ancestor in XTL to have a cleaner object model and simpler parsing algorithms.
     Its an abstract interface of parsable text or containers of same satisfying various rules (one or more, zero or more, exactly one, etc).
     */
    class rule_base : std::vector<std::shared_ptr<rule_base>>, std::enable_shared_from_this<rule_base>{
    public:
      using pointer_type = std::shared_ptr<rule_base>;
      using weak_ptr_t = std::weak_ptr<rule_base>;
      using super_t = std::vector<pointer_type>;
      using vector_type = super_t;

      /** Constructor
      @param[in] oChildRules list of child rules that successfully parsed to satisfy the current parse rule. The child rules are stored in a local variable and accessible via the items() member.
      */
      template <typename ... _child_ts>
      rule_base(_child_ts&& ... oChildRules) : super_t{ std::forward<_child_ts>(oChildRules)... }, _parent(){}

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
      pointer_type parent() { return _parent.lock(); }
    private:
      template <typename, bool, typename> friend class xtd::parser;
      void set_parent(weak_ptr_t oParent){
        _parent = oParent;
        for (auto & oChild : static_cast<super_t&>(*this)){
          oChild->set_parent(shared_from_this());
        }
      }
      std::weak_ptr<rule_base> _parent;
    };


    /** Curiously recurring template pattern to simplify creation of rule_base implementations
    Rules and terminals implement the rule_base interface through this pattern. It provides boiler plate pointer_type, isa() and type() members.
    @tparam _decl_t The type declaration. Permits access to the implementation from the rule class template. Declaration pass their name in the form of: class SomeRule : rule<SomeRule>
    @tparam _impl_t The declaration used by the parsing algorithms. The library instantiates type of _decl_t when _impl_t successfully parses.
    */
    template <typename _decl_t, typename _impl_t = _decl_t>
    class rule : public rule_base{
    public:
      using decl_type = _decl_t;
      using impl_type = _impl_t;
      using rule_type = rule<_decl_t, _impl_t>;

      template <typename ... _child_rule_ts>
      explicit rule(_child_rule_ts&& ... oChildRules) : rule_base(std::forward<_child_rule_ts>(oChildRules)...){}

      virtual ~rule() = default;

      virtual bool isa(const std::type_info& oType) const override{
        return (typeid(rule) == oType) || (typeid(decl_type) == oType) || (typeid(impl_type) == oType) || (typeid(rule_base) == oType);
      }

      virtual const std::type_info& type() const override{
        return typeid(_decl_t);
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
      template <typename ... _child_rule_ts>
      explicit not_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...){}
    };

    template <typename _head_t, typename ... _tail_ts> class not_<_head_t, _tail_ts...> : public rule<not_<_head_t, _tail_ts...>>{
    public:
      using _super_t = rule<not_<_head_t, _tail_ts...>>;
      template <typename ... _child_rule_ts>
      explicit not_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...){}
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
      template <typename ... _child_rule_ts>
      explicit and_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...){}
    };

    template <typename _head_t, typename ... _tail_ts> class and_<_head_t, _tail_ts...> : public rule<and_<_head_t, _tail_ts...>>{
    public:
      using _super_t = rule<and_<_head_t, _tail_ts...>>;
      template <typename ... _child_rule_ts>
      explicit and_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...){}

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
      template <typename ... _child_rule_ts>
      explicit or_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...){}
    };

    template <typename _head_t, typename ... _tail_ts> class or_<_head_t, _tail_ts...> : public rule<or_<_head_t, _tail_ts...>>{
    public:
      using _super_t = rule<or_<_head_t, _tail_ts...>>;
      template <typename ... _child_rule_ts>
      explicit or_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...){}
    };
#endif


    /**
    Represents a parse algorithm where where the terminal or rule is repeated one or more times in the input stream
    The specified item must parse successfully at least once to satisfy the parse rule. Consecutive occurrences are attempted and permitted if present.
    @tparam _ty The rule or terminal to parse
    */
    template <typename _ty> class one_or_more_ : public rule<one_or_more_<_ty>>{
    public:
      using _super_t = rule<one_or_more_<_ty>>;
      template <typename ... _child_rule_ts>
      explicit one_or_more_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...){}
    };



    /**
    Represents a parse algorithm where the terminal or rule repeats zero or more times in the input stream
    Always successfully parses whether or not the element parses. Data in the input stream is consumed if present.
    @tparam _ty the rule or terminal to parse
    */
    template <typename _ty> class zero_or_more_ : public rule<zero_or_more_<_ty>>{
    public:
      using _super_t = rule<zero_or_more_<_ty>>;
      template <typename ... _child_rule_ts>
      explicit zero_or_more_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...){}
    };


    /** Represents a parse algorithm where the terminal or rule repeats exactly zero or one time in the input stream
    Always successfully parses whether or not the element parses.
    @tparam _ty the rule or terminal to parse
    */
    template <typename _ty> class zero_or_one_ : public rule<zero_or_one_<_ty>>{
    public:
      using _super_t = rule<zero_or_one_<_ty>>;
      template <typename ... _child_rule_ts>
      explicit zero_or_one_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...){}
    };


    /** list of whitespace characters to ignore in the input stream.
    Whitespace characters are parsed and discarded from the input stream when encountered between rules or terminals.
    A typical whitespace declaration for written language might be: whitespace<'\\r', '\\n', '\\t', ' '>;
    @tparam _chs... list of characters to ignore.
    */
    template <char..._chs> class whitespace{
    public:
      using whitespace_type = whitespace<_chs...>;
    };


    /** String terminal parsing algorithm.
    This template is infrequently used directly. The STRING and STRING_ macros are provided to declare string terminals.
    */
    template <typename _ty, _ty &> class string;
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
    template <typename _ty, _ty &> class regex;
#if (!DOXY_INVOKED)
    template <size_t _len, char(&_str)[_len]> class regex<char[_len], _str> : public rule<regex<char[_len], _str>>{
    public:
      using _super_t = rule<regex<char[_len], _str>>;
      static constexpr size_t length = _len;
      regex(const std::string& newval) : _super_t(), _value(newval){}
      const std::string& value() const{ return _value; }
    protected:
      std::string _value;
    };
#endif

    struct EndOfFile {};


#if (!DOXY_INVOKED)
    /**@internal
    Internal parsing algorithms are specializations of parse_helper that partially specialize the built in rules and terminals
     */

    namespace _{

      template <typename _decl_t, typename _impl_t, bool _ignore_case, typename _whitespace_t> class parse_helper;

      ///case sensitive string
      template <typename _decl_t, size_t _len, char(&_str)[_len], typename _whitespace_t>
      class parse_helper<_decl_t, parse::string<char[_len], _str>, false, _whitespace_t>{
      public:
        template<typename _iterator_t> static bool parse(context<_iterator_t> &oOuter) {
          context <_iterator_t> oContext(oOuter);
          parse_helper<_whitespace_t, void, true, void>::parse(oContext);

          for (size_t i = 0; (i < (_len-1)) && (oContext.begin < oContext.end); ++i, ++oContext.begin){
            if (_str[i] != *oContext.begin){
              oOuter.parse_errors.emplace_back(new parse::parse_error<_iterator_t>(typeid(_decl_t), oContext.begin));
              return false;
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
          if (oContext.begin < oContext.end && isalnum(*oContext.begin) && isalnum(_str[_len - 2])){
            oOuter.parse_errors.emplace_back(new parse::parse_error<_iterator_t>(typeid(_decl_t), oContext.begin));
            return false;
          }
          parse_helper<_whitespace_t, void, true, void>::parse(oContext);
          oContext.start_rule = rule_base::pointer_type(new _decl_t);
          oOuter = oContext;
          return true;
        }
      };


      ///ignore case string
      template <typename _decl_t, size_t _len, char(&_str)[_len], typename _whitespace_t>
      class parse_helper<_decl_t, parse::string<char[_len], _str>, true, _whitespace_t>{
      public:
        template<typename _iterator_t> static bool parse(context<_iterator_t> &oOuter) {
          context <_iterator_t> oContext(oOuter);
          parse_helper<_whitespace_t, void, true, void>::parse(oContext);
          for (size_t i = 0; (i < _len-1) && (oContext.begin < oContext.end); ++i, ++oContext.begin){
            if (tolower(_str[i]) != tolower(*oContext.begin)){
              oOuter.parse_errors.emplace_back(new parse::parse_error<_iterator_t>(typeid(_decl_t), oContext.begin));
              return false;
            }
          }
          ///ensure there's an identifiable separation between terminals. this should be done differently
          if (oContext.begin < oContext.end && isalnum(*oContext.begin) && isalnum(_str[_len - 2])){
            oOuter.parse_errors.emplace_back(new parse::parse_error<_iterator_t>(typeid(_decl_t), oContext.begin));
            return false;
          }
          parse_helper<_whitespace_t, void, true, void>::parse(oContext);
          oContext.start_rule = rule_base::pointer_type(new _decl_t);
          oOuter = oContext;
          return true;
        }
      };

      ///regex
      template <typename _decl_t, size_t _len, char(&_str)[_len], bool _ignore_case, typename _whitespace_t>
      class parse_helper<_decl_t, parse::regex<char[_len], _str>, _ignore_case, _whitespace_t> {
      public:
        template<typename _iterator_t> static bool parse(context<_iterator_t> &oOuter) {
          context <_iterator_t> oContext(oOuter);
          parse_helper<_whitespace_t, void, true, void>::parse(oContext);
          static std::regex_constants::syntax_option_type iFlags = std::regex_constants::optimize |
                                                                   (_ignore_case ? std::regex_constants::icase
                                                                                 : std::regex_constants::optimize);

          static std::regex oRE(_str, iFlags);
          std::match_results<std::string::iterator> oMatch;
          if (!std::regex_search(oContext.begin, oContext.end, oMatch, oRE, std::regex_constants::match_continuous)) {
            oOuter.parse_errors.emplace_back(new parse::parse_error<_iterator_t>(typeid(_decl_t), oContext.begin));
            return false;
          }
          oContext.begin += oMatch[0].length();

          ///ensure there's an identifiable separation between terminals. this should be done differently
          if (oContext.begin < oContext.end && isalnum(*oContext.begin) && isalnum(_str[_len - 1])) {
            oOuter.parse_errors.emplace_back(new parse::parse_error<_iterator_t>(typeid(_decl_t), oContext.begin));
            return false;
          }
          parse_helper<_whitespace_t, void, true, void>::parse(oContext);
          oContext.start_rule = rule_base::pointer_type(new _decl_t(oMatch[0].str()));
          oOuter = oContext;
          return true;
        }
      };

      ///whitespace
      template <bool _ignore_case>
      class parse_helper<whitespace<>, void, _ignore_case, void>{
      public:
        static bool parse(...) { return false; }
      };

      template <char _head_ch, char... _tail_chs, bool _ignore_case>
      class parse_helper<whitespace<_head_ch, _tail_chs...>, void, _ignore_case, void>{
      public:
        template <typename _iterator_t>
        static bool parse(context<_iterator_t>& oContext) {
          while (oContext.begin < oContext.end){
            if (*oContext.begin == _head_ch) {
              oContext.begin++;
              continue;
            }
            if (parse_helper<whitespace<_tail_chs...>, void, _ignore_case, void>::parse(oContext)) continue;
            break;
          }
          return true;
        }
      };

      //characters
      template <typename _decl_t, char _first, char _last, typename _whitespace_t>
      class parse_helper<_decl_t, characters<_first, _last>, true, _whitespace_t>{
      public:
        template <typename _iterator_t>
        static bool parse(context<_iterator_t>& oOuter) {
          context<_iterator_t> oContext(oOuter);
          parse_helper< _whitespace_t, void, true, void>::parse(oContext);
          if (oContext.begin < oContext.end && tolower(*oContext.begin) >= tolower(_first) && tolower(*oContext.begin) <= tolower(_last)){
            oContext.parse_errors.clear();
            oContext.start_rule = rule_base::pointer_type(new _decl_t);
            oContext.begin++;
            oOuter = oContext;
            return true;
          }
          oOuter.parse_errors.emplace_back(new parse::parse_error<_iterator_t>(typeid(characters<_first, _last>), oContext.begin));
          return false;
        }
      };
      template <typename _decl_t, char _first, char _last, typename _whitespace_t>
      class parse_helper<_decl_t, characters<_first, _last>, false, _whitespace_t>{
      public:
        template <typename _iterator_t>
        static bool parse(context<_iterator_t>& oOuter) {
          context<_iterator_t> oContext(oOuter);
          parse_helper< _whitespace_t, void, true, void>::parse(oContext);
          if (oContext.begin < oContext.end && *oContext.begin >= _first && *oContext.begin <= _last){
            oContext.parse_errors.clear();
            oContext.start_rule = rule_base::pointer_type(new _decl_t(*oContext.begin));
            oContext.begin++;
            oOuter = oContext;
            return true;
          }
          oOuter.parse_errors.emplace_back(new parse::parse_error<_iterator_t>(typeid(characters<_first, _last>), oContext.begin));
          return false;
        }
      };

      //character
      template <typename _decl_t, char _ch, bool _ignore_case, typename _whitespace_t>
      class parse_helper<_decl_t, character<_ch>, _ignore_case, _whitespace_t>
        : public parse_helper<_decl_t, characters<_ch, _ch>, _ignore_case, _whitespace_t>
      {};



      //not
      template <typename _decl_t, typename ... _ParamTs, bool _ignore_case, typename _whitespace_t >
      class parse_helper < _decl_t, parse::not_<_ParamTs...>, _ignore_case, _whitespace_t> {
      public:
        template <typename _iterator_t, typename ... _child_rule_ts>
        static bool parse(context<_iterator_t>& oOuter) {
          context<_iterator_t> oContext(oOuter);
          if (!parse_helper<and_<_ParamTs...>, and_<_ParamTs...>, _ignore_case, _whitespace_t>::parse(oContext)) return true;
          oOuter.parse_errors.emplace_back(new parse::parse_error<_iterator_t>(typeid(_decl_t), oOuter.begin));
          return false;
        }
      };

      ///and
      template <typename _decl_t, bool _ignore_case, typename _whitespace_t >
      class parse_helper < _decl_t, parse::and_<>, _ignore_case, _whitespace_t>{
      public:
        template <typename _iterator_t, typename ... _child_rule_ts>
        static bool parse(context<_iterator_t>& oOuter, _child_rule_ts&& ... oChildRules) {
          oOuter.start_rule = rule_base::pointer_type(new _decl_t(std::forward<_child_rule_ts>(oChildRules)...));
          oOuter.parse_errors.clear();
          return true;
        }
      };

      template <typename _decl_t, typename _head_t, typename ... _tail_ts, bool _ignore_case, typename _whitespace_t >
      class parse_helper < _decl_t, parse::and_<_head_t, _tail_ts...>, _ignore_case, _whitespace_t> {
      public:
        template <typename _iterator_t, typename ... _child_rule_ts>
        static bool parse(context<_iterator_t>& oOuter, _child_rule_ts&& ... oChildRules) {
          context<_iterator_t> oContext(oOuter);
          auto bRet = parse_helper<_head_t, typename _head_t::impl_type, _ignore_case, _whitespace_t>::parse(oContext);
          if (!bRet) {
            oOuter.parse_errors.emplace_back(new parse::parse_error<_iterator_t>(typeid(and_<_decl_t>), oOuter.begin));
            return false;
          }
          bRet = parse_helper<_decl_t, parse::and_<_tail_ts...>, _ignore_case, _whitespace_t>::parse(oContext, std::forward<_child_rule_ts>(oChildRules)..., oContext.start_rule);
          if (!bRet) {
            oOuter.parse_errors.emplace_back(new parse::parse_error<_iterator_t>(typeid(and_<_tail_ts...>), oOuter.begin));
            return false;
          }
          oOuter = oContext;
          return true;
        }
      };


      ///or
      template <typename _decl_t, bool _ignore_case, typename _whitespace_t >
      class parse_helper < _decl_t, parse::or_<>, _ignore_case, _whitespace_t>{
      public:
        static bool parse(...){ return false; }
      };

      template <typename _decl_t, typename _head_t, typename ... _tail_ts, bool _ignore_case, typename _whitespace_t >
      class parse_helper < _decl_t, parse::or_<_head_t, _tail_ts...>, _ignore_case, _whitespace_t>{
      public:
        template <typename _iterator_t, typename ... _child_rule_ts>
        static bool parse(context<_iterator_t>& oOuter){
          context<_iterator_t> oContext(oOuter);
          auto bRet = parse_helper<_head_t, typename _head_t::impl_type, _ignore_case, _whitespace_t>::parse(oContext);
          if (bRet){
            oOuter = oContext;
            oOuter.parse_errors.clear();
            oOuter.start_rule = rule_base::pointer_type(new _decl_t(oOuter.start_rule));
            return true;
          }
          oContext.parse_errors.emplace_back(new parse::parse_error<_iterator_t>(typeid(_head_t), oOuter.begin));
          return parse_helper<_decl_t, parse::or_<_tail_ts...>, _ignore_case, _whitespace_t>::parse(oContext);
        }
      };


      ///zero_or_one_
      template <typename _decl_t, typename _head_t, bool _ignore_case, typename _whitespace_t >
      class parse_helper < _decl_t, parse::zero_or_one_<_head_t>, _ignore_case, _whitespace_t> {
      public:
        template <typename _iterator_t>
        static bool parse(context<_iterator_t>& oOuter){
          parse_helper<_head_t, typename _head_t::impl_type, _ignore_case, _whitespace_t>::parse(oOuter);
          oOuter.start_rule = rule_base::pointer_type(new _decl_t(oOuter.start_rule));
          oOuter.parse_errors.clear();
          return true;
        }
      };


      ///one_or_more_
      template <typename _decl_t, typename _head_t, bool _ignore_case, typename _whitespace_t >
      class parse_helper < _decl_t, parse::one_or_more_<_head_t>, _ignore_case, _whitespace_t> {
      public:
        template <typename _iterator_t, typename ... _child_rule_ts>
        static bool parse(context<_iterator_t>& oOuter,_child_rule_ts&&...oChildren ){
          context<_iterator_t> oContext(oOuter);
          auto bRet = parse_helper<_head_t, typename _head_t::impl_type, _ignore_case, _whitespace_t>::parse(oContext);
          if (!bRet) {
            oOuter.parse_errors.emplace_back(new parse::parse_error<_iterator_t>(typeid(_decl_t), oOuter.begin));
            oOuter.start_rule = rule_base::pointer_type(new _decl_t(std::forward<_child_rule_ts>(oChildren)...));
            return false;
          }
          parse(oContext, std::forward<_child_rule_ts>(oChildren)..., oContext.start_rule);
          oContext.parse_errors.clear();
          oOuter = oContext;
          return true;
        }
      };

      ///zero or more
      template <typename _decl_t, typename _ty, bool _ignore_case, typename _whitespace_t >
      class parse_helper < _decl_t, parse::zero_or_more_<_ty>, _ignore_case, _whitespace_t>{
      public:

        template <typename _iterator_t, typename ... _child_rule_ts>
        static bool parse(context<_iterator_t>& oOuter, _child_rule_ts&&...oChildren){
        context<_iterator_t> oContext(oOuter);
          auto bRet = parse_helper<_ty, typename _ty::impl_type, _ignore_case, _whitespace_t>::parse(oContext);
          if (bRet){
            return parse_helper<_decl_t, zero_or_more_<_ty>, _ignore_case, _whitespace_t>::parse(oContext, std::forward<_child_rule_ts>(oChildren)..., oContext.start_rule);
          }else{
            oOuter = oContext;
            oOuter.parse_errors.clear();
            oOuter.start_rule = rule_base::pointer_type(new _decl_t(std::forward<_child_rule_ts>(oChildren)..., oContext.start_rule));
            return true;
          }
        }
      };
    }
#endif
  }
  /** Main parser class

  The xtd::parser is used to perform the parse and return a constructed AST if the parse succeeds
  @tparam _rule_t The start rule of the grammar
  @tparam _ignore_case Specifies whether case should be ignored during the parse
  @tparam _whitespace_t A specialization of xtd::parse::whitespace that specifies the characters to ignore
  */
  template <typename _rule_t, bool _ignore_case = false, typename _whitespace_t = xtd::parse::whitespace<>> class  parser {
  public:

    /** Parses text
    @param begin the beginning iterator of the text to parse
    @param end the end iterator of the text to parse
    @returns a fully constructed AST of type _RuleT if the parse succeeds or a nullptr if failed
    */
    template <typename _iterator_t> static bool parse(_iterator_t begin, _iterator_t end, typename _rule_t::pointer_type& ast, typename parse::parse_error<_iterator_t>::vector& errors) {
      typename parse::context<_iterator_t> oContext{begin, end};

      auto bRet = parse::_::parse_helper<_rule_t, typename _rule_t::impl_type, _ignore_case, _whitespace_t>::parse(oContext);
      errors = oContext.parse_errors;
      if (oContext.begin  < oContext.end) return false;
      ast = oContext.start_rule;
      ast->set_parent(parse::rule_base::pointer_type());
      return bRet;
    }
    template <typename _iterator_t> static bool parse(_iterator_t begin, _iterator_t end, typename _rule_t::pointer_type& ast) {
      typename parse::parse_error<_iterator_t>::vector oErrors;
      return parse(begin, end, ast, oErrors);
    }

  };
  ///@}

}


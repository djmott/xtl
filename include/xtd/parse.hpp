/** @file
text parsing and AST generation
@copyright David Mott (c) 2016.
Distributed under the Boost Software License Version 1.0.
See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
@example example_parse1.cpp
@example example_parse2.cpp
 */

#pragma once

#include <xtd/xtd.hpp>
#include <xtd/meta.hpp>
#include <memory>
#include <string>
#include <vector>
#include <regex>
#include <typeinfo>
#include <string_view>
#include <cctype>

namespace xtd {

  template <typename, bool, typename> class parser;

  namespace parse {

    // Forward declarations
    class rule_base;

    // Skip whitespace helper function
    template<typename Iterator>
    void skip_ws(Iterator& begin, Iterator& end, bool ignore_whitespace) {
      if (!ignore_whitespace) return;
      while (begin != end && (*begin == ' ' || *begin == '\t' || *begin == '\n' || *begin == '\r')) {
        ++begin;
      }
    }

    // Parse error structure
    template <typename iterator_t>
    struct parse_error {
      using iterator_type = iterator_t;
      using ptr = std::shared_ptr<parse_error>;
      using vector = std::vector<ptr>;
      const std::type_info& failed_rule;
      const iterator_type position;
      std::string expected;
      std::string found;

      parse_error(const std::type_info& failRule, const iterator_type pos,
        std::string expected_msg = "", std::string found_msg = "")
        : failed_rule(failRule), position(pos),
        expected(std::move(expected_msg)), found(std::move(found_msg)) {
      }
    };

    // Context structure
    template <typename iterator_t>
    struct context {
      using iterator_type = iterator_t;
      iterator_type begin;
      iterator_type end;
      bool ignore_whitespace;
      std::shared_ptr<rule_base> start_rule;
      typename parse_error<iterator_t>::vector parse_errors;

      context(iterator_type b, iterator_type e, bool ignore_ws = true)
        : begin(b), end(e), ignore_whitespace(ignore_ws) {
      }
    };

    // Rule base class
    class rule_base
      : public std::vector<std::shared_ptr<rule_base>>,
      public std::enable_shared_from_this<rule_base> {
    public:
      using iterator_type = std::string::const_iterator;
      using pointer_type = std::shared_ptr<rule_base>;
      using weak_ptr_t = std::weak_ptr<rule_base>;
      using super_t = std::vector<pointer_type>;
      using vector_type = super_t;

      template <typename ... _child_ts>
      rule_base(_child_ts&& ... oChildRules)
        : super_t{ std::forward<_child_ts>(oChildRules)... },
        _parent() {
      }

      virtual ~rule_base() = default;

      virtual bool isa(const std::type_info& oType) const = 0;
      virtual const std::type_info& type() const = 0;

      pointer_type parent() { return _parent.lock(); }

      virtual bool parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) = 0;

      virtual std::string_view get_text() const { return ""; }

    private:
      void set_parent(weak_ptr_t oParent) {
        _parent = oParent;
        auto oThis = shared_from_this();
        for (auto& oChild : static_cast<super_t&>(*this)) {
          oChild->set_parent(oThis);
        }
      }
      std::weak_ptr<rule_base> _parent;
    };

    // Helper to clone rules for AST building
    template <typename RuleT>
    std::shared_ptr<RuleT> clone_rule(const std::shared_ptr<RuleT>& src) {
      auto clone = std::make_shared<RuleT>(*src);
      clone->set_parent(src->parent());
      return clone;
    }

    // Rule template using CRTP
    template <typename _decl_t, typename _impl_t = _decl_t>
    class rule : public rule_base {
    public:
      using decl_type = _decl_t;
      using impl_type = _impl_t;
      using rule_type = rule<_decl_t, _impl_t>;

      template <typename ... _child_rule_ts>
      explicit rule(_child_rule_ts&& ... oChildRules)
        : rule_base(std::forward<_child_rule_ts>(oChildRules)...) {
      }

      virtual ~rule() = default;

      bool isa(const std::type_info& oType) const override {
        return (typeid(rule) == oType) || (typeid(decl_type) == oType) ||
          (typeid(impl_type) == oType) || (typeid(rule_base) == oType);
      }

      const std::type_info& type() const override {
        return typeid(_decl_t);
      }

      bool parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        if (ctx.ignore_whitespace) skip_ws(begin, end, ctx.ignore_whitespace);
        for (auto& child : *this) {
          if (!child->parse(ctx, begin, end)) {
            ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
              this->type(), backup, "Sequence element failed",
              begin != end ? std::string(1, *begin) : "EOF"));
            begin = backup;
            return false;
          }
        }
        return true;
      }
    };

    // NOT combinator
    template <typename ...> class not_;

    template <> class not_<> : public rule<not_<>> {
    public:
      using _super_t = rule<not_<>>;
      template <typename ... _child_rule_ts>
      explicit not_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...) {}

      bool parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        if (ctx.ignore_whitespace) skip_ws(begin, end, ctx.ignore_whitespace);
        if (empty() || this->front()->parse(ctx, begin, end)) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, "Negative assertion failed",
            begin != end ? std::string(1, *begin) : "EOF"));
          begin = backup;
          return false;
        }
        begin = backup;
        return true;
      }
    };

    template <typename _head_t, typename ... _tail_ts>
    class not_<_head_t, _tail_ts...> : public rule<not_<_head_t, _tail_ts...>> {
    public:
      using _super_t = rule<not_<_head_t, _tail_ts...>>;
      template <typename ... _child_rule_ts>
      explicit not_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...) {}

      bool parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        if (ctx.ignore_whitespace) skip_ws(begin, end, ctx.ignore_whitespace);

        auto temp_begin = begin;
        bool all_matched = true;
        for (auto& child : *this) {
          if (!child->parse(ctx, temp_begin, end)) {
            all_matched = false;
            break;
          }
        }

        if (all_matched) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, "Negative assertion failed",
            begin != end ? std::string(1, *begin) : "EOF"));
          begin = backup;
          return false;
        }

        begin = backup;
        return true;
      }
    };

    // AND combinator (uses default sequence implementation)
    template <typename ...> class and_;
    template <> class and_<> : public rule<and_<>> {};
    template <typename _head_t, typename ... _tail_ts>
    class and_<_head_t, _tail_ts...> : public rule<and_<_head_t, _tail_ts...>> {};

    // OR combinator
    template <typename ...> class or_;

    template <> class or_<> : public rule<or_<>> {
    public:
      using _super_t = rule<or_<>>;
      template <typename ... _child_rule_ts>
      explicit or_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...) {}

      bool parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        if (ctx.ignore_whitespace) skip_ws(begin, end, ctx.ignore_whitespace);
        typename parse_error<iterator_type>::vector branch_errors;
        for (auto& child : *this) {
          auto child_backup = begin;
          if (child->parse(ctx, begin, end)) return true;
          branch_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            child->type(), child_backup, "Alternative branch failed",
            child_backup != end ? std::string(1, *child_backup) : "EOF"));
          begin = backup;
        }
        ctx.parse_errors.insert(ctx.parse_errors.end(), branch_errors.begin(), branch_errors.end());
        begin = backup;
        return false;
      }
    };

    template <typename _head_t, typename ... _tail_ts>
    class or_<_head_t, _tail_ts...> : public rule<or_<_head_t, _tail_ts...>> {};

    // ONE_OR_MORE combinator
    template <typename _ty>
    class one_or_more_ : public rule<one_or_more_<_ty>> {
    public:
      using _super_t = rule<one_or_more_<_ty>>;
      template <typename ... _child_rule_ts>
      explicit one_or_more_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...) {}

      bool parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        if (ctx.ignore_whitespace) skip_ws(begin, end, ctx.ignore_whitespace);
        if (empty()) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, "One_or_more rule has no child", ""));
          begin = backup;
          return false;
        }
        auto child = this->front();

        if (!child->parse(ctx, begin, end)) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, "One_or_more requires at least one match",
            begin != end ? std::string(1, *begin) : "EOF"));
          begin = backup;
          return false;
        }
        this->push_back(clone_rule(child));

        while (true) {
          auto loop_backup = begin;
          if (ctx.ignore_whitespace) skip_ws(begin, end, ctx.ignore_whitespace);
          if (begin == end || !child->parse(ctx, begin, end)) {
            begin = loop_backup;
            break;
          }
          if (begin == loop_backup) break;
          this->push_back(clone_rule(child));
        }
        return true;
      }
    };

    // ZERO_OR_MORE combinator
    template <typename _ty>
    class zero_or_more_ : public rule<zero_or_more_<_ty>> {
    public:
      using _super_t = rule<zero_or_more_<_ty>>;
      template <typename ... _child_rule_ts>
      explicit zero_or_more_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...) {}

      bool parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        if (empty()) return true;
        auto child = this->front();
        while (true) {
          auto backup = begin;
          if (ctx.ignore_whitespace) skip_ws(begin, end, ctx.ignore_whitespace);
          if (begin == end || !child->parse(ctx, begin, end)) {
            begin = backup;
            break;
          }
          if (begin == backup) break;
          this->push_back(clone_rule(child));
        }
        return true;
      }
    };

    // ZERO_OR_ONE combinator
    template <typename _ty>
    class zero_or_one_ : public rule<zero_or_one_<_ty>> {
    public:
      using _super_t = rule<zero_or_one_<_ty>>;
      template <typename ... _child_rule_ts>
      explicit zero_or_one_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...) {}

      bool parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        if (empty()) return true;
        auto backup = begin;
        if (ctx.ignore_whitespace) skip_ws(begin, end, ctx.ignore_whitespace);
        auto child = this->front();
        if (child->parse(ctx, begin, end)) {
          this->push_back(clone_rule(child));
          return true;
        }
        begin = backup;
        return true;
      }
    };

    // STRING terminal
    template <typename _ty, _ty&> class string;
    template <size_t _len, char(&_str)[_len]>
    class string<char[_len], _str> : public rule<string<char[_len], _str>> {
    private:
      std::string _matched;

    public:
      using _super_t = rule<string<char[_len], _str>>;
      static constexpr size_t length = _len - 1;
      string() = default;

      bool parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        if (ctx.ignore_whitespace) skip_ws(begin, end, ctx.ignore_whitespace);
        constexpr const char* lit = _str;
        for (size_t i = 0; i < length; ++i) {
          if (begin == end || *begin != lit[i]) {
            ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
              this->type(), backup, std::string("Expected string: ") + lit,
              begin != end ? std::string(1, *begin) : "EOF"));
            begin = backup;
            return false;
          }
          ++begin;
        }
        _matched = std::string(lit, length);
        return true;
      }

      std::string_view get_text() const override { return _matched; }
    };

    // CHARACTER terminal
    template <char _value>
    class character : public rule<character<_value>> {
    private:
      char _matched = 0;

    public:
      bool parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        if (ctx.ignore_whitespace) skip_ws(begin, end, ctx.ignore_whitespace);
        if (begin == end || *begin != _value) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, std::string("Expected character: '") + _value + "'",
            begin != end ? std::string(1, *begin) : "EOF"));
          begin = backup;
          return false;
        }
        _matched = *begin;
        ++begin;
        return true;
      }

      std::string_view get_text() const override { return std::string_view(&_matched, 1); }
    };

    // CHARACTERS range terminal
    template <char _first, char _last>
    class characters : public rule<characters<_first, _last>> {
    private:
      char _matched = 0;

    public:
      using _super_t = rule<characters<_first, _last>>;

      bool parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        if (ctx.ignore_whitespace) skip_ws(begin, end, ctx.ignore_whitespace);
        if (begin == end) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, "Expected character in range",
            "EOF"));
          begin = backup;
          return false;
        }

        char c = *begin;
        if constexpr (_first <= _last) {
          if (c < _first || c > _last) {
            ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
              this->type(), backup,
              std::string("Expected character in range [") + _first + "-" + _last + "]",
              std::string(1, c)));
            begin = backup;
            return false;
          }
        }
        else {
          if (c > _first || c < _last) {
            ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
              this->type(), backup,
              std::string("Expected character in range [") + _last + "-" + _first + "]",
              std::string(1, c)));
            begin = backup;
            return false;
          }
        }

        _matched = c;
        ++begin;
        return true;
      }

      std::string_view get_text() const override { return std::string_view(&_matched, 1); }
    };

    // REGEX terminal
    template <typename _ty, _ty&> class regex;
    template <size_t _len, char(&_str)[_len]>
    class regex<char[_len], _str> : public rule<regex<char[_len], _str>> {
    private:
      std::string _matched;
      static std::regex& get_pattern() {
        static std::regex pattern(_str);
        return pattern;
      }

    public:
      using _super_t = rule<regex<char[_len], _str>>;
      static constexpr size_t length = _len - 1;

      bool parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        if (ctx.ignore_whitespace) skip_ws(begin, end, ctx.ignore_whitespace);

        if (begin == end) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, "Expected regex match", "EOF"));
          begin = backup;
          return false;
        }

        std::match_results<iterator_type> matches;
        auto& pattern = get_pattern();

        if (!std::regex_search(begin, end, matches, pattern,
          std::regex_constants::match_continuous)) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, std::string("Expected regex: ") + _str,
            begin != end ? std::string(begin, begin + std::min<size_t>(10, end - begin)) + "..." : "EOF"));
          begin = backup;
          return false;
        }

        if (matches.empty() || matches.position(0) != 0) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, "Regex match not at beginning",
            std::string(begin, begin + std::min<size_t>(10, end - begin)) + "..."));
          begin = backup;
          return false;
        }

        _matched = matches.str(0);
        begin += _matched.length();
        return true;
      }

      std::string_view get_text() const override { return _matched; }
    };

    // WHITESPACE definition
    template <char..._chs>
    class whitespace {
    public:
      using whitespace_type = whitespace<_chs...>;
    };

    // Macros for convenience
#define STRING_(x) \
        namespace _ { char x[] = #x; } \
        using x = xtd::parse::string<decltype(_::x), _::x>;

#define STRING(_name, _value) \
        namespace _ { char _name[] = _value; } \
        using _name = xtd::parse::string<decltype(_::_name), _::_name>;

#define CHARACTER_(_name, _value) \
        using _name = xtd::parse::character<_value>

#define CHARACTERS_(_name, _first, _last) \
        using _name = xtd::parse::characters<_first, _last>

#define REGEX(_name, _value) \
        namespace _ { char _name[] = _value; } \
        using _name = xtd::parse::regex<decltype(_::_name), _::_name>;

    // End of file marker
    struct EndOfFile {
      using impl_type = EndOfFile;
    };

  } // namespace parse

  // Main parser class
  template <typename _rule_t, bool _ignore_case = false, typename _whitespace_t = parse::whitespace<' ', '\t', '\n', '\r'>>
  class parser {
  public:
    template <typename _iterator_t>
    static bool parse(_iterator_t begin, _iterator_t end,
      std::shared_ptr<_rule_t>& ast,
      typename parse::parse_error<_iterator_t>::vector& errors) {
      parse::context<_iterator_t> ctx(begin, end, true);
      auto start_rule = std::make_shared<_rule_t>();

      if (!start_rule->parse(ctx, begin, end)) {
        errors = ctx.parse_errors;
        return false;
      }

      if (begin != end) {
        errors.emplace_back(std::make_shared<parse::parse_error<_iterator_t>>(
          typeid(_rule_t), begin, "Expected end of input",
          std::string(begin, begin + std::min<size_t>(10, end - begin)) + "..."));
        return false;
      }

      ast = start_rule;
      return true;
    }

    template <typename _iterator_t>
    static bool parse(_iterator_t begin, _iterator_t end, std::shared_ptr<_rule_t>& ast) {
      typename parse::parse_error<_iterator_t>::vector errors;
      return parse(begin, end, ast, errors);
    }
  };

} // namespace xtd

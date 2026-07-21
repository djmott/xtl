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
#include <iterator>
#include <cctype>

namespace xtd {

  template <typename, bool, typename> class parser;

  namespace parse {

    // Forward declarations
    class rule_base;

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

    // Case-insensitive character helpers
    inline char to_lower_ch(char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c; }
    inline char swap_case_ch(char c) {
      if (c >= 'A' && c <= 'Z') return static_cast<char>(c - 'A' + 'a');
      if (c >= 'a' && c <= 'z') return static_cast<char>(c - 'a' + 'A');
      return c;
    }
    inline bool char_match(char a, char b, bool ignore_case) {
      return ignore_case ? (to_lower_ch(a) == to_lower_ch(b)) : (a == b);
    }

    // Context structure
    template <typename iterator_t>
    struct context {
      using iterator_type = iterator_t;
      iterator_type begin;
      iterator_type end;
      bool ignore_whitespace;
      bool ignore_case;
      std::string whitespace_chars;
      std::shared_ptr<rule_base> start_rule;
      typename parse_error<iterator_t>::vector parse_errors;

      context(iterator_type b, iterator_type e, bool ignore_ws = true, std::string ws = " \t\n\r", bool ignore_case_ = false)
        : begin(b), end(e), ignore_whitespace(ignore_ws), ignore_case(ignore_case_), whitespace_chars(std::move(ws)) {
      }

      bool is_whitespace(char c) const {
        return ignore_whitespace && whitespace_chars.find(c) != std::string::npos;
      }
    };

    // Skip whitespace helper: honors the context's configured whitespace set.
    template<typename Iterator, typename iterator_t>
    void skip_ws(Iterator& begin, Iterator& end, const context<iterator_t>& ctx) {
      while (begin != end && ctx.is_whitespace(*begin)) {
        ++begin;
      }
    }

    /** Rule base class.
     *
     * A rule node IS-A vector of child rule nodes. When a grammar rule matches,
     * its `parse()` instantiates a node of that rule's own specialization, adds
     * the matched child node(s) to it, and returns the node. A failed match
     * returns nullptr and consumes no input. On overall success the returned
     * root is a fully-formed, iterable AST: iterate a node to visit its
     * children; leaves expose the matched text via `get_text()`.
     */
    class rule_base
      : public std::vector<std::shared_ptr<rule_base>>,
      public std::enable_shared_from_this<rule_base> {
    public:
      using iterator_type = std::string::const_iterator;
      using pointer_type = std::shared_ptr<rule_base>;
      using weak_ptr_t = std::weak_ptr<rule_base>;
      using super_t = std::vector<pointer_type>;
      using vector_type = super_t;
      using ptr = pointer_type;

      template <typename ... _child_ts>
      rule_base(_child_ts&& ... oChildRules)
        : super_t{ std::forward<_child_ts>(oChildRules)... },
        _parent() {
      }

      virtual ~rule_base() = default;

      virtual bool isa(const std::type_info& oType) const = 0;
      virtual const std::type_info& type() const = 0;

      pointer_type parent() { return _parent.lock(); }

      /// Parses input starting at `begin`. Returns the matched AST node on
      /// success (nullptr on failure). Advances `begin` past consumed input.
      virtual pointer_type parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) = 0;

      /// Matched text. Composite nodes return the whole matched span; leaf
      /// terminals override this to return their own matched value.
      virtual std::string_view get_text() const {
        return (_span_ptr && _span_len) ? std::string_view(_span_ptr, _span_len) : std::string_view();
      }

      /// Adds a matched child node and wires its parent link.
      void add_child(const pointer_type& child) {
        child->_parent = this->weak_from_this();
        super_t::push_back(child);
      }

    protected:
      /// Records the [begin, end) span this node matched so composite nodes can
      /// report their matched text.
      void record_span(iterator_type begin, iterator_type end) {
        _span_len = static_cast<size_t>(std::distance(begin, end));
        _span_ptr = (_span_len > 0) ? &*begin : nullptr;
      }

    private:
      std::weak_ptr<rule_base> _parent;
      const char* _span_ptr = nullptr;
      size_t _span_len = 0;
    };

    // Rule template using CRTP
    template <typename _decl_t, typename _impl_t = _decl_t>
    class rule : public rule_base {
    public:
      using iterator_type = rule_base::iterator_type;
      using pointer_type = rule_base::pointer_type;
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

      // Named rule (decl != impl): delegate to the implementation rule and adopt
      // the parsed implementation as this node's single child. Combinators and
      // terminals (decl == impl) override parse() and never reach the delegating
      // branch.
      pointer_type parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        if constexpr (std::is_same_v<_decl_t, _impl_t>) {
          // No implementation to delegate to; matches empty. Real behavior is
          // supplied by the combinator/terminal overrides.
          this->clear();
          this->record_span(begin, begin);
          return this->shared_from_this();
        }
        else {
          auto backup = begin;
          this->clear();
          auto child = std::make_shared<_impl_t>()->parse(ctx, begin, end);
          if (!child) {
            begin = backup;
            return nullptr;
          }
          this->add_child(child);
          this->record_span(backup, begin);
          return this->shared_from_this();
        }
      }
    };

    // NOT combinator (negative lookahead: matches when the child does NOT match,
    // consumes no input, and produces an empty node).
    template <typename ...> class not_;

    template <> class not_<> : public rule<not_<>> {
    public:
      using iterator_type = rule_base::iterator_type;
      using pointer_type = rule_base::pointer_type;
      using _super_t = rule<not_<>>;
      template <typename ... _child_rule_ts>
      explicit not_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...) {}

      pointer_type parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        (void)ctx; (void)end;
        // No child to negate; treat as a failed assertion.
        ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
          this->type(), begin, "Negative assertion has no child",
          begin != end ? std::string(1, *begin) : "EOF"));
        return nullptr;
      }
    };

    template <typename _head_t, typename ... _tail_ts>
    class not_<_head_t, _tail_ts...> : public rule<not_<_head_t, _tail_ts...>> {
    public:
      using iterator_type = rule_base::iterator_type;
      using pointer_type = rule_base::pointer_type;
      using _super_t = rule<not_<_head_t, _tail_ts...>>;
      template <typename ... _child_rule_ts>
      explicit not_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...) {}

      pointer_type parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        skip_ws(begin, end, ctx);

        auto temp_begin = begin;
        bool all_matched = try_seq<_head_t, _tail_ts...>(ctx, temp_begin, end);

        begin = backup; // negative lookahead never consumes input
        if (all_matched) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, "Negative assertion failed",
            begin != end ? std::string(1, *begin) : "EOF"));
          return nullptr;
        }

        this->clear();
        this->record_span(backup, backup);
        return this->shared_from_this();
      }

    private:
      template <typename _t0, typename ... _tn>
      bool try_seq(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) {
        auto child = std::make_shared<_t0>()->parse(ctx, begin, end);
        if (!child) return false;
        if constexpr (sizeof...(_tn) > 0) {
          return try_seq<_tn...>(ctx, begin, end);
        }
        else {
          return true;
        }
      }
    };

    // AND combinator: matches each child rule in sequence and, on success,
    // instantiates an and_ node holding every matched child in order.
    template <typename ...> class and_;

    template <> class and_<> : public rule<and_<>> {
    public:
      using iterator_type = rule_base::iterator_type;
      using pointer_type = rule_base::pointer_type;
      using _super_t = rule<and_<>>;
      template <typename ... _child_rule_ts>
      explicit and_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...) {}

      pointer_type parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        (void)ctx; (void)end;
        this->clear();
        this->record_span(begin, begin); // empty sequence matches empty
        return this->shared_from_this();
      }
    };

    template <typename _head_t, typename ... _tail_ts>
    class and_<_head_t, _tail_ts...> : public rule<and_<_head_t, _tail_ts...>> {
    public:
      using iterator_type = rule_base::iterator_type;
      using pointer_type = rule_base::pointer_type;
      using _super_t = rule<and_<_head_t, _tail_ts...>>;
      template <typename ... _child_rule_ts>
      explicit and_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...) {}

      pointer_type parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        this->clear();
        if (!try_all<_head_t, _tail_ts...>(ctx, begin, end)) {
          begin = backup;
          this->clear();
          return nullptr;
        }
        this->record_span(backup, begin);
        return this->shared_from_this();
      }

    private:
      template <typename _t0, typename ... _tn>
      bool try_all(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) {
        auto child = std::make_shared<_t0>()->parse(ctx, begin, end);
        if (!child) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), begin, "Sequence element failed",
            begin != end ? std::string(1, *begin) : "EOF"));
          return false;
        }
        this->add_child(child);
        if constexpr (sizeof...(_tn) > 0) {
          return try_all<_tn...>(ctx, begin, end);
        }
        else {
          return true;
        }
      }
    };

    // OR combinator: tries each alternative in order and, on the first success,
    // instantiates an or_ node holding the single matched alternative.
    template <typename ...> class or_;

    template <> class or_<> : public rule<or_<>> {
    public:
      using iterator_type = rule_base::iterator_type;
      using pointer_type = rule_base::pointer_type;
      using _super_t = rule<or_<>>;
      template <typename ... _child_rule_ts>
      explicit or_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...) {}

      pointer_type parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        (void)ctx; (void)begin; (void)end;
        return nullptr; // no alternatives can match
      }
    };

    template <typename _head_t, typename ... _tail_ts>
    class or_<_head_t, _tail_ts...> : public rule<or_<_head_t, _tail_ts...>> {
    public:
      using iterator_type = rule_base::iterator_type;
      using pointer_type = rule_base::pointer_type;
      using _super_t = rule<or_<_head_t, _tail_ts...>>;
      template <typename ... _child_rule_ts>
      explicit or_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...) {}

      pointer_type parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        this->clear();
        auto child = try_any<_head_t, _tail_ts...>(ctx, begin, end, backup);
        if (!child) {
          begin = backup;
          return nullptr;
        }
        this->add_child(child);
        this->record_span(backup, begin);
        return this->shared_from_this();
      }

    private:
      template <typename _t0, typename ... _tn>
      pointer_type try_any(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end, iterator_type backup) {
        auto child = std::make_shared<_t0>()->parse(ctx, begin, end);
        if (child) return child;
        begin = backup;
        if constexpr (sizeof...(_tn) > 0) {
          return try_any<_tn...>(ctx, begin, end, backup);
        }
        else {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, "No alternative matched",
            backup != end ? std::string(1, *backup) : "EOF"));
          return nullptr;
        }
      }
    };

    // ONE_OR_MORE combinator
    template <typename _ty>
    class one_or_more_ : public rule<one_or_more_<_ty>> {
    public:
      using iterator_type = rule_base::iterator_type;
      using pointer_type = rule_base::pointer_type;
      using _super_t = rule<one_or_more_<_ty>>;
      template <typename ... _child_rule_ts>
      explicit one_or_more_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...) {}

      pointer_type parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        this->clear();

        auto first = std::make_shared<_ty>()->parse(ctx, begin, end);
        if (!first) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, "One_or_more requires at least one match",
            begin != end ? std::string(1, *begin) : "EOF"));
          begin = backup;
          return nullptr;
        }
        this->add_child(first);

        while (begin != end) {
          auto loop_backup = begin;
          auto child = std::make_shared<_ty>()->parse(ctx, begin, end);
          if (!child) {
            begin = loop_backup;
            break;
          }
          if (begin == loop_backup) break; // no progress guard
          this->add_child(child);
        }
        this->record_span(backup, begin);
        return this->shared_from_this();
      }
    };

    // ZERO_OR_MORE combinator
    template <typename _ty>
    class zero_or_more_ : public rule<zero_or_more_<_ty>> {
    public:
      using iterator_type = rule_base::iterator_type;
      using pointer_type = rule_base::pointer_type;
      using _super_t = rule<zero_or_more_<_ty>>;
      template <typename ... _child_rule_ts>
      explicit zero_or_more_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...) {}

      pointer_type parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        this->clear();
        while (begin != end) {
          auto loop_backup = begin;
          auto child = std::make_shared<_ty>()->parse(ctx, begin, end);
          if (!child) {
            begin = loop_backup;
            break;
          }
          if (begin == loop_backup) break; // no progress guard
          this->add_child(child);
        }
        this->record_span(backup, begin);
        return this->shared_from_this();
      }
    };

    // ZERO_OR_ONE combinator
    template <typename _ty>
    class zero_or_one_ : public rule<zero_or_one_<_ty>> {
    public:
      using iterator_type = rule_base::iterator_type;
      using pointer_type = rule_base::pointer_type;
      using _super_t = rule<zero_or_one_<_ty>>;
      template <typename ... _child_rule_ts>
      explicit zero_or_one_(_child_rule_ts&& ... oChildRules) : _super_t(std::forward<_child_rule_ts>(oChildRules)...) {}

      pointer_type parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        this->clear();
        auto child = std::make_shared<_ty>()->parse(ctx, begin, end);
        if (child) {
          this->add_child(child);
        }
        else {
          begin = backup;
        }
        this->record_span(backup, begin);
        return this->shared_from_this();
      }
    };

    // STRING terminal
    template <typename _ty, _ty&> class string;
    template <size_t _len, char(&_str)[_len]>
    class string<char[_len], _str> : public rule<string<char[_len], _str>> {
    private:
      std::string _matched;

    public:
      using iterator_type = rule_base::iterator_type;
      using pointer_type = rule_base::pointer_type;
      using _super_t = rule<string<char[_len], _str>>;
      static constexpr size_t length = _len - 1;
      string() = default;

      pointer_type parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        skip_ws(begin, end, ctx);
        constexpr const char* lit = _str;
        auto match_start = begin;
        for (size_t i = 0; i < length; ++i) {
          if (begin == end || !char_match(*begin, lit[i], ctx.ignore_case)) {
            ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
              this->type(), backup, std::string("Expected string: ") + lit,
              begin != end ? std::string(1, *begin) : "EOF"));
            begin = backup;
            return nullptr;
          }
          ++begin;
        }
        _matched = std::string(match_start, begin); // actual matched input
        return this->shared_from_this();
      }

      std::string_view get_text() const override { return _matched; }
    };

    // CHARACTER terminal
    template <char _value>
    class character : public rule<character<_value>> {
    private:
      char _matched = 0;

    public:
      using iterator_type = rule_base::iterator_type;
      using pointer_type = rule_base::pointer_type;

      pointer_type parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        skip_ws(begin, end, ctx);
        if (begin == end || !char_match(*begin, _value, ctx.ignore_case)) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, std::string("Expected character: '") + _value + "'",
            begin != end ? std::string(1, *begin) : "EOF"));
          begin = backup;
          return nullptr;
        }
        _matched = *begin;
        ++begin;
        return this->shared_from_this();
      }

      std::string_view get_text() const override { return std::string_view(&_matched, 1); }
    };

    // CHARACTERS range terminal
    template <char _first, char _last>
    class characters : public rule<characters<_first, _last>> {
    private:
      char _matched = 0;

    public:
      using iterator_type = rule_base::iterator_type;
      using pointer_type = rule_base::pointer_type;
      using _super_t = rule<characters<_first, _last>>;

      pointer_type parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        skip_ws(begin, end, ctx);
        if (begin == end) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, "Expected character in range",
            "EOF"));
          begin = backup;
          return nullptr;
        }

        char c = *begin;
        auto in_range = [](char x) {
          return (_first <= _last) ? (x >= _first && x <= _last) : (x <= _first && x >= _last);
        };
        if (!in_range(c) && !(ctx.ignore_case && in_range(swap_case_ch(c)))) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup,
            std::string("Expected character in range [") + _first + "-" + _last + "]",
            std::string(1, c)));
          begin = backup;
          return nullptr;
        }

        _matched = c;
        ++begin;
        return this->shared_from_this();
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
      using iterator_type = rule_base::iterator_type;
      using pointer_type = rule_base::pointer_type;
      using _super_t = rule<regex<char[_len], _str>>;
      static constexpr size_t length = _len - 1;

      pointer_type parse(context<iterator_type>& ctx, iterator_type& begin, iterator_type& end) override {
        auto backup = begin;
        skip_ws(begin, end, ctx);

        if (begin == end) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, "Expected regex match", "EOF"));
          begin = backup;
          return nullptr;
        }

        std::match_results<iterator_type> matches;
        auto& pattern = get_pattern();

        if (!std::regex_search(begin, end, matches, pattern,
          std::regex_constants::match_continuous)) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, std::string("Expected regex: ") + _str,
            begin != end ? std::string(begin, begin + std::min<size_t>(10, end - begin)) + "..." : "EOF"));
          begin = backup;
          return nullptr;
        }

        if (matches.empty() || matches.position(0) != 0) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<iterator_type>>(
            this->type(), backup, "Regex match not at beginning",
            std::string(begin, begin + std::min<size_t>(10, end - begin)) + "..."));
          begin = backup;
          return nullptr;
        }

        _matched = matches.str(0);
        begin += _matched.length();
        return this->shared_from_this();
      }

      std::string_view get_text() const override { return _matched; }
    };

    // WHITESPACE definition
    template <char..._chs>
    class whitespace {
    public:
      using whitespace_type = whitespace<_chs...>;
      static std::string chars() { return std::string{ _chs... }; }
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
      parse::context<_iterator_t> ctx(begin, end, true, _whitespace_t::chars(), _ignore_case);
      auto start_rule = std::make_shared<_rule_t>();

      auto result = start_rule->parse(ctx, begin, end);
      if (!result) {
        errors = ctx.parse_errors;
        return false;
      }

      if (begin != end) {
        errors.emplace_back(std::make_shared<parse::parse_error<_iterator_t>>(
          typeid(_rule_t), begin, "Expected end of input",
          std::string(begin, begin + std::min<size_t>(10, end - begin)) + "..."));
        return false;
      }

      ast = std::static_pointer_cast<_rule_t>(result);
      return true;
    }

    template <typename _iterator_t>
    static bool parse(_iterator_t begin, _iterator_t end, std::shared_ptr<_rule_t>& ast) {
      typename parse::parse_error<_iterator_t>::vector errors;
      return parse(begin, end, ast, errors);
    }
  };

} // namespace xtd

/** @file
text parsing with a separate, iterable AST node tree.

This is an alternative implementation to `xtd/parse.hpp`. Grammar rules here are
stateless matchers: each rule type exposes a `static parse(...)` that, on a
successful match, produces a distinct `ast_node` (rule type + matched text +
children + parent). The grammar types themselves are never instantiated or
mutated, so the resulting AST is a clean, independent, iterable tree.

@copyright David Mott (c) 2016.
Distributed under the Boost Software License Version 1.0.
See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
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

  namespace parse_ast {

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

    /** A node in the abstract syntax tree.
     *
     * Distinct from the grammar rule types: an `ast_node` records which rule
     * produced it (`rule_type`), the matched text, its child nodes, and its
     * parent. Iterate a node directly to visit its children.
     */
    struct ast_node {
      using pointer_type = std::shared_ptr<ast_node>;
      using vector_type = std::vector<pointer_type>;

      const std::type_info& rule_type;
      std::string text;
      vector_type children;
      std::weak_ptr<ast_node> parent;

      explicit ast_node(const std::type_info& t) : rule_type(t) {}

      const std::type_info& type() const { return rule_type; }
      bool isa(const std::type_info& t) const { return rule_type == t; }

      // Iterable child access
      vector_type::iterator begin() { return children.begin(); }
      vector_type::iterator end() { return children.end(); }
      vector_type::const_iterator begin() const { return children.begin(); }
      vector_type::const_iterator end() const { return children.end(); }
      size_t size() const { return children.size(); }
      bool empty() const { return children.empty(); }
      const pointer_type& operator[](size_t i) const { return children[i]; }

      std::string_view get_text() const { return text; }
    };

    namespace detail {
      /// Attaches a child node to a parent and wires the parent link.
      inline void adopt(const ast_node::pointer_type& parent, const ast_node::pointer_type& child) {
        child->parent = parent;
        parent->children.push_back(child);
      }
      /// Builds the matched-text string for a [begin, end) span.
      template <typename It>
      std::string span_text(It begin, It end) {
        return std::string(begin, end);
      }
    }

    // Named rule wrapper: delegate to the implementation rule and wrap its result
    // in a node typed as the declaration rule. Combinators and terminals define
    // their own static parse and are used directly (not via this wrapper).
    template <typename _decl_t, typename _impl_t = _decl_t>
    struct rule {
      template <typename It>
      static bool parse(context<It>& ctx, It& begin, It& end, ast_node::pointer_type& out) {
        if constexpr (std::is_same_v<_decl_t, _impl_t>) {
          // Degenerate base: empty match. Real behavior comes from overrides.
          out = std::make_shared<ast_node>(typeid(_decl_t));
          return true;
        }
        else {
          auto backup = begin;
          ast_node::pointer_type child;
          if (!_impl_t::parse(ctx, begin, end, child)) {
            begin = backup;
            return false;
          }
          auto node = std::make_shared<ast_node>(typeid(_decl_t));
          detail::adopt(node, child);
          node->text = detail::span_text(backup, begin);
          out = node;
          return true;
        }
      }
    };

    // NOT combinator (negative lookahead)
    template <typename ...> struct not_;

    template <typename _head_t, typename ... _tail_ts>
    struct not_<_head_t, _tail_ts...> {
      template <typename It>
      static bool parse(context<It>& ctx, It& begin, It& end, ast_node::pointer_type& out) {
        auto backup = begin;
        skip_ws(begin, end, ctx);
        auto temp = begin;
        bool matched = try_seq<It, _head_t, _tail_ts...>(ctx, temp, end);
        begin = backup; // negative lookahead never consumes
        if (matched) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<It>>(
            typeid(not_), backup, "Negative assertion failed",
            begin != end ? std::string(1, *begin) : "EOF"));
          return false;
        }
        out = std::make_shared<ast_node>(typeid(not_));
        return true;
      }

    private:
      template <typename It, typename _t0, typename ... _tn>
      static bool try_seq(context<It>& ctx, It& begin, It& end) {
        ast_node::pointer_type child;
        if (!_t0::parse(ctx, begin, end, child)) return false;
        if constexpr (sizeof...(_tn) > 0) {
          return try_seq<It, _tn...>(ctx, begin, end);
        }
        else {
          return true;
        }
      }
    };

    // AND combinator: matches each child in sequence; the node holds every child.
    template <typename ...> struct and_;

    template <> struct and_<> {
      template <typename It>
      static bool parse(context<It>& ctx, It& begin, It& end, ast_node::pointer_type& out) {
        (void)ctx; (void)end;
        out = std::make_shared<ast_node>(typeid(and_)); // empty sequence
        return true;
      }
    };

    template <typename _head_t, typename ... _tail_ts>
    struct and_<_head_t, _tail_ts...> {
      template <typename It>
      static bool parse(context<It>& ctx, It& begin, It& end, ast_node::pointer_type& out) {
        auto backup = begin;
        auto node = std::make_shared<ast_node>(typeid(and_));
        if (!try_all<It, _head_t, _tail_ts...>(ctx, begin, end, node)) {
          begin = backup;
          return false;
        }
        node->text = detail::span_text(backup, begin);
        out = node;
        return true;
      }

    private:
      template <typename It, typename _t0, typename ... _tn>
      static bool try_all(context<It>& ctx, It& begin, It& end, const ast_node::pointer_type& node) {
        ast_node::pointer_type child;
        if (!_t0::parse(ctx, begin, end, child)) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<It>>(
            typeid(and_), begin, "Sequence element failed",
            begin != end ? std::string(1, *begin) : "EOF"));
          return false;
        }
        detail::adopt(node, child);
        if constexpr (sizeof...(_tn) > 0) {
          return try_all<It, _tn...>(ctx, begin, end, node);
        }
        else {
          return true;
        }
      }
    };

    // OR combinator: tries each alternative; the node holds the single winner.
    template <typename ...> struct or_;

    template <> struct or_<> {
      template <typename It>
      static bool parse(context<It>& ctx, It& begin, It& end, ast_node::pointer_type& out) {
        (void)ctx; (void)begin; (void)end; (void)out;
        return false; // no alternatives can match
      }
    };

    template <typename _head_t, typename ... _tail_ts>
    struct or_<_head_t, _tail_ts...> {
      template <typename It>
      static bool parse(context<It>& ctx, It& begin, It& end, ast_node::pointer_type& out) {
        auto backup = begin;
        ast_node::pointer_type child;
        if (!try_any<It, _head_t, _tail_ts...>(ctx, begin, end, backup, child)) {
          begin = backup;
          return false;
        }
        auto node = std::make_shared<ast_node>(typeid(or_));
        detail::adopt(node, child);
        node->text = detail::span_text(backup, begin);
        out = node;
        return true;
      }

    private:
      template <typename It, typename _t0, typename ... _tn>
      static bool try_any(context<It>& ctx, It& begin, It& end, It backup, ast_node::pointer_type& child) {
        if (_t0::parse(ctx, begin, end, child)) return true;
        begin = backup;
        if constexpr (sizeof...(_tn) > 0) {
          return try_any<It, _tn...>(ctx, begin, end, backup, child);
        }
        else {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<It>>(
            typeid(or_), backup, "No alternative matched",
            backup != end ? std::string(1, *backup) : "EOF"));
          return false;
        }
      }
    };

    // ONE_OR_MORE combinator
    template <typename _ty>
    struct one_or_more_ {
      template <typename It>
      static bool parse(context<It>& ctx, It& begin, It& end, ast_node::pointer_type& out) {
        auto backup = begin;
        auto node = std::make_shared<ast_node>(typeid(one_or_more_));
        ast_node::pointer_type child;
        if (!_ty::parse(ctx, begin, end, child)) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<It>>(
            typeid(one_or_more_), backup, "One_or_more requires at least one match",
            begin != end ? std::string(1, *begin) : "EOF"));
          begin = backup;
          return false;
        }
        detail::adopt(node, child);
        while (begin != end) {
          auto loop_backup = begin;
          ast_node::pointer_type next;
          if (!_ty::parse(ctx, begin, end, next)) {
            begin = loop_backup;
            break;
          }
          if (begin == loop_backup) break; // no progress guard
          detail::adopt(node, next);
        }
        node->text = detail::span_text(backup, begin);
        out = node;
        return true;
      }
    };

    // ZERO_OR_MORE combinator
    template <typename _ty>
    struct zero_or_more_ {
      template <typename It>
      static bool parse(context<It>& ctx, It& begin, It& end, ast_node::pointer_type& out) {
        auto backup = begin;
        auto node = std::make_shared<ast_node>(typeid(zero_or_more_));
        while (begin != end) {
          auto loop_backup = begin;
          ast_node::pointer_type child;
          if (!_ty::parse(ctx, begin, end, child)) {
            begin = loop_backup;
            break;
          }
          if (begin == loop_backup) break; // no progress guard
          detail::adopt(node, child);
        }
        node->text = detail::span_text(backup, begin);
        out = node;
        return true;
      }
    };

    // ZERO_OR_ONE combinator
    template <typename _ty>
    struct zero_or_one_ {
      template <typename It>
      static bool parse(context<It>& ctx, It& begin, It& end, ast_node::pointer_type& out) {
        auto backup = begin;
        auto node = std::make_shared<ast_node>(typeid(zero_or_one_));
        ast_node::pointer_type child;
        if (_ty::parse(ctx, begin, end, child)) {
          detail::adopt(node, child);
        }
        else {
          begin = backup;
        }
        node->text = detail::span_text(backup, begin);
        out = node;
        return true;
      }
    };

    // STRING terminal
    template <typename _ty, _ty&> struct string;
    template <size_t _len, char(&_str)[_len]>
    struct string<char[_len], _str> {
      static constexpr size_t length = _len - 1;
      template <typename It>
      static bool parse(context<It>& ctx, It& begin, It& end, ast_node::pointer_type& out) {
        auto backup = begin;
        skip_ws(begin, end, ctx);
        constexpr const char* lit = _str;
        auto match_start = begin;
        for (size_t i = 0; i < length; ++i) {
          if (begin == end || !char_match(*begin, lit[i], ctx.ignore_case)) {
            ctx.parse_errors.emplace_back(std::make_shared<parse_error<It>>(
              typeid(string), backup, std::string("Expected string: ") + lit,
              begin != end ? std::string(1, *begin) : "EOF"));
            begin = backup;
            return false;
          }
          ++begin;
        }
        auto node = std::make_shared<ast_node>(typeid(string));
        node->text = std::string(match_start, begin); // actual matched input
        out = node;
        return true;
      }
    };

    // CHARACTER terminal
    template <char _value>
    struct character {
      template <typename It>
      static bool parse(context<It>& ctx, It& begin, It& end, ast_node::pointer_type& out) {
        auto backup = begin;
        skip_ws(begin, end, ctx);
        if (begin == end || !char_match(*begin, _value, ctx.ignore_case)) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<It>>(
            typeid(character), backup, std::string("Expected character: '") + _value + "'",
            begin != end ? std::string(1, *begin) : "EOF"));
          begin = backup;
          return false;
        }
        auto node = std::make_shared<ast_node>(typeid(character));
        node->text = std::string(1, *begin);
        ++begin;
        out = node;
        return true;
      }
    };

    // CHARACTERS range terminal
    template <char _first, char _last>
    struct characters {
      template <typename It>
      static bool parse(context<It>& ctx, It& begin, It& end, ast_node::pointer_type& out) {
        auto backup = begin;
        skip_ws(begin, end, ctx);
        if (begin == end) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<It>>(
            typeid(characters), backup, "Expected character in range", "EOF"));
          begin = backup;
          return false;
        }
        char c = *begin;
        auto in_range = [](char x) {
          return (_first <= _last) ? (x >= _first && x <= _last) : (x <= _first && x >= _last);
        };
        if (!in_range(c) && !(ctx.ignore_case && in_range(swap_case_ch(c)))) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<It>>(
            typeid(characters), backup, "Expected character in range", std::string(1, c)));
          begin = backup;
          return false;
        }
        auto node = std::make_shared<ast_node>(typeid(characters));
        node->text = std::string(1, c);
        ++begin;
        out = node;
        return true;
      }
    };

    // REGEX terminal
    template <typename _ty, _ty&> struct regex;
    template <size_t _len, char(&_str)[_len]>
    struct regex<char[_len], _str> {
      static constexpr size_t length = _len - 1;
    private:
      static std::regex& get_pattern() {
        static std::regex pattern(_str);
        return pattern;
      }
    public:
      template <typename It>
      static bool parse(context<It>& ctx, It& begin, It& end, ast_node::pointer_type& out) {
        auto backup = begin;
        skip_ws(begin, end, ctx);
        if (begin == end) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<It>>(
            typeid(regex), backup, "Expected regex match", "EOF"));
          begin = backup;
          return false;
        }
        std::match_results<It> matches;
        auto& pattern = get_pattern();
        if (!std::regex_search(begin, end, matches, pattern,
          std::regex_constants::match_continuous) || matches.empty() || matches.position(0) != 0) {
          ctx.parse_errors.emplace_back(std::make_shared<parse_error<It>>(
            typeid(regex), backup, std::string("Expected regex: ") + _str,
            begin != end ? std::string(begin, begin + std::min<size_t>(10, end - begin)) : "EOF"));
          begin = backup;
          return false;
        }
        auto node = std::make_shared<ast_node>(typeid(regex));
        node->text = matches.str(0);
        begin += node->text.length();
        out = node;
        return true;
      }
    };

    // WHITESPACE definition
    template <char..._chs>
    struct whitespace {
      using whitespace_type = whitespace<_chs...>;
      static std::string chars() { return std::string{ _chs... }; }
    };

    // Macros for convenience (distinct names to coexist with xtd/parse.hpp macros)
#define AST_STRING_(x) \
        namespace _ { char x[] = #x; } \
        using x = xtd::parse_ast::string<decltype(_::x), _::x>;

#define AST_STRING(_name, _value) \
        namespace _ { char _name[] = _value; } \
        using _name = xtd::parse_ast::string<decltype(_::_name), _::_name>;

#define AST_CHARACTER_(_name, _value) \
        using _name = xtd::parse_ast::character<_value>

#define AST_CHARACTERS_(_name, _first, _last) \
        using _name = xtd::parse_ast::characters<_first, _last>

#define AST_REGEX(_name, _value) \
        namespace _ { char _name[] = _value; } \
        using _name = xtd::parse_ast::regex<decltype(_::_name), _::_name>;

    // End of file marker
    struct EndOfFile {
      using impl_type = EndOfFile;
    };

    // Main parser class: produces a separate ast_node tree.
    template <typename _rule_t, bool _ignore_case = false, typename _whitespace_t = whitespace<' ', '\t', '\n', '\r'>>
    struct parser {
      template <typename _iterator_t>
      static bool parse(_iterator_t begin, _iterator_t end,
        ast_node::pointer_type& ast,
        typename parse_error<_iterator_t>::vector& errors) {
        context<_iterator_t> ctx(begin, end, true, _whitespace_t::chars(), _ignore_case);
        ast_node::pointer_type result;
        if (!_rule_t::parse(ctx, begin, end, result)) {
          errors = ctx.parse_errors;
          return false;
        }
        if (begin != end) {
          errors.emplace_back(std::make_shared<parse_error<_iterator_t>>(
            typeid(_rule_t), begin, "Expected end of input",
            std::string(begin, begin + std::min<size_t>(10, end - begin))));
          return false;
        }
        ast = result;
        return true;
      }

      template <typename _iterator_t>
      static bool parse(_iterator_t begin, _iterator_t end, ast_node::pointer_type& ast) {
        typename parse_error<_iterator_t>::vector errors;
        return parse(begin, end, ast, errors);
      }
    };

  } // namespace parse_ast

} // namespace xtd

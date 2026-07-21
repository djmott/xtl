/** @file
STL-ish map using an on-disk b-tree
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

A memory mapped B+ tree implementing an STL-like unique-key map persisted on disk.

Layout
------
The backing store is a single file divided into fixed-size pages accessed through
xtd::mapped_file. Page 0 is the file header. All other pages are either branch
(internal) nodes, leaf nodes, or entries on the embedded free-page list.

  * file header (page 0) : root page index, leftmost-leaf index, element count,
    tree height, and the head/length of the embedded free-page list.
  * branch page          : sorted separator keys and child page indices.
  * leaf page            : sorted key/value records; leaves are doubly linked so
    the container can be iterated in key order.
  * free page            : a node returned to the allocator; its body holds the
    index of the next free page, forming a singly linked stack whose head lives
    in the file header.

Because pages are memory mapped and shared, key and value types must be trivially
copyable. A least-recently-used cache keeps hot pages mapped.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <utility>

#include <xtd/lru_cache.hpp>
#include <xtd/mapped_file.hpp>
#include <xtd/memory.hpp>
#include <xtd/filesystem.hpp>
#include <xtd/debug.hpp>

namespace xtd{

  namespace _{
    namespace btree{

      /// discriminates the kind of data stored on a page (value 0 == fresh/zeroed page)
      enum class page_type : uint32_t{
        free_page = 0,
        file_header_page,
        leaf_page,
        branch_page,
      };

      /// magic value stamped into a freshly initialized file header
      static const uint64_t file_magic = 0x0eb712ee42a9c001ULL;

      /// common prefix of every page; the discriminator lives at offset 0
      PACK_PUSH(1);
      struct page_base{
        page_type _page_type;
      };
      PACK_POP();

      /// file header, present only on page 0
      PACK_PUSH(1);
      struct file_header : page_base{
        uint64_t _magic;
        size_t _root_page;        ///< index of the root page, 0 when the tree is empty
        size_t _first_leaf;       ///< index of the leftmost leaf, 0 when empty
        size_t _count;            ///< number of key/value pairs in the container
        size_t _height;           ///< number of levels in the tree
        size_t _free_list_head;   ///< index of the first free page, 0 when none
        size_t _free_list_count;  ///< number of pages on the free list
      };
      PACK_POP();

      /// a page returned to the allocator, threaded onto the embedded free list
      PACK_PUSH(1);
      struct free_page : page_base{
        size_t _next;             ///< index of the next free page, 0 when last
      };
      PACK_POP();

      /// resolves the effective page size, honoring the -1 "use system page size" sentinel
      template <size_t _page_size> struct page_sizer{ static size_t value(){ return _page_size; } };
      template <> struct page_sizer<static_cast<size_t>(-1)>{ static size_t value(){ return xtd::memory::page_size(); } };

      /** loads pages on demand for the lru_cache; refers to a mapped_file owned elsewhere
      @tparam _page_size size of a page or -1 for the system page size

      A pointer (rather than an owning member) keeps the loader freely copyable and
      movable, which matters because mapped_file is not movable (it closes its OS
      handles in its destructor).
      */
      template <size_t _page_size>
      class page_loader{
        template <size_t, size_t> friend class page_cache;
        mapped_file<_page_size>* _file;
      public:
        explicit page_loader(mapped_file<_page_size>& oFile) : _file(&oFile){}
        page_loader(const page_loader&) = default;
        page_loader(page_loader&&) = default;

        mapped_page<page_base> operator()(size_t pagenum){
          return _file->template get<page_base>(pagenum);
        }
      };

      /** an lru_cache of mapped pages that can also grow the backing file
      @tparam _page_size size of a page or -1 for the system page size
      @tparam _cache_size number of pages kept resident
      */
      template <size_t _page_size, size_t _cache_size>
      class page_cache : public xtd::lru_cache<size_t, mapped_page<page_base>, _cache_size, page_loader<_page_size> >{
        using _super_t = xtd::lru_cache<size_t, mapped_page<page_base>, _cache_size, page_loader<_page_size> >;
      public:
        explicit page_cache(mapped_file<_page_size>& oFile) : _super_t(page_loader<_page_size>(oFile)){}

        /// appends a brand new page at the end of the file and caches it
        mapped_page<page_base> append(size_t& newpage){
          if (_super_t::size() >= _cache_size){
            _super_t::pop_back();
          }
          auto oPage = _super_t::_loader._file->template append<page_base>(newpage);
          _super_t::push_front(std::make_pair(newpage, oPage));
          return _super_t::front().second;
        }
      };

    }
  }

  /** on-disk, STL-like unique-key map backed by a memory mapped B+ tree
  @tparam _key_t key type (must be trivially copyable and less-than comparable)
  @tparam _value_t mapped value type (must be trivially copyable)
  @tparam _order maximum records per node, or 0 to derive a value from the page size
  @tparam _page_size size of a page or -1 for the system page size
  @tparam _cache_size number of resident pages in the lru cache
  */
  template <typename _key_t, typename _value_t, size_t _order = 0, size_t _page_size = static_cast<size_t>(-1), size_t _cache_size = 64>
  class btree{
  public:
    using key_type = _key_t;
    using mapped_type = _value_t;
    using value_type = std::pair<_key_t, _value_t>;

  private:
    using page_type = _::btree::page_type;
    using page_base = _::btree::page_base;
    using file_header_t = _::btree::file_header;
    using free_page_t = _::btree::free_page;
    using cache_t = _::btree::page_cache<_page_size, _cache_size>;

    static constexpr size_t ptr_size = sizeof(size_t);
    static constexpr size_t leaf_slot = sizeof(_key_t) + sizeof(_value_t);
    static constexpr size_t branch_slot = sizeof(_key_t) + ptr_size;
    static constexpr size_t slot = leaf_slot > branch_slot ? leaf_slot : branch_slot;
    /// conservative lower bound of the runtime page size, used for the compile-time default order
    static constexpr size_t assumed_page = (_page_size == static_cast<size_t>(-1)) ? 4096 : _page_size;
    static constexpr size_t header_reserve = 64;
    static constexpr size_t auto_order = (assumed_page > header_reserve + 4 * slot) ? (assumed_page - header_reserve) / slot : 4;

  public:
    /// maximum number of records held by a single node
    static constexpr size_t order = _order ? _order : auto_order;

  private:
    static_assert(order >= 2, "btree order must be at least 2");

    /// leaf node: sorted key/value records, doubly linked to its siblings
    PACK_PUSH(1);
    struct leaf_page : page_base{
      size_t _count;
      size_t _next_page;
      size_t _prev_page;
      struct record{
        key_type _key;
        mapped_type _value;
      };
      record _records[order];
    };
    PACK_POP();

    /// branch node: order separator keys and order+1 child page indices
    PACK_PUSH(1);
    struct branch_page : page_base{
      size_t _count;
      key_type _keys[order];
      size_t _children[order + 1];
    };
    PACK_POP();

    static_assert(sizeof(leaf_page) <= assumed_page, "leaf node does not fit in a page; reduce order");
    static_assert(sizeof(branch_page) <= assumed_page, "branch node does not fit in a page; reduce order");

    using leaf_t = leaf_page;
    using branch_t = branch_page;

    mutable mapped_file<_page_size> _file; ///< declared before _cache: the cache refers to it
    mutable cache_t _cache;

    // ---- page access helpers -------------------------------------------------

    mapped_page<file_header_t> header() const { return static_page_cast<file_header_t>(_cache[0]); }
    mapped_page<leaf_t> get_leaf(size_t idx) const { return static_page_cast<leaf_t>(_cache[idx]); }
    mapped_page<branch_t> get_branch(size_t idx) const { return static_page_cast<branch_t>(_cache[idx]); }
    page_type type_of(size_t idx) const { return _cache[idx]->_page_type; }

    // ---- allocation ----------------------------------------------------------

    /// obtains a page, reusing one from the embedded free list when available
    mapped_page<page_base> allocate_raw(size_t& idx){
      auto oHeader = header();
      if (oHeader->_free_list_head){
        idx = oHeader->_free_list_head;
        auto oFree = static_page_cast<free_page_t>(_cache[idx]);
        oHeader->_free_list_head = oFree->_next;
        oHeader->_free_list_count--;
        return _cache[idx];
      }
      return _cache.append(idx);
    }

    mapped_page<leaf_t> new_leaf(size_t& idx){
      auto oPage = allocate_raw(idx);
      auto oLeaf = static_page_cast<leaf_t>(oPage);
      oLeaf->_page_type = page_type::leaf_page;
      oLeaf->_count = 0;
      oLeaf->_next_page = 0;
      oLeaf->_prev_page = 0;
      return oLeaf;
    }

    mapped_page<branch_t> new_branch(size_t& idx){
      auto oPage = allocate_raw(idx);
      auto oBranch = static_page_cast<branch_t>(oPage);
      oBranch->_page_type = page_type::branch_page;
      oBranch->_count = 0;
      return oBranch;
    }

    /// links a page onto the head of the embedded free list
    void free_page_at(size_t idx){
      auto oHeader = header();
      auto oFree = static_page_cast<free_page_t>(_cache[idx]);
      oFree->_page_type = page_type::free_page;
      oFree->_next = oHeader->_free_list_head;
      oHeader->_free_list_head = idx;
      oHeader->_free_list_count++;
    }

    // ---- search helpers ------------------------------------------------------

    /// first record index whose key is not less than @p key
    static size_t lower_bound_leaf(const mapped_page<leaf_t>& oLeaf, const key_type& key){
      size_t lo = 0, hi = oLeaf->_count;
      while (lo < hi){
        size_t mid = lo + (hi - lo) / 2;
        if (oLeaf->_records[mid]._key < key) lo = mid + 1; else hi = mid;
      }
      return lo;
    }

    /// index of the child subtree that must contain @p key
    static size_t child_index(const mapped_page<branch_t>& oBranch, const key_type& key){
      size_t lo = 0, hi = oBranch->_count;
      while (lo < hi){
        size_t mid = lo + (hi - lo) / 2;
        if (key < oBranch->_keys[mid]) hi = mid; else lo = mid + 1;
      }
      return lo;
    }

    /// descends from the root to the leaf that would hold @p key
    size_t find_leaf(const key_type& key) const {
      size_t node = header()->_root_page;
      while (node && type_of(node) == page_type::branch_page){
        auto oBranch = get_branch(node);
        node = oBranch->_children[child_index(oBranch, key)];
      }
      return node;
    }

    // ---- insertion -----------------------------------------------------------

    struct insert_result{
      bool inserted;      ///< true when a new record was added (false on duplicate key)
      bool split;         ///< true when the visited node split and promoted a key
      key_type promo_key; ///< separator key promoted to the parent on split
      size_t right_idx;   ///< page index of the new right sibling on split
    };

    static void leaf_insert_at(const mapped_page<leaf_t>& oLeaf, size_t pos, const key_type& key, const mapped_type& value){
      for (size_t i = oLeaf->_count; i > pos; --i) oLeaf->_records[i] = oLeaf->_records[i - 1];
      oLeaf->_records[pos]._key = key;
      oLeaf->_records[pos]._value = value;
      oLeaf->_count++;
    }

    insert_result insert_leaf(size_t idx, const key_type& key, const mapped_type& value){
      auto oLeaf = get_leaf(idx);
      size_t pos = lower_bound_leaf(oLeaf, key);
      if (pos < oLeaf->_count && oLeaf->_records[pos]._key == key){
        return { false, false, key, 0 };
      }
      if (oLeaf->_count < order){
        leaf_insert_at(oLeaf, pos, key, value);
        return { true, false, key, 0 };
      }
      // full: split into a new right sibling, then insert into the proper half
      size_t ridx;
      auto oRight = new_leaf(ridx);
      oLeaf = get_leaf(idx);
      size_t mid = order / 2;
      oRight->_count = order - mid;
      for (size_t i = 0; i < oRight->_count; ++i) oRight->_records[i] = oLeaf->_records[mid + i];
      oLeaf->_count = mid;
      // maintain the doubly linked leaf chain
      oRight->_next_page = oLeaf->_next_page;
      oRight->_prev_page = idx;
      oLeaf->_next_page = ridx;
      if (oRight->_next_page){
        auto oAfter = get_leaf(oRight->_next_page);
        oAfter->_prev_page = ridx;
      }
      if (key < oRight->_records[0]._key){
        leaf_insert_at(oLeaf, lower_bound_leaf(oLeaf, key), key, value);
      } else {
        leaf_insert_at(oRight, lower_bound_leaf(oRight, key), key, value);
      }
      return { true, true, oRight->_records[0]._key, ridx };
    }

    static void branch_insert_at(const mapped_page<branch_t>& oBranch, size_t at, const key_type& sep, size_t right_child){
      for (size_t i = oBranch->_count; i > at; --i) oBranch->_keys[i] = oBranch->_keys[i - 1];
      for (size_t i = oBranch->_count + 1; i > at + 1; --i) oBranch->_children[i] = oBranch->_children[i - 1];
      oBranch->_keys[at] = sep;
      oBranch->_children[at + 1] = right_child;
      oBranch->_count++;
    }

    insert_result insert_branch(size_t idx, const key_type& key, const mapped_type& value){
      auto oBranch = get_branch(idx);
      size_t ci = child_index(oBranch, key);
      size_t child = oBranch->_children[ci];
      auto res = insert_rec(child, key, value);
      if (!res.split) return res;

      oBranch = get_branch(idx);
      if (oBranch->_count < order){
        branch_insert_at(oBranch, ci, res.promo_key, res.right_idx);
        return { res.inserted, false, key, 0 };
      }
      // full branch: build the over-full sequence, split around the median, promote it
      key_type tkeys[order + 1];
      size_t tchildren[order + 2];
      for (size_t i = 0; i < oBranch->_count; ++i) tkeys[i] = oBranch->_keys[i];
      for (size_t i = 0; i < oBranch->_count + 1; ++i) tchildren[i] = oBranch->_children[i];
      for (size_t i = oBranch->_count; i > ci; --i) tkeys[i] = tkeys[i - 1];
      for (size_t i = oBranch->_count + 1; i > ci + 1; --i) tchildren[i] = tchildren[i - 1];
      tkeys[ci] = res.promo_key;
      tchildren[ci + 1] = res.right_idx;

      size_t total = order + 1;
      size_t mid = total / 2;
      key_type promo = tkeys[mid];

      oBranch = get_branch(idx);
      oBranch->_count = mid;
      for (size_t i = 0; i < mid; ++i) oBranch->_keys[i] = tkeys[i];
      for (size_t i = 0; i < mid + 1; ++i) oBranch->_children[i] = tchildren[i];

      size_t ridx;
      auto oRight = new_branch(ridx);
      oRight->_count = total - mid - 1;
      for (size_t i = 0; i < oRight->_count; ++i) oRight->_keys[i] = tkeys[mid + 1 + i];
      for (size_t i = 0; i < oRight->_count + 1; ++i) oRight->_children[i] = tchildren[mid + 1 + i];

      return { res.inserted, true, promo, ridx };
    }

    insert_result insert_rec(size_t idx, const key_type& key, const mapped_type& value){
      if (type_of(idx) == page_type::leaf_page) return insert_leaf(idx, key, value);
      return insert_branch(idx, key, value);
    }

    // ---- erase ---------------------------------------------------------------

    struct erase_result{
      bool erased;      ///< true when a record was removed
      bool child_empty; ///< true when the visited node became empty and should be reclaimed
    };

    /// removes child @p ci (and its separator) from a branch, reclaiming the child page
    void remove_child(size_t bidx, size_t ci){
      auto oBranch = get_branch(bidx);
      size_t cidx = oBranch->_children[ci];
      if (type_of(cidx) == page_type::leaf_page){
        auto oLeaf = get_leaf(cidx);
        size_t p = oLeaf->_prev_page, n = oLeaf->_next_page;
        if (p) get_leaf(p)->_next_page = n;
        if (n) get_leaf(n)->_prev_page = p;
        auto oHeader = header();
        if (oHeader->_first_leaf == cidx) oHeader->_first_leaf = n;
      }
      free_page_at(cidx);
      oBranch = get_branch(bidx);
      size_t cnt = oBranch->_count;
      if (ci == cnt){
        if (cnt > 0) oBranch->_count--; // drops separator keys[cnt-1] and child children[cnt]
      } else {
        for (size_t i = ci; i < cnt; ++i) oBranch->_children[i] = oBranch->_children[i + 1];
        for (size_t i = ci; i + 1 < cnt; ++i) oBranch->_keys[i] = oBranch->_keys[i + 1];
        oBranch->_count--;
      }
    }

    erase_result erase_rec(size_t idx, const key_type& key){
      if (type_of(idx) == page_type::leaf_page){
        auto oLeaf = get_leaf(idx);
        size_t pos = lower_bound_leaf(oLeaf, key);
        if (!(pos < oLeaf->_count && oLeaf->_records[pos]._key == key)) return { false, false };
        for (size_t i = pos; i + 1 < oLeaf->_count; ++i) oLeaf->_records[i] = oLeaf->_records[i + 1];
        oLeaf->_count--;
        return { true, oLeaf->_count == 0 };
      }
      auto oBranch = get_branch(idx);
      size_t ci = child_index(oBranch, key);
      auto res = erase_rec(oBranch->_children[ci], key);
      if (!res.erased) return { false, false };
      if (res.child_empty){
        oBranch = get_branch(idx);
        bool becomes_empty = (oBranch->_count == 0); // the removed child was the only one
        remove_child(idx, ci);
        return { true, becomes_empty };
      }
      return { true, false };
    }

    // ---- teardown ------------------------------------------------------------

    void free_subtree(size_t idx){
      if (type_of(idx) == page_type::branch_page){
        auto oBranch = get_branch(idx);
        size_t n = oBranch->_count + 1;
        size_t kids[order + 1];
        for (size_t i = 0; i < n; ++i) kids[i] = oBranch->_children[i];
        for (size_t i = 0; i < n; ++i) free_subtree(kids[i]);
      }
      free_page_at(idx);
    }

  public:

    /// opens (creating if needed) the b-tree stored at @p oPath
    explicit btree(const xtd::filesystem::path& oPath) : _file(oPath), _cache(_file){
      auto oHeader = header();
      if (oHeader->_magic != _::btree::file_magic){
        oHeader->_page_type = page_type::file_header_page;
        oHeader->_magic = _::btree::file_magic;
        oHeader->_root_page = 0;
        oHeader->_first_leaf = 0;
        oHeader->_count = 0;
        oHeader->_height = 0;
        oHeader->_free_list_head = 0;
        oHeader->_free_list_count = 0;
      }
    }

    btree(const btree&) = delete;
    btree& operator=(const btree&) = delete;

    // ---- iteration -----------------------------------------------------------

    /// forward, key-ordered iterator over the leaf record chain
    class const_iterator{
      friend class btree;
      const btree* _tree;
      size_t _leaf_idx;   ///< 0 designates end()
      size_t _pos;
      mapped_page<leaf_t> _leaf; ///< pins the current leaf so dereferences stay valid

      const_iterator(const btree* tree, size_t leaf_idx, size_t pos, mapped_page<leaf_t> leaf)
        : _tree(tree), _leaf_idx(leaf_idx), _pos(pos), _leaf(std::move(leaf)){}

      /// advances to the next non-empty leaf, or becomes end()
      void skip_to_nonempty(){
        while (_leaf_idx && _leaf->_count == 0){
          size_t nxt = _leaf->_next_page;
          if (!nxt){ *this = _tree->end(); return; }
          _leaf_idx = nxt;
          _pos = 0;
          _leaf = _tree->get_leaf(nxt);
        }
      }

    public:
      const_iterator() : _tree(nullptr), _leaf_idx(0), _pos(0), _leaf(){}

      bool operator==(const const_iterator& rhs) const { return _leaf_idx == rhs._leaf_idx && _pos == rhs._pos; }
      bool operator!=(const const_iterator& rhs) const { return !(*this == rhs); }

      value_type operator*() const { return value_type(_leaf->_records[_pos]._key, _leaf->_records[_pos]._value); }

      const key_type& key() const { return _leaf->_records[_pos]._key; }
      const mapped_type& value() const { return _leaf->_records[_pos]._value; }

      struct arrow_proxy{
        value_type _pair;
        const value_type* operator->() const { return &_pair; }
      };
      arrow_proxy operator->() const { return arrow_proxy{ **this }; }

      const_iterator& operator++(){
        ++_pos;
        if (_pos >= _leaf->_count){
          size_t nxt = _leaf->_next_page;
          if (!nxt){ *this = _tree->end(); return *this; }
          _leaf_idx = nxt;
          _pos = 0;
          _leaf = _tree->get_leaf(nxt);
          skip_to_nonempty();
        }
        return *this;
      }

      const_iterator operator++(int){ const_iterator tmp(*this); ++(*this); return tmp; }
    };

    using iterator = const_iterator;

    const_iterator end() const { return const_iterator(this, 0, 0, mapped_page<leaf_t>()); }

    const_iterator begin() const {
      auto oHeader = header();
      if (oHeader->_count == 0 || oHeader->_first_leaf == 0) return end();
      const_iterator it(this, oHeader->_first_leaf, 0, get_leaf(oHeader->_first_leaf));
      it.skip_to_nonempty();
      return it;
    }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    // ---- capacity ------------------------------------------------------------

    size_t size() const { return header()->_count; }
    bool empty() const { return size() == 0; }

    // ---- lookup --------------------------------------------------------------

    const_iterator find(const key_type& key) const {
      size_t leaf = find_leaf(key);
      if (!leaf) return end();
      auto oLeaf = get_leaf(leaf);
      size_t pos = lower_bound_leaf(oLeaf, key);
      if (pos < oLeaf->_count && oLeaf->_records[pos]._key == key) return const_iterator(this, leaf, pos, oLeaf);
      return end();
    }

    bool contains(const key_type& key) const { return find(key) != end(); }
    size_t count(const key_type& key) const { return contains(key) ? 1 : 0; }

    /// returns a copy of the value for @p key or throws std::out_of_range
    mapped_type at(const key_type& key) const {
      size_t leaf = find_leaf(key);
      if (leaf){
        auto oLeaf = get_leaf(leaf);
        size_t pos = lower_bound_leaf(oLeaf, key);
        if (pos < oLeaf->_count && oLeaf->_records[pos]._key == key) return oLeaf->_records[pos]._value;
      }
      throw std::out_of_range("xtd::btree::at: key not found");
    }

    /// copies the value for @p key into @p out, returning false when absent
    bool find_value(const key_type& key, mapped_type& out) const {
      size_t leaf = find_leaf(key);
      if (!leaf) return false;
      auto oLeaf = get_leaf(leaf);
      size_t pos = lower_bound_leaf(oLeaf, key);
      if (pos < oLeaf->_count && oLeaf->_records[pos]._key == key){ out = oLeaf->_records[pos]._value; return true; }
      return false;
    }

    // ---- modifiers -----------------------------------------------------------

    /** inserts @p value under @p key without overwriting an existing entry
    @return true when a new element was inserted, false when @p key already exists
    */
    bool insert(const key_type& key, const mapped_type& value){
      auto oHeader = header();
      if (oHeader->_root_page == 0){
        size_t lidx;
        auto oLeaf = new_leaf(lidx);
        oLeaf->_records[0]._key = key;
        oLeaf->_records[0]._value = value;
        oLeaf->_count = 1;
        oHeader = header();
        oHeader->_root_page = lidx;
        oHeader->_first_leaf = lidx;
        oHeader->_height = 1;
        oHeader->_count = 1;
        return true;
      }
      auto res = insert_rec(oHeader->_root_page, key, value);
      if (res.split){
        size_t nr;
        auto oRoot = new_branch(nr);
        oHeader = header();
        oRoot->_count = 1;
        oRoot->_keys[0] = res.promo_key;
        oRoot->_children[0] = oHeader->_root_page;
        oRoot->_children[1] = res.right_idx;
        oHeader->_root_page = nr;
        oHeader->_height++;
      }
      if (res.inserted){
        header()->_count++;
        return true;
      }
      return false;
    }

    bool insert(const value_type& kv){ return insert(kv.first, kv.second); }

    /// overwrites the value for @p key if present, otherwise inserts it
    void insert_or_assign(const key_type& key, const mapped_type& value){
      if (!update(key, value)) insert(key, value);
    }

    /// overwrites the value for an existing @p key; returns false when absent
    bool update(const key_type& key, const mapped_type& value){
      size_t leaf = find_leaf(key);
      if (!leaf) return false;
      auto oLeaf = get_leaf(leaf);
      size_t pos = lower_bound_leaf(oLeaf, key);
      if (pos < oLeaf->_count && oLeaf->_records[pos]._key == key){ oLeaf->_records[pos]._value = value; return true; }
      return false;
    }

    /** removes @p key
    @return 1 when an element was removed, 0 otherwise
    */
    size_t erase(const key_type& key){
      auto oHeader = header();
      if (oHeader->_root_page == 0) return 0;
      auto res = erase_rec(oHeader->_root_page, key);
      if (!res.erased) return 0;
      oHeader = header();
      oHeader->_count--;
      if (res.child_empty){
        free_page_at(oHeader->_root_page);
        oHeader = header();
        oHeader->_root_page = 0;
        oHeader->_first_leaf = 0;
        oHeader->_height = 0;
        return 1;
      }
      // collapse a chain of single-child root branches to keep the tree shallow
      while (oHeader->_root_page && type_of(oHeader->_root_page) == page_type::branch_page){
        auto oRoot = get_branch(oHeader->_root_page);
        if (oRoot->_count != 0) break;
        size_t only = oRoot->_children[0];
        size_t old = oHeader->_root_page;
        oHeader->_root_page = only;
        oHeader->_height--;
        free_page_at(old);
        oHeader = header();
      }
      return 1;
    }

    /// removes every element, returning all pages to the embedded free list
    void clear(){
      auto oHeader = header();
      if (oHeader->_root_page) free_subtree(oHeader->_root_page);
      oHeader = header();
      oHeader->_root_page = 0;
      oHeader->_first_leaf = 0;
      oHeader->_count = 0;
      oHeader->_height = 0;
    }

    // ---- element access ------------------------------------------------------

    /// write-through/read proxy returned by operator[] (disk-backed values cannot expose a stable reference)
    class reference_proxy{
      friend class btree;
      btree* _tree;
      key_type _key;
      reference_proxy(btree* tree, const key_type& key) : _tree(tree), _key(key){}
    public:
      operator mapped_type() const {
        mapped_type v{};
        if (!_tree->find_value(_key, v)){
          _tree->insert(_key, mapped_type{});
          return mapped_type{};
        }
        return v;
      }
      reference_proxy& operator=(const mapped_type& value){ _tree->insert_or_assign(_key, value); return *this; }
      reference_proxy& operator=(const reference_proxy& rhs){ return operator=(static_cast<mapped_type>(rhs)); }
    };

    /// std::map-like element access; inserts a default value when @p key is absent
    reference_proxy operator[](const key_type& key){ return reference_proxy(this, key); }
  };
}

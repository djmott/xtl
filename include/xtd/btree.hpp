/** @file
STL-ish map using on disk b-tree
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


namespace xtd{
  /** B-Tree key-value container
   * @tparam _KeyT the key type
   * @tparam _ValueT the value type
   */
  template <typename _KeyT, typename _ValueT> class btree{

  public:
    using key_type = _KeyT;
    using value_type = _ValueT;

    /** insert a value in the container
     *
     * @param key unique key associated with the value
     * @param value value to insert
     * @return true of insert was successful, false otherwise
     */
    bool insert(const key_type& key, const value_type& value){
      return false;
    }


  };
}

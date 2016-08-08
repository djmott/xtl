/** @file
demonstrates event trace debugging
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <xtd/debug.hpp>



int Level4(int i){
  DBG("Level4 just returns a formal parameter : ", i);
  return i;
}


void SomeImportantStuff(){
  for (int i = 0; i < 5; i++){
    DUMP(Level4(i));
  }
}

void Level2(){
  DBG("Level2 is at depth 2");
  SomeImportantStuff();
}

int main(){
  Level2();
  return 0;
}

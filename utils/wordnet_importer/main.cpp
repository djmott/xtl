#include <xtd/xtd.hpp>
#include <xtd/nlp/english.hpp>
#include "wordnet.hpp"

int main() {
  wordnet::database oDB;
  xtd::nlp::english::pointer oEnglish(new xtd::nlp::english);
  oDB.import_to(oEnglish);
  return 0;
}
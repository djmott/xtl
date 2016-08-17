#include <xtd/xtd.hpp>
#include <xtd/nlp/english.hpp>

namespace wordnet{
  struct database{

  };
}


int main() {
  wordnet::database oDB;
  xtd::nlp::english::pointer oEnglish(new xtd::nlp::english);
  oDB.import_to(oEnglish);
  return 0;
}
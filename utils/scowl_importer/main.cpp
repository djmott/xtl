
#include <xtd/xtd.hpp>

#include <fstream>

#include <xtd/sqlite.hpp>
#include <xtd/filesystem.hpp>

bool import_pos(){
  auto oPath = xtd::filesystem::home_directory_path() + "/xtl.db";
  auto oDB = xtd::sqlite::database::open_database(oPath);
  auto oRS = oDB->execute_reader<int>("Select Count(*) From sqlite_master Where Type='table' And Name='scowl_pos';");
  if (!oRS->next() || !oRS->get<0>()){
    oDB->execute("Create Table scowl_pos (lemma string, pos int);");
  }
  auto oCmd = oDB->prepare<xtd::string, int>("Insert Into scowl_pos (lemma, pos) Values (?, ?);");

  std::ifstream in;
  in.open(XTD_ASSETS_DIR "/scowl/pos/part-of-speech.txt");
  if (in.bad()) return false;
  auto oTrans = oDB->begin_transaction();
  while (!in.eof()){
    xtd::string sLine;
    std::getline(in, sLine);
    if (0==sLine.size()) continue;
    auto sParts = sLine.split({'\t'});
    if (sParts.size() < 2) continue;
    xtd::string sLemma = sParts[0];
    for (char ch : sParts[1]){
      if ('|' == ch) continue;
      (*oCmd)(sLemma, ch);
    }
  }
  oTrans.commit();
  return true;
}


int main(){

  import_pos();
}

/** @file
c++ interface to moby databases
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>
#include <fstream>
#include <xtd/nlp/nlp.hpp>
#include <xtd/filesystem.hpp>

namespace xtd {
  namespace nlp {
    namespace moby {

      class database {

        class record {
        public:
          using vector = std::vector<record>;
          using map = std::map<std::string, record>;
          xtd::string _word;
          xtd::nlp::part_of_speech _pos;
          record(const std::string& sWord) : _word(sWord) {}
        };

        record::map records;

        database() {

          xtd::filesystem::path oPath(XTD_ASSETS_DIR "/mpos/mobyposi.i");
          std::ifstream in(oPath.string());
          in.exceptions(std::ios::badbit | std::ios::failbit);
          xtd::string sFile((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
          for (auto sBegin = sFile.begin(); sBegin < sFile.end(); ++sBegin) {
            for (; ('\r' == *sBegin || '\n' == *sBegin) && sBegin < sFile.end(); ++sBegin);
            auto sEnd = sBegin;
            for (; (char)0xd7 != *sEnd && sEnd < sFile.end(); ++sEnd);
            record r(std::string(sBegin, sEnd));
            records.insert(std::make_pair(r._word, r));
            for (++sEnd; '\r' != *sEnd && '\n' != *sEnd && sEnd < sFile.end(); ++sEnd) {
              part_of_speech iPOS = part_of_speech::unknown;
              switch (*sEnd) {
              case 'N':
                iPOS = part_of_speech::noun; break;
              case 'p':
                iPOS = part_of_speech::noun_plural; break;
              case 'h':
                iPOS = part_of_speech::noun_phrase; break;
              case 'V':
                iPOS = part_of_speech::verb_participle; break;
              case 't':
                iPOS = part_of_speech::verb_transitive; break;
              case 'i':
                iPOS = part_of_speech::verb_intransitive; break;
              case 'A':
                iPOS = part_of_speech::adj; break;
              case 'v':
                iPOS = part_of_speech::adv; break;
              case 'C':
                iPOS = part_of_speech::conjunction; break;
              case 'P':
                iPOS = part_of_speech::preposition; break;
              case '!':
                iPOS = part_of_speech::interjection; break;
              case 'r':
                iPOS = part_of_speech::pronoun; break;
              case 'D':
                iPOS = part_of_speech::definite_article; break;
              case 'I':
                iPOS = part_of_speech::indefinite_article; break;
              case 'o':
                iPOS = part_of_speech::nominative; break;
              }
              r._pos = static_cast<part_of_speech>(static_cast<uint64_t>(iPOS) | static_cast<uint64_t>(r._pos));
            }
            sBegin = sEnd;
          }
        }
      public:
        static database& get() {
          static database _database;
          return _database;
        }

        nlp::part_of_speech get_pos(const xtd::string& sWord) const {
          auto oItem = records.find(sWord);
          if (records.end() == oItem) return part_of_speech::unknown;
          return oItem->second._pos;
        }

      };
    }
  }
}

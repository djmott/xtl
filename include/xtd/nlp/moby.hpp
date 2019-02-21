/** @file
c++ interface to moby databases
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once
#if 0
#include <xtd/xtd.hpp>
#include <fstream>

#include <xtd/filesystem.hpp>
#include <xtd/nlp/document.hpp>

namespace xtd {
  namespace nlp {
    namespace moby {

      class database {

        class record {
        public:
          using vector = std::vector<record>;
          using map = std::map<std::string, record>;
          xtd::cstring _word;
          part_of_speech_t _pos;
          record(const std::string& sWord) : _word(sWord), _pos(part_of_speech_t::unknown_pos) {}
        };

        record::map records;

        database() {

          xtd::filesystem::path oPath(XTD_ASSETS_DIR "/mpos/mobyposi.i");
          std::ifstream in(oPath.string());
          in.exceptions(std::ios::badbit | std::ios::failbit);
          xtd::cstring sFile((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
          for (auto sBegin = sFile.begin(); sBegin < sFile.end(); ++sBegin) {
            for (; ('\r' == *sBegin || '\n' == *sBegin) && sBegin < sFile.end(); ++sBegin);
            auto sEnd = sBegin;
            for (; (char)0xd7 != *sEnd && sEnd < sFile.end(); ++sEnd);
            record r(std::string(sBegin, sEnd));
            for (++sEnd; '\r' != *sEnd && '\n' != *sEnd && sEnd < sFile.end(); ++sEnd) {
              part_of_speech_t iPOS = part_of_speech_t::unknown_pos;
              switch (*sEnd) {
              case 'N':
                iPOS = part_of_speech_t::noun; break;
              case 'p':
                iPOS = part_of_speech_t::plural; break;
              case 'h':
                iPOS = part_of_speech_t::noun_phrase; break;
              case 'V':
                iPOS = part_of_speech_t::verb_participle; break;
              case 't':
                iPOS = part_of_speech_t::verb_transitive; break;
              case 'i':
                iPOS = part_of_speech_t::verb_intransitive; break;
              case 'A':
                iPOS = part_of_speech_t::adjective; break;
              case 'v':
                iPOS = part_of_speech_t::adverb; break;
              case 'C':
                iPOS = part_of_speech_t::conjunction; break;
              case 'P':
                iPOS = part_of_speech_t::preposition; break;
              case '!':
                iPOS = part_of_speech_t::interjection; break;
              case 'r':
                iPOS = part_of_speech_t::pronoun; break;
              case 'D':
                iPOS = part_of_speech_t::definite_article; break;
              case 'I':
                iPOS = part_of_speech_t::indefinite_article; break;
              case 'o':
                iPOS = part_of_speech_t::nominative; break;
              }
              r._pos = static_cast<part_of_speech_t>(static_cast<uint64_t>(iPOS) | static_cast<uint64_t>(r._pos));
            }
            records.insert(std::make_pair(r._word, r));
            sBegin = sEnd;
          }
        }
      public:
        static database& get() {
          static database _database;
          return _database;
        }

        nlp::part_of_speech_t get_pos(const xtd::cstring& sWord) const {
          auto oItem = records.find(sWord);
          if (records.end() == oItem) return part_of_speech_t::unknown_pos;
          return oItem->second._pos;
        }

      };
    }
  }
}
#endif
#pragma once


namespace xtd{
  namespace nlp{

    template <typename _SuperT>
    struct simple_tokenizer : itokenizer, _SuperT{
    
      template <typename _Ty>
      itokenizer::document operator()(_Ty&& src) {
        if (!_SuperT::template has<typename _SuperT::locale>()) throw std::runtime_error("locale property not set");
        const auto& oLocale = _SuperT::template value<typename _SuperT::locale>();
        itokenizer::document oRet(std::move(src));
        auto& sText = oRet.template value<raw_document::text>();
        auto oTokens = sText.split(std::function<bool(char)>([&oLocale](char ch)->bool{
          return !(std::isalnum(ch, oLocale));
        }));



        oRet.template value<itokenizer::document::tokens>() = oTokens;
        return oRet;
      }
    };

  }
}
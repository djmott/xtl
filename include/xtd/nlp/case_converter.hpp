#pragma once

namespace xtd{
  namespace nlp{
    template <typename _SuperT, typename _VisitorT> struct character_visitor : _SuperT{
      template <typename _ParamT> _ParamT operator()(_ParamT&& oParam) const {
        if (!_SuperT::template has<typename _SuperT::locale>()) throw std::runtime_error("locale property not set");
        const auto& oLocale = _SuperT::template value<typename _SuperT::locale>();
        _VisitorT oVisitor(oLocale);
        auto & sText = oParam.template value<typename _ParamT::text>();
        for (auto & oChar : sText){
          oVisitor(oChar);
        }
        return oParam;
      }
    };


    struct lcase_visitor{
      const std::locale& _locale;
      lcase_visitor(const std::locale& oLocale) : _locale(oLocale){}
      void operator()(char& ch) const{
        if (!std::islower(ch, _locale)) ch = std::tolower(ch, _locale);
      }
    };


    template <typename _SuperT> using lcase_converter = character_visitor<_SuperT, lcase_visitor>;

  }
}
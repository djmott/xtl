#pragma once

namespace xtd{
  namespace nlp{



    //unstructured, unformatted and unanalyzed text
    struct raw_document : dynamic_object{
      using text = PROPERTY(xtd::string);

      template <typename..._ParamTs> raw_document(_ParamTs&&...oParams) : dynamic_object(std::forward<_ParamTs>(oParams)...){}

    };



    struct document_factory : dynamic_object{

      using locale = PROPERTY(std::locale);

      raw_document operator()(const xtd::string& src) const {
        raw_document oRet;
        oRet.value<raw_document::text>() = src;
        return oRet;
      }
    };

    struct itokenizer{

      struct document : raw_document{
        template <typename..._ParamTs> document(_ParamTs&&...oParams) : raw_document(std::forward<_ParamTs>(oParams)...){}

        using tokens = PROPERTY(std::vector<xtd::string>);

      };

    };

    struct istemmer{

      struct document : raw_document{
        template <typename..._ParamTs> document(_ParamTs&&...oParams) : raw_document(std::forward<_ParamTs>(oParams)...){}
      };

    };

    
    struct isentence_boundary_detector{

      struct document : raw_document{
        template <typename..._ParamTs> document(_ParamTs&&...oParams) : raw_document(std::forward<_ParamTs>(oParams)...){}
      };

    };


    template <typename ...> struct document_process;

    template <> struct document_process<>{
      template <typename _ParamT>
      _ParamT operator()(_ParamT&& src) const { return src; }
    };



    template <typename _HeadT, typename ... _TailT> struct document_process<_HeadT, _TailT...> : document_process<_TailT...>{
      using _super_t = document_process<_TailT...>;

      //template <typename _ParamT> using _return_t = decltype(std::declval<_HeadT>()(_ParamT()));
      template <typename _ParamT> using _return_t = typename  std::result_of<_HeadT(_ParamT&&)>::type;

      template <typename _ParamT>
      _return_t<_ParamT> operator()(_ParamT&& src) const {
        _HeadT oHead;
        const auto& oSuper = static_cast<const _super_t&>(*this);
        return oHead(std::move(oSuper(std::move(src))));
      }

    };


  }
}
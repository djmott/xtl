#pragma once


namespace xtd{
  namespace nlp{
    template <typename ...> struct text_processor;

    template <> struct text_processor<>{};

    template <typename _HeadT, typename ... _TailT> struct text_processor<_HeadT, _TailT...>{};
  }
}

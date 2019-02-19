#include <xtd/xtd.hpp>


extern "C" {

#if (XTD_OS_WINDOWS & XTD_OS)
  __declspec(dllexport)
#endif
  int dummy_export(int x, int y) { return x + y; }
}
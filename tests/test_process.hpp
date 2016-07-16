#pragma once

TEST(test_process, initialization){
  ASSERT_NO_THROW(auto & oProc = xtd::process::this_process());
}

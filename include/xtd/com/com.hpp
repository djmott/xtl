/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <atomic>

namespace xtd {
	namespace com {

		static std::atomic<int>& GlobalDLLLock() {
			static std::atomic<int> _DllLock;
			return _DllLock;
		}

	}
}

#include "com_initializer.hpp"
#include "com_pointer.hpp"
#include "com_exception.hpp"
#include "com_data_object.hpp"
#include "com_object.hpp"
#include "com_class_factory.hpp"
#include "com_dll.hpp"

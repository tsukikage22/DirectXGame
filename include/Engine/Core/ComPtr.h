#pragma once

#include <wrl/client.h>

namespace engine
{
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
}

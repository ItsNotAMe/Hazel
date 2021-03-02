#pragma once

#include <string>
#include <optional>

namespace Hazel {

	class FileDialogs
	{
	public:
		// Returns empty strings if canceled
		static std::optional<std::string> OpenFile(const char* filter);
		static std::optional<std::string> SaveFile(const char* filter);
	};

}
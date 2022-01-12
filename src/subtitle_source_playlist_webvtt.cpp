#include "lib_modules/core/module.hpp"
#include <string>
#include <vector>

std::string getContentWebvtt(const std::vector<char> &input) {
	return std::string(input.begin(), input.end());
}

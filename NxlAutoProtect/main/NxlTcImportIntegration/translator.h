#include <stdafx.h>
#pragma once

extern bool translator_unprotect(const std::string &file, std::string &outputFile);
extern bool translator_protect(const std::string &file, const std::vector<std::pair<std::string, std::string>> &tags, std::string &outputDir);

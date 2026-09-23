// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 Vivek Shukla
#pragma once
#include <string>
#include <vector>
#include <stdexcept>
namespace wql {
struct Issue { size_t offset; std::string message; };
struct Error : std::runtime_error {
    size_t offset;
    Error(size_t p, const std::string& m) : std::runtime_error(m), offset(p) {}
};
std::string format(const std::string& source, const std::string& eol = "\n");
std::string minify(const std::string& source, const std::string& eol = "\n");
std::vector<Issue> validate(const std::string& source);
}

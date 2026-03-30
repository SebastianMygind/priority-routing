#pragma once
#include "node_attributes.h"

#include <cstdint>
#include <expected>
#include <string>
#include <unordered_map>

using file_format_t = std::unordered_map<uint64_t, NodeAttributes>;

std::expected<void, std::string>  WriteToCache(
    std::string filePath,
    file_format_t& attrMap,
    size_t hash,
    size_t attrCount) ;

std::expected<file_format_t, std::string> ReadFromCache(std::string filePath);

bool cacheIsValid(const std::string& filepath, size_t hash);
#include "caching.h"
#include <filesystem>
#include <expected>
#include <fstream>
#include <ranges>
#include <string>
#include <spdlog/spdlog.h>

// HASH, NODE_COUNT, ATTR_COUNT, ...DATA

std::expected<void, std::string>  WriteToCache(
    std::string filePath,
    file_format_t& attrMap,
    size_t hash,
    size_t attrCount) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return std::unexpected("Could not open cache file");
    }
    file.write(reinterpret_cast<const char*>(&hash), sizeof(hash)); //NOLINT

    const auto nodeCount = attrMap.size();
    file.write(reinterpret_cast<const char*>(&nodeCount), sizeof(nodeCount)); //NOLINT

    file.write(reinterpret_cast<const char*>(&attrCount), sizeof(attrCount)); //NOLINT

    // file.setf(std::ios::fixed);
    // file.precision(10);

    for (const auto& [nodeId, attributes] : attrMap) {
        file.write(reinterpret_cast<const char*>(&nodeId), sizeof(nodeId));

        for (const double val : attributes.attributes | std::views::values) {
            file.write(reinterpret_cast<const char*>(&val), sizeof(val));
        }
        file << '\n';
    }
    if (!file.good()) {
        std::unexpected("Error while writing to cache file");
    }

    return {};
}

std::expected<file_format_t, std::string> ReadFromCache(std::string filePath) {
    file_format_t attrMap;
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return std::unexpected("Could not open cache file");
    }


    return attrMap;
}

bool cacheIsValid(const std::string& filepath, const size_t hash) {
    std::ifstream file(filepath, std::ios::binary);

    if (!file.is_open()) {
        spdlog::warn(
            "Could not open cache file for reading at path: {}", filepath);
        return false;
    }

    size_t fileHash = 0;

    if (file.peek() == std::ifstream::traits_type::eof()) {
        spdlog::warn("Cache file exists, but is empty which is an invalid form.");
        return false;
    }

    file.read(reinterpret_cast<char*>(&fileHash), sizeof(fileHash)); //NOLINT

    return fileHash == hash;
}
#include "node_attributes.h"
#include <ranges>

std::string NodeAttributes::ListAttributes() const {
    return attributes
    | std::views::keys
    | std::views::join
    | std::ranges::to<std::string>();
}

// This hashing is not used to compare actual objects, but only the keys.
size_t NodeAttributes::HashAttributes() const {
    return std::hash<std::string>{}(ListAttributes());
}
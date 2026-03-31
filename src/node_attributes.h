#pragma once

#include <map>
#include <string>

class NodeAttributes {
public:
    std::map<std::string, double> attributes;
    NodeAttributes() = default;
private:
    [[nodiscard]] std::string ListAttributes() const;
    [[nodiscard]] size_t HashAttributes() const;
};


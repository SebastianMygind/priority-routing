#include <unordered_set>
#include <string>

static const std::unordered_set<std::string> kDrivableHighways = {
    "motorway", "trunk", "primary", "secondary", "tertiary",
    "unclassified", "residential", "service",
    "motorway_link", "trunk_link", "primary_link",
    "secondary_link", "tertiary_link", "living_street"
};

static const std::unordered_set<std::string> kMotorways = {
    "motorway", "trunk", "motorway_link", "trunk_link"
};

static const std::unordered_set<std::string> kPrimary = {
    "primary", "primary_link"
};

static const std::unordered_set<std::string> kSecondary = {
    "secondary", "secondary_link"
};

static const std::unordered_set<std::string> kTertiary = {
    "tertiary", "tertiary_link"
};

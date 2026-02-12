#pragma once
#include <string>
#include <initializer_list>
#include "nlohmann/json.hpp"

class ReadConfig {
public:
    // Constructor that loads the JSON file
    explicit ReadConfig(const std::string& filename);

    // Templated function to get a configuration value given nested keys.
    // Example usage: getValue<double>({"electronPick", "minPt"})
    template <typename T>
    T getValue(std::initializer_list<std::string> keys) const {
        nlohmann::json current = config_;
        std::string path;
        for (const auto& key : keys) {
            if (!current.contains(key)) {
                if (path.empty()) {
                    path = key;
                } else {
                    path += "/" + key;
                }
                throw std::runtime_error("Missing required JSON element: " + filename_ + " -> " + path);
            }
            if (path.empty())
                path = key;
            else
                path += "/" + key;
            current = current[key];
        }
        return current.get<T>();
    }

private:
    nlohmann::json config_;
    std::string filename_;
};


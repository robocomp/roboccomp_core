#ifndef CONFIG_LOADER_TPP
#define CONFIG_LOADER_TPP

#include "ConfigLoader.h"

template <typename T>
T ConfigLoader::get(const std::string& key) const {
    auto it = configData.find(key);
    if (it == configData.end()) {
        throw std::runtime_error("Key not found: " + key);
    }

    try {
        return std::get<T>(it->second);
    } catch (const std::bad_variant_access& e) {
        throw std::runtime_error("Type mismatch for key: " + key);
    }
}

    template <typename T>
    std::string ConfigLoader::vectorToString(const std::vector<T>& vec) {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            oss << vec[i];
            if (i < vec.size() - 1) {
                oss << ", ";
            }
        }
        oss << "]";
        return oss.str();
    }


#endif // CONFIG_LOADER_TPP


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
        throw std::runtime_error("Key \"" + key + "\" type mismatch.\n"
        "Type identified: " + ConfigLoader::getTypeName(it->second) + "\n"
        "Requested type: " +
            ([] {
                // Mapeo de nombres de typeid a nombres legibles
                static const std::unordered_map<std::string, std::string> typeNames = {
                    {"i", "int"},
                    {"d", "double"},
                    {"b", "bool"},
                    {"NSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE", "std::string"},
                    {"St3vectorIiE", "std::vector<int>"},
                    {"St3vectorIdE", "std::vector<double>"},
                    {"St3vectorIbE", "std::vector<bool>"},
                    {"St3vectorINSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE", "std::vector<std::string>"}
                };

                auto it = typeNames.find(typeid(T).name());
                return (it != typeNames.end()) ? it->second : std::string(typeid(T).name());
            })() +
            "\n" + TypeExample);
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


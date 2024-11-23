#include "ConfigLoader.h"


bool ConfigLoader::isInteger(const std::string& str) const {
    char* p;
    strtol(str.c_str(), &p, 10);
    return (*p == 0);
}

bool ConfigLoader::isDouble(const std::string& str) const {
    char* p;
    strtod(str.c_str(), &p);
    return (*p == 0);
}

bool ConfigLoader::isBoolean(const std::string& str, bool& result) const {
    if (str == "true" || str == "TRUE" || str == "True") {
        result = true;
        return true;
    }
    if (str == "false" || str == "FALSE" || str == "False") {
        result = false;
        return true;
    }
    return false;
}

void ConfigLoader::processLine(const std::string& line) {
    size_t pos = line.find('=');
    if (pos == std::string::npos) return;

    std::string key = line.substr(0, pos);
    std::string value = line.substr(pos + 1);

    // Trim whitespace
    key.erase(0, key.find_first_not_of(" \t"));
    key.erase(key.find_last_not_of(" \t") + 1);
    value.erase(0, value.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t") + 1);

    // Determine the value type
    bool boolResult;
    if (isInteger(value)) {
        configData[key] = stoi(value);
    } else if (isDouble(value)) {
        configData[key] = stod(value);
    } else if (isBoolean(value, boolResult)) {
        configData[key] = boolResult;
    } else {
        configData[key] = value; // Treat as string
    }
}

void ConfigLoader::loadTxt(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open text file: " + filename);
    }

    std::string line;
    while (getline(file, line)) {
        // Skip empty lines or comments
        if (line.empty() || line[0] == '#') continue;
        processLine(line);
    }
}

void ConfigLoader::loadToml(const std::string& filename) {
    auto config = toml::parse_file(filename);
    processTomlTable(config);
}

void ConfigLoader::processTomlTable(const toml::table& table, const std::string& prefix) {
    for (const auto& [key, value] : table) {
        std::string fullKey = prefix.empty() ? std::string(key.str()) : prefix + "." + std::string(key.str());

        if (value.is_table()) {
            processTomlTable(*value.as_table(), fullKey);
        } else if (value.is_integer()) {
            configData[fullKey] = static_cast<int>(value.as_integer()->get());
        } else if (value.is_floating_point()) {
            configData[fullKey] = value.as_floating_point()->get();
        } else if (value.is_string()) {
            configData[fullKey] = value.as_string()->get();
        } else if (value.is_boolean()) {
            configData[fullKey] = value.as_boolean()->get();
        }
    }
}

void ConfigLoader::load(const std::string& filename) {
        if (filename.ends_with(".toml")) {
            loadToml(filename);
        } else {
            loadTxt(filename);
        }
    }


void ConfigLoader::printConfig() const {
    for (const auto& [key, value] : configData) {
        std::cout << key << " = ";
        std::visit([](auto&& v) { std::cout << v; }, value);
        std::cout << std::endl;
    }
}

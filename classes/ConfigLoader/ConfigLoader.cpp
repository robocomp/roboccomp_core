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

bool ConfigLoader::isString(const std::string& str) const {
    // Verificar si el valor es un string y si está entre comillas
    if (str.front() == '"' || str.back() == '"') {
        return true;
    }
    return false;
}

void ConfigLoader::processLine(const std::string& line) {
    // Ignorar comentarios y espacios en blanco
    std::string lineWithoutComment = line;
    size_t commentPos = line.find('#');
    if (commentPos != std::string::npos) {
        lineWithoutComment = line.substr(0, commentPos);
    }

    // Eliminar espacios en blanco al principio y final
    lineWithoutComment.erase(0, lineWithoutComment.find_first_not_of(" \t"));
    lineWithoutComment.erase(lineWithoutComment.find_last_not_of(" \t") + 1);

    // Si la línea es vacía después de eliminar comentarios y espacios, ignorarla
    if (lineWithoutComment.empty()) return;

    size_t pos = lineWithoutComment.find('=');
    if (pos == std::string::npos) return; // No tiene un '='

    std::string key = lineWithoutComment.substr(0, pos);
    std::string value = lineWithoutComment.substr(pos + 1);

    // Trim whitespace from key and value
    key.erase(0, key.find_first_not_of(" \t"));
    key.erase(key.find_last_not_of(" \t") + 1);
    value.erase(0, value.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t") + 1);

    // Determinar el tipo de valor (entero, doble, booleano, string)
    bool boolResult;
    if (isInteger(value)) {
        configData[key] = stoi(value);
    } else if (isDouble(value)) {
        configData[key] = stod(value);
    } else if (isBoolean(value, boolResult)) {
        configData[key] = boolResult;
    } else if (isString(value)) {
        configData[key] = value.substr(1, value.size() - 2);
    }
    else{
        std::cerr << "Key " << key << " with value:"<< value << "dont found type";
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

ConfigTypes ConfigLoader::processSimpleValue(const toml::node& value) {
    if (value.is_integer()) {
        return static_cast<int>(value.as_integer()->get());
    } else if (value.is_floating_point()) {
        return value.as_floating_point()->get();
    } else if (value.is_string()) {
        return value.as_string()->get();
    } else if (value.is_boolean()) {
        return value.as_boolean()->get();
    } else {
        throw std::runtime_error("Unsupported TOML value encountered.");
    }
}

ConfigTypes ConfigLoader::processArray(const toml::array& array) {
    if (array.empty()) {
        throw std::runtime_error("Empty arrays are not supported.");
    }

    if (array.front().is_integer()) {
        std::vector<int> result;
        for (const auto& element : array) {
            if (element.is_integer()) {
                result.push_back(element.as_integer()->get());
            } else {
                throw std::runtime_error("Mixed-type arrays are not supported.");
            }
        }
        return result;
    } else if (array.front().is_floating_point()) {
        std::vector<double> result;
        for (const auto& element : array) {
            if (element.is_floating_point()) {
                result.push_back(element.as_floating_point()->get());
            } else {
                throw std::runtime_error("Mixed-type arrays are not supported.");
            }
        }
        return result;
    } else if (array.front().is_string()) {
        std::vector<std::string> result;
        for (const auto& element : array) {
            if (element.is_string()) {
                result.push_back(element.as_string()->get());
            } else {
                throw std::runtime_error("Mixed-type arrays are not supported.");
            }
        }
        return result;
    } else if (array.front().is_boolean()) {
        std::vector<bool> result;
        for (const auto& element : array) {
            if (element.is_boolean()) {
                result.push_back(element.as_boolean()->get());
            } else {
                throw std::runtime_error("Mixed-type arrays are not supported.");
            }
        }
        return result;
    } else {
        throw std::runtime_error("Unsupported array type encountered.");
    }
}

void ConfigLoader::processTomlTable(const toml::table& table, const std::string& prefix) {
    for (const auto& [key, value] : table) {
        std::string fullKey = prefix.empty() ? std::string(key.str()) : prefix + "." + std::string(key.str());

        if (value.is_table()) {
            // Procesar tablas de forma recursiva
            processTomlTable(*value.as_table(), fullKey);
        } else if (value.is_array()) {
            // Procesar arreglos y almacenarlos como vector<T>
            configData[fullKey] = processArray(*value.as_array());
        } else {
            // Procesar valores simples
            configData[fullKey] = processSimpleValue(value);
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
            std::visit([&](auto&& v) { 
                if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::vector<int>>) {
                    std::cout << vectorToString(v);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::vector<double>>) {
                    std::cout << vectorToString(v);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::vector<std::string>>) {
                    std::cout << vectorToString(v);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::vector<bool>>) {
                    std::cout << vectorToString(v);
                } else {
                    std::cout << v;
                }
            }, value);
            std::cout << std::endl;
        }
    }

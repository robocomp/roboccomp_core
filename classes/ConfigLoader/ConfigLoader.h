/****************************************************************************
* File name: ConfigLoader.h
* Author: Alejandro Torrejon Harto
* Date: 21/11/2024
* Description:  Header file for the ConfigLoader class, designed to load,
*              parse, and manage configuration data from TOML and text files.
*              Supports type-safe access to configuration values and 
*              provides utilities for loading and printing configuration data.
****************************************************************************/

#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <string>
#include <unordered_map>
#include <variant>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <toml++/toml.h>


// Alias for configuration types
using ConfigTypes = std::variant<int, double, std::string, bool>;

/**
 * @class ConfigLoader
 * @brief A class to load and manage configuration data from TOML and text files.
 */
class ConfigLoader {
private:
    std::unordered_map<std::string, ConfigTypes> configData;

    // Helper functions for type detection
    bool isInteger(const std::string& str) const;
    bool isDouble(const std::string& str) const;
    bool isBoolean(const std::string& str, bool& result) const;

    // Helper to process a line from a text configuration file
    void processLine(const std::string& line);

    // Loaders for specific file types
    void loadTxt(const std::string& filename);
    void loadToml(const std::string& filename);

    // Recursive processing of TOML tables
    void processTomlTable(const toml::table& table, const std::string& prefix = "");

public:
    /**
     * @brief Retrieves the value for a given key and casts it to the specified type.
     * @tparam T The expected type of the value.
     * @param key The configuration key.
     * @return The value cast to the specified type.
     * @throws std::runtime_error If the key is not found or the type does not match.
     */
    template <typename T>
    T get(const std::string& key) const;

    /**
     * @brief Loads a configuration file (TOML or text format).
     * @param filename The path to the configuration file.
     */
    void load(const std::string& filename);

    /**
     * @brief Prints all loaded configuration data to the console.
     */
    void printConfig() const;
};
#include "ConfigLoader.tpp"
#endif // CONFIG_LOADER_H

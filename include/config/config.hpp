#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#pragma once
class Config {
private:
  std::string config_file = "/data/adb/bypass_charge.ini";
  std::map<std::string, std::map<std::string, std::string>>
  paserINI(const std::string &filename) {
    std::ifstream file(filename);
    std::map<std::string, std::map<std::string, std::string>> iniData;
    std::string line, section;
    if (!file.is_open()) {
      std::cerr << "Unable to open file: " << filename << std::endl;
      return iniData;
    }
    while (std::getline(file, line)) {
      // Remove comments
      line = line.substr(0, line.find(';'));
      line = line.substr(0, line.find('#'));
      // Trim whitespace
      line.erase(0, line.find_first_not_of(" \t\n\r"));
      line.erase(line.find_last_not_of(" \t\n\r") + 1);
      if (line.empty())
        continue;
      // Section header
      if (line.front() == '[' && line.back() == ']') {
        section = line.substr(1, line.size() - 2);
      } else {
        // Key-value pair
        std::istringstream is_line(line);
        std::string key, value;
        if (std::getline(is_line, key, '=') && std::getline(is_line, value)) {
          // Trim whitespace
          key.erase(0, key.find_first_not_of(" \t\n\r"));
          key.erase(key.find_last_not_of(" \t\n\r") + 1);
          value.erase(0, value.find_first_not_of(" \t\n\r"));
          value.erase(value.find_last_not_of(" \t\n\r") + 1);
          iniData[section][key] = value;
        }
      }
    }
    file.close();
    return iniData;
  }

public:
  std::map<std::string, std::map<std::string, std::string>> get_config() {
    return paserINI(config_file);
  }
};
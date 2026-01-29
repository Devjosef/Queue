#ifndef ENV_LOADER_HPP
#define ENV_LOADER_HPP

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <fstream>
#include <iostream>

class EnvLoader {
public:
    static std::string get(const std::string& key, const std::string& default_value = "") {
        if (env_map.find(key) != env_map.end()) {
            return env_map[key];
        }
        char* value = std::getenv(key.c_str());
        return value? std::string(value) : default_value;
    }

private:
    static std::unordered_map<std::string, std::string> env_map;
    static bool loaded;

    static void load_dotenv(){
        if (loaded) return;
        
        std::ifstream file(".env");
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            if(line.empty() || line[0] == '#') continue;

            size_t eq_pos = line.find('=');
            if (eq_pos == std::string::npos) {
                std::string key = line.substr(0, eq_pos);
                std::string value = line.substr(eq_pos + 1);
                env_map[key] = value;
            }
        }
        loaded = true;
    }
    
};

std::unordered_map<std::string, std::string> EnvLoader::env_map;
bool EnvLoader::loaded = false;

#endif
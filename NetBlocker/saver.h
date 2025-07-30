#pragma once
#include <fstream>
#include <string>
#include <cstring>
#include "netblocker.h"
#include "json.h"
#include "UI/vars.h"
#include "parser.h"
#include "soundSystem.h"

namespace saver {
    inline const std::string configFile = "config.json";

    inline void save() {
        json j;
        // Save sound settings
        j["soundPackIndex"] = soundSystem::SoundManager::GetSelectedPackIndex();

        // Save netBlock variables
        j["processName"] = std::string(netBlock::processName);

        // Save varsUI variables
        j["uiUpdateTime"] = varsUI::uiUpdateTime;
        j["toggleKeybind"] = varsUI::toggleKeybind;

        // Save accentColor components
        json& accentColorJson = j["accentColor"];
        accentColorJson["x"] = varsUI::accentColor.x;
        accentColorJson["y"] = varsUI::accentColor.y;
        accentColorJson["z"] = varsUI::accentColor.z;
        accentColorJson["w"] = varsUI::accentColor.w;

        std::ofstream file(configFile);
        if (file.is_open()) {
            file << j.dump(4);
            file.close();
        }
    }

    inline void load() {
        std::ifstream file(configFile);
        if (file.is_open()) {
            json j;
            file >> j;
            file.close();

            // Initialize sound system first
            soundSystem::SoundManager::Initialize();

            // Load sound settings
            if (j.contains("soundPackIndex")) {
                int index = j["soundPackIndex"].get<int>();
                auto& soundPacks = soundSystem::SoundManager::GetSoundPacks();
                if (index >= 0 && index < soundPacks.size()) {
                    soundSystem::SoundManager::SetSelectedPackIndex(index);
                }
            }

            // Load netBlock variables
            if (j.contains("processName")) {
                std::string p = j["processName"].get<std::string>();
                strncpy_s(netBlock::processName, p.c_str(), sizeof(netBlock::processName) - 1);
                netBlock::processName[sizeof(netBlock::processName) - 1] = '\0';
            }

            // Load varsUI variables
            if (j.contains("uiUpdateTime"))
                varsUI::uiUpdateTime = j["uiUpdateTime"].get<int>();

            if (j.contains("toggleKeybind"))
                varsUI::toggleKeybind = j["toggleKeybind"].get<int>();

            std::wstring url = L"https://raw.githubusercontent.com/isBink/NetBlocker/main/NetBlocker/UI/vars.h";
            float version = getVersionFromURL(url);
            if (version != varsUI::version)
            {
                varsUI::menuName += " (OUTDATED)";
                MessageBoxA(nullptr, "A new version of NetBlocker is available. Consider updating for the best experience.", "NetBlocker", MB_OK | MB_ICONINFORMATION);
            }
            if (j.contains("accentColor")) {
                auto& c = j["accentColor"];
                varsUI::accentColor.x = static_cast<float>(c["x"].get<double>());
                varsUI::accentColor.y = static_cast<float>(c["y"].get<double>());
                varsUI::accentColor.z = static_cast<float>(c["z"].get<double>());
                varsUI::accentColor.w = static_cast<float>(c["w"].get<double>());
            }
        }
    }
}
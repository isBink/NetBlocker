#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <windows.h>
#include <algorithm>

namespace fs = std::filesystem;

namespace soundSystem {

    struct SoundPack {
        std::string name;           // Name of the sound pack (folder name)
        std::string directory;      // Full path to the sound pack
        std::string onSoundPath;    // Path to on.wav (empty if doesn't exist)
        std::string offSoundPath;   // Path to off.wav (empty if doesn't exist)
    };

    class SoundManager {
    private:
        static inline std::string executableDir;
        static inline std::string soundsBaseDir;
        static inline std::vector<SoundPack> soundPacks;
        static inline int selectedPackIndex = 0;
        static inline bool initialized = false;

    public:
        static void Initialize() {
            if (initialized) return;

            executableDir = GetExecutableFolder();
            soundsBaseDir = executableDir + "\\sounds";

            LoadSoundPacks();
            initialized = true;
        }

        static void ReloadSounds() {
            LoadSoundPacks();
        }

        static const std::vector<SoundPack>& GetSoundPacks() { return soundPacks; }
        static int GetSelectedPackIndex() { return selectedPackIndex; }
        static void SetSelectedPackIndex(int index) {
            if (index >= 0 && index < soundPacks.size()) {
                selectedPackIndex = index;
            }
        }

        static void PlaySound(bool blocked) {
            if (soundPacks.empty()) return;

            const auto& pack = soundPacks[selectedPackIndex];
            const std::string& soundPath = blocked ? pack.onSoundPath : pack.offSoundPath;

            // If requested sound doesn't exist, try playing the other one
            const std::string& fallbackSoundPath = blocked ? pack.offSoundPath : pack.onSoundPath;

            ::PlaySound(NULL, NULL, 0); // Stop any currently playing sound

            if (!soundPath.empty()) {
                ::PlaySound(soundPath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
            }
            else if (!fallbackSoundPath.empty()) {
                ::PlaySound(fallbackSoundPath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
            }
        }

        static std::string GetExecutableFolder() {
            char buffer[MAX_PATH];
            GetModuleFileNameA(NULL, buffer, MAX_PATH);
            return fs::path(buffer).parent_path().string();
        }

    private:
        static void LoadSoundPacks() {
            soundPacks.clear();

            if (!fs::exists(soundsBaseDir)) {
                fs::create_directories(soundsBaseDir);
                return;
            }

            // Scan for sound pack directories
            for (const auto& entry : fs::directory_iterator(soundsBaseDir)) {
                if (!entry.is_directory()) continue;

                SoundPack pack;
                pack.name = entry.path().filename().string();
                pack.directory = entry.path().string();

                std::string onPath = pack.directory + "\\on.wav";
                if (fs::exists(onPath)) {
                    pack.onSoundPath = onPath;
                }

                std::string offPath = pack.directory + "\\off.wav";
                if (fs::exists(offPath)) {
                    pack.offSoundPath = offPath;
                }

                // Only add pack if it has at least one sound
                if (!pack.onSoundPath.empty() || !pack.offSoundPath.empty()) {
                    soundPacks.push_back(pack);
                }
            }

            if (selectedPackIndex >= soundPacks.size()) {
                selectedPackIndex = 0;
            }
        }
    };
}
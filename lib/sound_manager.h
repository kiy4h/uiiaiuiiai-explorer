#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <map>
#include <string>
#include <unordered_map>

class SoundManager {
public:
    SoundManager();
    ~SoundManager();

    bool loadBGM(const std::string &filePath, const std::string &bgmName); // Load BGM with a name
    bool changeBGM(const std::string &bgmName);                            // Change BGM by name

    void playBGM();
    void stopBGM();
    void stopAllSoundEffects();
    void playFootsteps();
    void stopFootsteps();

    bool loadSoundEffect(const std::string &effectName, const std::string &effectPath);
    void playSoundEffect(const std::string &effectName);

    void setBGMVolume(int volume) {
        Mix_VolumeMusic(volume);
    }

    void setEffectVolume(int volume) {
        for (auto &pair : soundEffects) {
            Mix_VolumeChunk(pair.second, volume);
        }
    }

private:
    Mix_Music *currentBGM;                             // Pointer to the currently playing BGM
    std::unordered_map<std::string, Mix_Music *> bgms; // Map of BGMs
    std::unordered_map<std::string, Mix_Chunk *> soundEffects;

    int footstepChannel = -1; // Store the channel used for footsteps
};

#endif // SOUND_MANAGER_H
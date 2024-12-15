#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <map>
#include <string>

class SoundManager {
public:
    // Constructor and Destructor
    SoundManager();
    ~SoundManager();

    // Load audio files
    bool loadBGM(const std::string &bgmPath);
    bool loadSoundEffect(const std::string &effectName, const std::string &effectPath);

    // Control audio
    void playBGM();
    void stopBGM();
    void playSoundEffect(const std::string &effectName);

private:
    Mix_Music *bgm;                                  // Background music
    std::map<std::string, Mix_Chunk *> soundEffects; // Map of sound effects
};

#endif

#include "lib/sound_manager.h"
#include <iostream>

SoundManager::SoundManager() : currentBGM(nullptr) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Failed to initialize SDL_Mixer: " << Mix_GetError() << std::endl;
    }
}

SoundManager::~SoundManager() {
    if (currentBGM) {
        Mix_FreeMusic(currentBGM);
    }

    for (auto &pair : soundEffects) {
        Mix_FreeChunk(pair.second);
    }

    Mix_CloseAudio();
    SDL_Quit();
}

bool SoundManager::loadBGM(const std::string &filePath, const std::string &bgmName) {
    Mix_Music *music = Mix_LoadMUS(filePath.c_str());
    if (!music) {
        std::cerr << "Failed to load BGM (" << bgmName << "): " << Mix_GetError() << std::endl;
        return false;
    }
    bgms[bgmName] = music;
    return true;
}

bool SoundManager::changeBGM(const std::string &bgmName) {
    auto it = bgms.find(bgmName);
    if (it == bgms.end()) {
        std::cerr << "BGM not found: " << bgmName << std::endl;
        return false;
    }

    stopBGM();
    currentBGM = it->second;
    playBGM();
    return true;
}

void SoundManager::playBGM() {
    if (currentBGM) {
        Mix_PlayMusic(currentBGM, -1); // Loop indefinitely
    }
}

void SoundManager::stopBGM() {
    Mix_HaltMusic();
}

void SoundManager::stopAllSoundEffects() {
    Mix_HaltChannel(-1); // Stops all active sound effect channels
}

bool SoundManager::loadSoundEffect(const std::string &effectName, const std::string &effectPath) {
    Mix_Chunk *effect = Mix_LoadWAV(effectPath.c_str());
    if (!effect) {
        std::cerr << "Failed to load sound effect (" << effectName << "): " << Mix_GetError() << std::endl;
        return false;
    }
    soundEffects[effectName] = effect;
    return true;
}

void SoundManager::playSoundEffect(const std::string &effectName) {
    stopAllSoundEffects();
    auto it = soundEffects.find(effectName);
    if (it != soundEffects.end()) {
        Mix_PlayChannel(-1, it->second, 0); // Play on any free channel
    } else {
        std::cerr << "Sound effect not found: " << effectName << std::endl;
    }
}
void SoundManager::playFootsteps() {
    // Only play if the footstep channel is not already active
    if (footstepChannel == -1 || !Mix_Playing(footstepChannel)) {
        footstepChannel = Mix_PlayChannel(-1, soundEffects["footstep"], -1); // Play on a free channel in loop
        if (footstepChannel == -1) {
            std::cerr << "Failed to play footstep sound: " << Mix_GetError() << std::endl;
        }
    }
}

void SoundManager::stopFootsteps() {
    if (footstepChannel != -1 && Mix_Playing(footstepChannel)) {
        Mix_HaltChannel(footstepChannel); // Stop the footstep sound
        footstepChannel = -1;
    }
}

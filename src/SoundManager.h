#pragma once
#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include <SDL3/SDL.h>
#include <string>
#include <map>
#include <iostream>

class SoundManager {
public:
    static SoundManager* getInstance();
    ~SoundManager();

    bool init();
    bool loadSound(const std::string& fileName, const std::string& id);
    void playSound(const std::string& id);
    void clean();

private:
    SoundManager();
    static SoundManager* instance;
    
    SDL_AudioDeviceID audioDevice;
    std::map<std::string, SDL_AudioSpec> soundSpecs;
    std::map<std::string, Uint8*> soundBuffers;
    std::map<std::string, Uint32> soundLengths;
};

#endif // SOUND_MANAGER_H
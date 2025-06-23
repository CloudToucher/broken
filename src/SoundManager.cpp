#include "SoundManager.h"

SoundManager* SoundManager::instance = nullptr;

SoundManager* SoundManager::getInstance() {
    if (instance == nullptr) {
        instance = new SoundManager();
    }
    return instance;
}

SoundManager::SoundManager() : audioDevice(0) {}

SoundManager::~SoundManager() {
    clean();
}

bool SoundManager::init() {
    // 初始化SDL音频子系统
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL audio subsystem could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // 打开默认音频设备
    SDL_AudioSpec desired;
    SDL_zero(desired);
    desired.freq = 44100;
    desired.format = SDL_AUDIO_S16;
    desired.channels = 2;
    // SDL3中不再使用samples成员

    // SDL3中的SDL_OpenAudioDevice只接受2个参数
    audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired);
    if (!audioDevice) {
        std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
        return false;
    }

    // 开始播放音频 - SDL3中使用ResumeAudioDevice
    SDL_ResumeAudioDevice(audioDevice);
    
    std::cout << "SDL3 Audio initialized successfully." << std::endl;
    return true;
}

bool SoundManager::loadSound(const std::string& fileName, const std::string& id) {
    // 检查文件扩展名，优先使用.wav文件
    std::string actualFileName = fileName;
    if (fileName.find(".mp3") != std::string::npos) {
        // 尝试使用.wav版本
        std::string wavFileName = fileName.substr(0, fileName.length() - 4) + ".wav";
        FILE* file = nullptr;
        // 使用fopen_s替代fopen
        errno_t err = fopen_s(&file, wavFileName.c_str(), "rb");
        if (file && err == 0) {
            fclose(file);
            actualFileName = wavFileName;
            std::cout << "Using WAV file instead of MP3: " << wavFileName << std::endl;
        }
    }

    // 加载WAV文件
    SDL_AudioSpec spec;
    Uint8* buffer;
    Uint32 length;
    
    if (SDL_LoadWAV(actualFileName.c_str(), &spec, &buffer, &length) == NULL) {
        std::cerr << "Failed to load sound file: " << actualFileName << " Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // 存储音频数据
    soundSpecs[id] = spec;
    soundBuffers[id] = buffer;
    soundLengths[id] = length;
    
    std::cout << "Loaded sound: " << id << " from " << actualFileName << std::endl;
    return true;
}

void SoundManager::playSound(const std::string& id) {
    auto bufferIt = soundBuffers.find(id);
    auto lengthIt = soundLengths.find(id);
    auto specIt = soundSpecs.find(id);
    
    if (bufferIt != soundBuffers.end() && lengthIt != soundLengths.end() && specIt != soundSpecs.end()) {
        // 获取音频设备格式
        SDL_AudioSpec deviceSpec;
        // SDL_GetAudioDeviceFormat返回bool类型，true表示成功
        if (!SDL_GetAudioDeviceFormat(audioDevice, &deviceSpec, NULL)) {
            std::cerr << "Failed to get audio device format: " << SDL_GetError() << std::endl;
            return;
        }
        
        // 创建音频流进行格式转换
        SDL_AudioStream* stream = SDL_CreateAudioStream(&specIt->second, &deviceSpec);
        if (!stream) {
            std::cerr << "Failed to create audio stream: " << SDL_GetError() << std::endl;
            return;
        }
        
        // 将音频数据放入流
        if (SDL_PutAudioStreamData(stream, bufferIt->second, lengthIt->second) < 0) {
            std::cerr << "Failed to put audio data into stream: " << SDL_GetError() << std::endl;
            SDL_DestroyAudioStream(stream);
            return;
        }
        
        // 完成音频流
        SDL_FlushAudioStream(stream);
        
        // 在SDL3中，我们需要将流绑定到设备
        // SDL_BindAudioStream返回bool类型，true表示成功
        if (!SDL_BindAudioStream(audioDevice, stream)) {
            std::cerr << "Failed to bind audio stream: " << SDL_GetError() << std::endl;
            SDL_DestroyAudioStream(stream);
            return;
        }
        
        // 恢复音频设备播放
        SDL_ResumeAudioDevice(audioDevice);
        
        // 注意：不要在这里销毁stream，因为它已经绑定到设备
        // 当音频播放完毕后，SDL会自动解绑并清理资源
        // 或者在下一次播放前，我们可以手动解绑之前的流
    } else {
        std::cerr << "Sound with id '" << id << "' not found." << std::endl;
    }
}

void SoundManager::clean() {
    // 关闭音频设备
    if (audioDevice) {
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
    }
    
    // 释放所有音频缓冲区
    for (auto& pair : soundBuffers) {
        SDL_free(pair.second);
    }
    
    soundSpecs.clear();
    soundBuffers.clear();
    soundLengths.clear();
    
    std::cout << "SDL3 Audio cleaned up." << std::endl;
}
#pragma once

class AudioManager {
public:
    static void init();
    static void playSound(int soundId);
    static void cleanup();
};

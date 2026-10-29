#include "Audio.h"
#include <windows.h>
#include <mmsystem.h>
#include <vector>
#include <cmath>
#include <cstdint>

// Bộ nhớ chứa dữ liệu âm thanh WAV dạng PCM
static std::vector<uint8_t> soundBuffers[7];

static std::vector<uint8_t> createWavBuffer(int sampleRate, float duration, const std::vector<float>& frequencies, int type = 0) {
    int totalSamples = (int)(sampleRate * duration);
    int dataSize = totalSamples;
    int fileSize = 36 + dataSize;

    std::vector<uint8_t> buf;
    buf.reserve(44 + dataSize);

    // Header RIFF WAV
    const char riff[] = "RIFF";
    buf.insert(buf.end(), riff, riff + 4);
    buf.push_back((uint8_t)(fileSize & 0xFF));
    buf.push_back((uint8_t)((fileSize >> 8) & 0xFF));
    buf.push_back((uint8_t)((fileSize >> 16) & 0xFF));
    buf.push_back((uint8_t)((fileSize >> 24) & 0xFF));

    const char wave[] = "WAVEfmt ";
    buf.insert(buf.end(), wave, wave + 8);
    // Subchunk1Size = 16
    buf.push_back(16); buf.push_back(0); buf.push_back(0); buf.push_back(0);
    // AudioFormat = 1 (PCM)
    buf.push_back(1); buf.push_back(0);
    // NumChannels = 1 (Mono)
    buf.push_back(1); buf.push_back(0);
    // SampleRate
    buf.push_back((uint8_t)(sampleRate & 0xFF));
    buf.push_back((uint8_t)((sampleRate >> 8) & 0xFF));
    buf.push_back((uint8_t)((sampleRate >> 16) & 0xFF));
    buf.push_back((uint8_t)((sampleRate >> 24) & 0xFF));
    // ByteRate = SampleRate * NumChannels * BitsPerSample/8
    buf.push_back((uint8_t)(sampleRate & 0xFF));
    buf.push_back((uint8_t)((sampleRate >> 8) & 0xFF));
    buf.push_back((uint8_t)((sampleRate >> 16) & 0xFF));
    buf.push_back((uint8_t)((sampleRate >> 24) & 0xFF));
    // BlockAlign = 1
    buf.push_back(1); buf.push_back(0);
    // BitsPerSample = 8
    buf.push_back(8); buf.push_back(0);

    const char data[] = "data";
    buf.insert(buf.end(), data, data + 4);
    buf.push_back((uint8_t)(dataSize & 0xFF));
    buf.push_back((uint8_t)((dataSize >> 8) & 0xFF));
    buf.push_back((uint8_t)((dataSize >> 16) & 0xFF));
    buf.push_back((uint8_t)((dataSize >> 24) & 0xFF));

    // Tổng hợp dạng sóng (Wave synthesis)
    for (int i = 0; i < totalSamples; ++i) {
        float t = (float)i / sampleRate;
        float sample = 0.0f;

        if (frequencies.size() == 1) {
            float freq = frequencies[0];
            if (type == 1) { // Tiếng rơi/thud giảm dần tần số
                freq = frequencies[0] * (1.0f - (t / duration) * 0.7f);
            }
            float envelope = 1.0f - (t / duration); // Giảm âm lượng theo thời gian
            sample = std::sin(2.0f * 3.14159f * freq * t) * envelope;
        } else if (frequencies.size() > 1) {
            // Chuỗi nốt nhạc arpeggio
            int noteIndex = (int)((t / duration) * frequencies.size());
            if (noteIndex >= (int)frequencies.size()) noteIndex = (int)frequencies.size() - 1;
            float freq = frequencies[noteIndex];
            float noteT = fmod(t, duration / frequencies.size());
            float env = 1.0f - (noteT / (duration / frequencies.size()));
            sample = std::sin(2.0f * 3.14159f * freq * t) * env;
        }

        // Chuyển float [-1..1] thành unsigned byte 8-bit [0..255]
        int byteVal = 128 + (int)(sample * 110.0f);
        if (byteVal < 0) byteVal = 0;
        if (byteVal > 255) byteVal = 255;
        buf.push_back((uint8_t)byteVal);
    }

    return buf;
}

void AudioManager::init() {
    int sr = 22050;
    // 1: Bắt đầu game (Do - Mi - Sol - Do cao)
    soundBuffers[1] = createWavBuffer(sr, 0.35f, { 523.25f, 659.25f, 783.99f, 1046.50f });
    // 2: Di chuyển (Tick ngắn)
    soundBuffers[2] = createWavBuffer(sr, 0.04f, { 880.0f });
    // 3: Xoay khối (Blip vui nhộn)
    soundBuffers[3] = createWavBuffer(sr, 0.06f, { 600.0f, 950.0f });
    // 4: Thả/Khóa khối (Thud đầm ấm)
    soundBuffers[4] = createWavBuffer(sr, 0.12f, { 220.0f }, 1);
    // 5: Xóa tầng (Fanfare hợp âm trong trẻo)
    soundBuffers[5] = createWavBuffer(sr, 0.45f, { 523.25f, 659.25f, 783.99f, 1046.50f, 1318.51f });
    // 6: Game Over (Chuỗi nốt trầm buồn)
    soundBuffers[6] = createWavBuffer(sr, 0.65f, { 440.0f, 370.0f, 311.13f, 220.0f });
}

void AudioManager::playSound(int soundId) {
    if (soundId >= 1 && soundId <= 6 && !soundBuffers[soundId].empty()) {
        PlaySoundA((LPCSTR)soundBuffers[soundId].data(), NULL, SND_MEMORY | SND_ASYNC);
    }
}

void AudioManager::cleanup() {
    PlaySoundA(NULL, NULL, 0);
    for (int i = 0; i <= 6; ++i) {
        soundBuffers[i].clear();
    }
}

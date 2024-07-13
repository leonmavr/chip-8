#ifndef BEEPER_HPP
#define BEEPER_HPP

#include <alsa/asoundlib.h>
#include <cmath>
#include <ctime>
#include <unistd.h>
#include <iostream>

class Beeper {
public:
    Beeper() : freq_(550), duration_ms_(50), sampling_rate_(44100), success_(false) {
        Init();
    }

    Beeper(int freq, int duration_ms) : freq_(freq), duration_ms_(duration_ms), sampling_rate_(44100), success_(false) {
        Init();
    }

    ~Beeper() {
        if (success_) {
            snd_pcm_close(handle_);
        }
    }

    void Beep() {
        if (!success_) return;
        constexpr int channels = 1, soft_resample = 1, latency_us = 500;
        int err = snd_pcm_set_params(handle_, SND_PCM_FORMAT_U8, SND_PCM_ACCESS_RW_INTERLEAVED,
                                     channels, sampling_rate_, soft_resample, latency_us);
        if (err < 0) {
            std::cerr << "Error setting PCM parameters: " << snd_strerror(err) << std::endl;
            return;
        }

        err = snd_pcm_writei(handle_, buffer_.data(), buffer_.size());

        if (err < 0)
            std::cerr << "Error writing to PCM device: " << snd_strerror(err) << std::endl;
    }

private:
    void Init() {
        buffer_size_ = static_cast<size_t>(sampling_rate_ * duration_ms_ / 1000.0);
        buffer_.reserve(buffer_size_); // Reserve the size to avoid repeated allocations
        for (size_t i = 0; i < buffer_size_; ++i) {
            buffer_.push_back(static_cast<unsigned char>(0xFF * sin(2 * M_PI * freq_ * i / sampling_rate_)));
        }

        int err = snd_pcm_open(&handle_, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK /* blocking mode */);
        if (err < 0) {
            std::cerr << "Error opening PCM device: " << snd_strerror(err) << std::endl;
            return;
        }
        success_ = true;
    }

    int sampling_rate_;
    int freq_;
    int duration_ms_;
    /* can alsoaudio be reached? */
    bool success_;
    snd_pcm_t* handle_;
    /* sampled sound wave values */
    std::vector<unsigned char> buffer_;
    size_t buffer_size_;

};

#endif // BEEPER_HPP

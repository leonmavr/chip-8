#include "beeper.hpp"

#include <alsa/asoundlib.h>

#include <cerrno>
#include <chrono>
#include <thread>

namespace {

constexpr unsigned SAMPLE_RATE = 44100;

/** ~10 ms of mono audio per write: long enough that the write paces the loop,
 *  short enough that a tone starts without a perceptible delay. */
constexpr snd_pcm_uframes_t CHUNK_FRAMES = SAMPLE_RATE / 100;

/** Deliberately well below full scale to prevent it from being harsh */
constexpr int16_t AMPLITUDE = 6000;

/** Keep feeding silence for a moment after a tone ends, so a stopped emulator
 *  does not thrash the device with drop/prepare on every call */
constexpr unsigned SILENCE_GRACE_MS = 150;

unsigned NowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

} // namespace

Beeper::~Beeper() {
    if (pcm_ != nullptr) {
        snd_pcm_drop(static_cast<snd_pcm_t*>(pcm_));
        snd_pcm_close(static_cast<snd_pcm_t*>(pcm_));
    }
}

bool Beeper::ok() const {
    return pcm_ != nullptr;
}

bool Beeper::EnsureOpen() {
    if (pcm_ != nullptr)
        return true;
    if (failed_)
        return false;

    snd_pcm_t* pcm = nullptr;
    if (snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        failed_ = true;
        return false;
    }
    // 20 ms of buffering: small enough for a beep to feel immediate
    if (snd_pcm_set_params(pcm, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,
                           1, SAMPLE_RATE, 1, 20000) < 0) {
        snd_pcm_close(pcm);
        failed_ = true;
        return false;
    }
    pcm_ = pcm;
    return true;
}

void Beeper::WriteChunk() {
    snd_pcm_t* pcm = static_cast<snd_pcm_t*>(pcm_);
    int16_t samples[CHUNK_FRAMES];

    if (freq_hz_ == 0) {
        for (snd_pcm_uframes_t i = 0; i < CHUNK_FRAMES; ++i)
            samples[i] = 0;
    } else {
        for (snd_pcm_uframes_t i = 0; i < CHUNK_FRAMES; ++i) {
            phase_ += static_cast<double>(freq_hz_) / SAMPLE_RATE;
            if (phase_ >= 1.0)
                phase_ -= 1.0;
            samples[i] = (phase_ < 0.5) ? AMPLITUDE : -AMPLITUDE;
        }
    }

    snd_pcm_sframes_t written = snd_pcm_writei(pcm, samples, CHUNK_FRAMES);
    if (written == -EPIPE) { // underrun: recover, then retry once
        snd_pcm_prepare(pcm);
        written = snd_pcm_writei(pcm, samples, CHUNK_FRAMES);
    }
    if (written < 0)
        snd_pcm_recover(pcm, static_cast<int>(written), 1);
}

void Beeper::SetTone(bool on, unsigned freq_hz) {
    if (on) {
        if (!EnsureOpen()) {
            failed_ = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return;
        }
        if (!writing_) {
            // Idle device; start it up again from a clean state
            snd_pcm_prepare(static_cast<snd_pcm_t*>(pcm_));
            phase_ = 0.0;
            writing_ = true;
        }
        freq_hz_ = freq_hz;
        last_active_ms_ = NowMs();
        WriteChunk();
        return;
    }

    if (!writing_) {
        // Never started, or already idled: nothing to write.
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return;
    }

    if (NowMs() - last_active_ms_ > SILENCE_GRACE_MS) {
        snd_pcm_drop(static_cast<snd_pcm_t*>(pcm_));
        writing_ = false;
        freq_hz_ = 0;
        return;
    }

    freq_hz_ = 0; // silence
    WriteChunk();
}

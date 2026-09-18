#ifndef BEEPER_HPP
#define BEEPER_HPP

/**
 * Plays a square-wave tone on demand through ALSA (Linux).
 *
 * The audio device is opened lazily on the first tone, 
 * so a program that never beeps never touches audio,
 * and a machine without a sound device simply stays
 * silent instead of failing. ALSA types are kept
 * behind a void* so no external dependencies are needed.
 */
class Beeper {
    public:
        Beeper() = default;
        ~Beeper();
        Beeper(const Beeper&) = delete;
        Beeper& operator=(const Beeper&) = delete;

        /** Whether an audio device is open and usable. */
        bool ok() const;

        /**
         * @brief Emit a square wave at `freq_hz`, or silence when `on` is false.
         *
         * Writes roughly one audio chunk, so calling it in a loop blocks for
         * about that long and the device itself paces the caller.
         */
        void SetTone(bool on, unsigned freq_hz);

    private:
        /** Open the playback device on first use. */
        bool EnsureOpen();
        /** Write one chunk of tone (or silence) to the device. */
        void WriteChunk();

        void* pcm_ = nullptr;  // snd_pcm_t*, hidden to keep ALSA out of this header
        bool writing_ = false; // device is currently being fed samples
        bool failed_ = false;  // opening already failed; don't retry every call
        double phase_ = 0.0;   // position in the waveform, in [0, 1)
        unsigned freq_hz_ = 0;
        unsigned last_active_ms_ = 0;
};

#endif // BEEPER_HPP

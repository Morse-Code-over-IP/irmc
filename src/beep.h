#include <math.h>

#ifdef __MACH__
#define PORTAUDIO
#endif

#ifdef __arm__
#define ALSA
#endif

#ifdef PORTAUDIO
#include <portaudio.h>
#endif

#define NUM_SECONDS   (5)
#define SAMPLE_RATE   (48000)
#define FRAMES_PER_BUFFER  (64)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

#ifdef PORTAUDIO
typedef struct
{
    uint32_t total_count;
    uint32_t up_count;

    uint32_t counter;
    uint32_t prev_freq;
    uint32_t freq;
} paTestData;

static int patestCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData );

void buzzer_set_freq(int frequency);
void buzzer_beep(int frequency, int msecs);
int buzzer_start(void);
int buzzer_stop();
void msleep(int d);
int beep_test(void);
#endif

#ifdef ALSA
#include <stdint.h>
#include <alsa/asoundlib.h>
#include <math.h>

#define FRAMES 32L //1024L //16384L

int16_t buffer[FRAMES*2]; /* 16bit stereo sound samples */

#endif

// compatibility to old interface
int beep(double freq_hz, double duration_sec);
int beep_init();
int beep_close();

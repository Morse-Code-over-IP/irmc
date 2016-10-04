#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h> // for usleep()

#include "beep.h"

#ifdef PORTAUDIO 
// http://stackoverflow.com/questions/7678470/generating-sound-of-a-particular-frequency-using-gcc-in-ubuntu

static PaStream *stream;
static paTestData data;

/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int patestCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    paTestData *data = (paTestData*)userData;
    uint8_t *out = (uint8_t*)outputBuffer;
    unsigned long i;
    uint32_t freq = data->freq;

    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;

    for( i=0; i<framesPerBuffer; i++ )
    {
        if(data->up_count > 0 && data->total_count == data->up_count) {
            *out++ = 0x00;
            continue;
        }
        data->total_count++;

        if(freq != data->prev_freq) {
            data->counter = 0;
        }

        if(freq) {
            int overflow_max = SAMPLE_RATE / freq;
            uint32_t data_cnt = data->counter % overflow_max;
            if(data_cnt > overflow_max/2)
                *out++ = 0xff;
            else {
                *out++ = 0x00;
            }
            data->counter++;
        }
        else {
            data->counter = 0;
            *out++ = 0;
        }
        data->prev_freq = freq;
    }

    return paContinue;
}

void buzzer_set_freq(int frequency)
{
    data.up_count = 0; // do not stop!
    data.freq = frequency;
}

void buzzer_beep(int frequency, int msecs)
{
    data.total_count = 0;
    data.up_count = SAMPLE_RATE * msecs / 1000;
    data.freq = frequency;
}

int buzzer_start(void)
{
    PaStreamParameters outputParameters;

    PaError err;

    err = Pa_Initialize();
    if( err != paNoError ) goto error;

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    outputParameters.channelCount = 1;       /* stereo output */
    outputParameters.sampleFormat = paUInt8; /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
        &stream,
        NULL, /* no input */
        &outputParameters,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,      /* we won't output out of range samples so don't bother clipping them */
        patestCallback,
        &data );
    if( err != paNoError ) goto error;

    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;

    return err;
error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;

}

int buzzer_stop() 
{
    PaError err = 0;
    err = Pa_StopStream( stream );
    if( err != paNoError ) goto error;

    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;

    Pa_Terminate();

    return err;
error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;
}
void msleep(int d){
    usleep(d*1000);
}

int beep(double freq_hz, double duration_sec)
{
	if (freq_hz > 0.0) {	
		buzzer_set_freq(freq_hz);
		msleep(duration_sec*1000.);
		buzzer_set_freq(0.);
	}
	else
	{
		msleep(duration_sec*1000.);
	}
	return 0;
}
int beep_init()
{
	buzzer_start();
	return 0;
}

int beep_close()
{
	buzzer_stop();
	return 0;
}

int beep_test(void)
{

    // notes frequency chart: http://www.phy.mtu.edu/~suits/notefreqs.html

    buzzer_start();
    buzzer_set_freq(261);
    msleep(250);
    buzzer_set_freq(0);
    msleep(250);
    buzzer_set_freq(329);
    msleep(250);
    buzzer_set_freq(349);
    msleep(250);
    buzzer_set_freq(392);
    msleep(250);
    buzzer_set_freq(440);
    msleep(250);
    buzzer_set_freq(494);
    msleep(250);
    buzzer_beep(523, 200);
    msleep(250);

    buzzer_stop();

    return 0;
}

#endif



// Raspi does not work with portaudio?!
#ifdef ALSA 
/*
 * This extra small demo sends sinusoidal  samples to your speakers.
 */

// https://www.raspberrypi.org/forums/viewtopic.php?t=84485&p=603451

static char *device = "hw:0,0"; /* playback device */
snd_output_t *output = NULL;
    snd_pcm_t *handle;

int beep_test(void)
{
  }

int beep(double freq_hz, double duration_sec)
{
  double p1,p2,f1,f2;
    unsigned int i,j;
    snd_pcm_sframes_t frames;
   
 int up_count = (int)(SAMPLE_RATE * duration_sec);
 
 
    p1 = p2 = f1 = f2 = 0.02;
    for (i = 0; i < up_count; ) {
   for (j = 0; j < FRAMES*2 ; j+=2)
   //for (j = 0; j < up_count*2; j+=2)
   {  
i += 2;
if (i<up_count)
{
       buffer[j] = freq_hz*100.0 * sin(p1);
       buffer[j+1] = freq_hz*100.0 * sin(p2);
       p1 += f1;
       p2 += f2;
}
   }

   frames = snd_pcm_writei(handle, buffer, FRAMES);

   if (frames < 0)
       frames = snd_pcm_recover(handle, frames, 0);
   if (frames < 0) {
       //break;
   }
   if (frames > 0 && frames < FRAMES)
       printf("Short write (expected %li, wrote %li)\n", FRAMES, (long)frames);
    }
    return 0;

}
int beep_init()
{
    int err;

   if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
   printf("Playback open error: %s\n", snd_strerror(err));
   exit(EXIT_FAILURE);
    }
    if ((err = snd_pcm_set_params(handle,
              SND_PCM_FORMAT_S16_LE,
              SND_PCM_ACCESS_RW_INTERLEAVED,
              2,
              SAMPLE_RATE,
              1,
              500000)) < 0) { /* 0.5sec */
   printf("Playback open error: %s\n", snd_strerror(err));
   exit(EXIT_FAILURE);
    }
 return 0;
}
int beep_close()
{
    snd_pcm_close(handle);
	return 0;
}

#endif

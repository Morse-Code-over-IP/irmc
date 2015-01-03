/* irmc - internet relay morsecode client */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <soundcard.h>
#include <math.h>

#define SAMPLE_RATE 48000

int sample_rate = SAMPLE_RATE;
int fd_speaker;
int zinebuf[SAMPLE_RATE * 2]; 
int pitch = 650;

void
init_sound()
{
	int i, pr;

	pr = sample_rate / pitch;

	for(i = 0; i < SAMPLE_RATE * 2; i++)
		zinebuf[i] = 32767 * sin(2 * 3.1415 / pr * (double)i );
}

void
play_code_element (int length)
{
  	int i;
  	short buf[SAMPLE_RATE * 4];		

  	int outsz;
	
	outsz = abs(length) * SAMPLE_RATE / 1000;
	
	if(length == 0 || abs(length) > 2000) return;
	if(length < 0) {   /* Space */
		for (i = 0; i < outsz; i++) buf[i] = 0;
	} else { /* Mark */
		for (i = 0; i < outsz; i++) buf[i] = zinebuf[i];
	}

	if (write (fd_speaker, buf, outsz * 2) != (outsz * 2)){
      		perror ("Audio write");
      		exit (-1);
    	}
}


int
open_audio_device (char *name, int mode)
{
  	int tmp, fd;

  	if ((fd = open (name, mode, 0)) == -1){
      		perror (name);
      		//exit (-1);
    	}

  	tmp = AFMT_S16_NE;		/* Native 16 bits */
  	if (ioctl (fd, SNDCTL_DSP_SETFMT, &tmp) == -1){
      		perror ("SNDCTL_DSP_SETFMT");
      		exit (-1);
    	}

  	if (tmp != AFMT_S16_NE){
      		fprintf (stderr,
	       		"The device doesn't support the 16 bit sample format.\n");
      		exit (-1);
    	}

  	tmp = 1;
  	if (ioctl (fd, SNDCTL_DSP_CHANNELS, &tmp) == -1){
      		perror ("SNDCTL_DSP_CHANNELS");
      		exit (-1);
    	}

  	if (tmp != 1){
      		fprintf (stderr, "The device doesn't support mono mode.\n");
      		exit (-1);
    	}

  	//sample_rate = 48000;
  	if (ioctl (fd, SNDCTL_DSP_SPEED, &sample_rate) == -1){
      		perror ("SNDCTL_DSP_SPEED");
      		exit (-1);
    	}

  	return fd;
}



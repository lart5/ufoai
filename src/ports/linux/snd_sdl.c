/**
 * @file snd_sdl.c
 * @brief SDL sound driver
 */

/*

Sound code taken from SDLQuake and modified to work with Quake2
Robert Bml 2001-12-25

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to:

Free Software Foundation, Inc.
59 Temple Place - Suite 330
Boston, MA  02111-1307, USA

*/

#include "snd_sdl.h"

static int  snd_inited;

static struct sndinfo *si;

/**
 * @brief
 */
static void paint_audio (void *unused, Uint8 * stream, int len)
{
	int pos;
	int tobufend;
	int len1;
	int len2;
	if (si->dma) {
		pos = (si->dma->dmapos * (si->dma->samplebits/8));
		if (pos >= si->dma->dmasize)
			si->dma->dmapos = pos = 0;

#if 0
		si->dma->buffer = stream;
		si->dma->samplepos += len / (si->dma->samplebits / 4);

		/* Check for samplepos overflow? */
		S_PaintChannels (si->dma->samplepos);
#else
		/* bytes to buffer's end. */
		tobufend = si->dma->dmasize - pos;
		len1 = len;
		len2 = 0;

		if (len1 > tobufend) {
			len1 = tobufend;
			len2 = len - len1;
		}
		memcpy(stream, si->dma->buffer + pos, len1);
		if (len2 <= 0)
			si->dma->dmapos += (len1 / (si->dma->samplebits/8));
		else {
			/* wraparound? */
			memcpy(stream+len1, si->dma->buffer, len2);
			si->dma->dmapos = (len2 / (si->dma->samplebits/8));
		}
#endif
		if (si->dma->dmapos >= si->dma->dmasize)
			si->dma->dmapos = 0;
	}
}

/**
 * @brief
 */
qboolean SND_Init (struct sndinfo *s)
{
	SDL_AudioSpec desired, obtained;
	int desired_bits, freq;
	char drivername[128];

	if (snd_inited)
		return qtrue;

	si = s;

	snd_inited = 0;

	si->Com_Printf("Soundsystem: SDL.\n");

	if (!SDL_WasInit(SDL_INIT_AUDIO))
		if (SDL_Init(SDL_INIT_AUDIO) == -1) {
			si->Com_Printf("Couldn't init SDL audio: %s\n", SDL_GetError () );
			return qfalse;
		}

	if (SDL_AudioDriverName(drivername, sizeof (drivername)) == NULL)
		strcpy(drivername, "(UNKNOWN)");
	si->Com_Printf("SDL audio driver is \"%s\".\n", drivername);

	memset(&desired, '\0', sizeof (desired));
	memset(&obtained, '\0', sizeof (obtained));

	desired_bits = si->bits->value;

	/* Set up the desired format */
	freq = si->s_khz->value;
	switch (freq) {
	case 44:
		desired.freq = 44100;
		desired.samples = 1024;
		break;
	case 22:
		desired.freq = 22050;
		desired.samples = 512;
		break;
	default:
		desired.freq = 11025;
		desired.samples = 256;
		break;
	}

	switch (desired_bits) {
	case 8:
		desired.format = AUDIO_U8;
		break;
	case 16:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			desired.format = AUDIO_S16MSB;
		else
			desired.format = AUDIO_S16LSB;
		break;
	default:
		si->Com_Printf("Unknown number of audio bits: %d\n", desired_bits);
		return qfalse;
	}
	desired.channels = si->channels->value;
	desired.callback = paint_audio;

	si->Com_Printf ("Bits: %i\n", desired_bits );
	si->Com_Printf ("Frequency: %i\n", desired.freq );
	si->Com_Printf ("Samples: %i\n", desired.samples );
	si->Com_Printf ("Channels: %i\n", desired.channels );

	/* Open the audio device */
	if (SDL_OpenAudio (&desired, &obtained) == -1) {
		si->Com_Printf ("Couldn't open SDL audio: %s\n", SDL_GetError ());
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return qfalse;
	}

	/* Make sure we can support the audio format */
	switch (obtained.format) {
	case AUDIO_U8:
		/* Supported */
		break;
	case AUDIO_S16LSB:
	case AUDIO_S16MSB:
		if (((obtained.format == AUDIO_S16LSB) &&
				(SDL_BYTEORDER == SDL_LIL_ENDIAN)) ||
			((obtained.format == AUDIO_S16MSB) &&
				(SDL_BYTEORDER == SDL_BIG_ENDIAN)))
		{
			/* Supported */
			break;
		}
		/* Unsupported, fall through */ ;
	default:
		/* Not supported -- force SDL to do our bidding */
		SDL_CloseAudio ();
		if (SDL_OpenAudio (&desired, NULL) == -1) {
			si->Com_Printf ("Couldn't open SDL audio (format): %s\n", SDL_GetError ());
			return qfalse;
		}
		memcpy (&obtained, &desired, sizeof (desired));
		break;
	}

	/* Fill the audio DMA information block */
	si->dma->samplebits = (obtained.format & 0xFF);
	si->dma->speed = obtained.freq;
	si->dma->channels = obtained.channels;
	si->dma->samples = obtained.samples * si->dma->channels;
	si->dma->samplepos = 0;
	si->dma->submission_chunk = 1;
	si->dma->dmasize = (si->dma->samples * (si->dma->samplebits/8));
	si->dma->buffer = calloc(1, si->dma->dmasize);;

	SDL_PauseAudio (0);
	snd_inited = 1;
	return qtrue;
}

/**
 * @brief
 */
int SND_GetDMAPos (void)
{
	return si->dma->dmapos;
}

/**
 * @brief
 */
void SND_Shutdown (void)
{
	if (snd_inited) {
		SDL_PauseAudio(1);
		SDL_CloseAudio ();
		snd_inited = 0;
		free(si->dma->buffer);
		si->dma->buffer = NULL;
		si->dma->samplepos = 0;
	}

	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	si->Com_Printf("SDL audio device shut down.\n");
}

/**
 * @brief Send sound to device if buffer isn't really the dma buffer
 */
void SND_Submit (void)
{
	SDL_UnlockAudio();
}

/**
 * @brief
 */
void SND_BeginPainting(void)
{
	SDL_LockAudio();
}

/**
 * @brief
 */
void SND_Activate (qboolean active)
{
}

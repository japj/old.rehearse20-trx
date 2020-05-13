#ifndef RX_ALSALIB_H
#define RX_ALSALIB_H

#include <alsa/asoundlib.h>
#include <opus/opus.h>

int play_one_frame(void *packet,
		size_t len,
		OpusDecoder *decoder,
		snd_pcm_t *snd,
		const unsigned int channels);

#endif
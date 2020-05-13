#include "rx_alsalib.h"
#include "device.h"

int play_one_frame(void *packet,
		size_t len,
		OpusDecoder *decoder,
		snd_pcm_t *snd,
		const unsigned int channels)
{
	int r;
	int16_t *pcm;
	snd_pcm_sframes_t f, samples = 1920;

	pcm = alloca(sizeof(*pcm) * samples * channels);

	if (packet == NULL) {
		r = opus_decode(decoder, NULL, 0, pcm, samples, 1);
	} else {
		r = opus_decode(decoder, packet, len, pcm, samples, 0);
	}
	if (r < 0) {
		fprintf(stderr, "opus_decode: %s\n", opus_strerror(r));
		return -1;
	}

	f = snd_pcm_writei(snd, pcm, r);
	if (f < 0) {
		f = snd_pcm_recover(snd, f, 0);
		if (f < 0) {
			aerror("snd_pcm_writei", f);
			return -1;
		}
		return 0;
	}
	if (f < r)
		fprintf(stderr, "Short write %ld\n", f);

	return r;
}
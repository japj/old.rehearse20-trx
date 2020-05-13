#include "tx_alsalib.h"
#include "device.h"

int send_one_frame(snd_pcm_t *snd,
		const unsigned int channels,
		const snd_pcm_uframes_t samples,
		OpusEncoder *encoder,
		const size_t bytes_per_frame,
		const unsigned int ts_per_frame,
		RtpSession *session)
{
	int16_t *pcm;
	void *packet;
	ssize_t z;
	snd_pcm_sframes_t f;
	static unsigned int ts = 0;

	pcm = alloca(sizeof(*pcm) * samples * channels);
	packet = alloca(bytes_per_frame);

	f = snd_pcm_readi(snd, pcm, samples);
	if (f < 0) {
		if (f == -ESTRPIPE)
			ts = 0;

		f = snd_pcm_recover(snd, f, 0);
		if (f < 0) {
			aerror("snd_pcm_readi", f);
			return -1;
		}
		return 0;
	}

	/* Opus encoder requires a complete frame, so if we xrun
	 * mid-frame then we discard the incomplete audio. The next
	 * read will catch the error condition and recover */

	if (f < samples) {
		fprintf(stderr, "Short read, %ld\n", f);
		return 0;
	}

	z = opus_encode(encoder, pcm, samples, packet, bytes_per_frame);
	if (z < 0) {
		fprintf(stderr, "opus_encode_float: %s\n", opus_strerror(z));
		return -1;
	}

	rtp_session_send_with_ts(session, packet, z, ts);
	ts += ts_per_frame;

	return 0;
}
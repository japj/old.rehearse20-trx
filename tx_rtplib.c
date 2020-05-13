#include "tx_rtplib.h"

unsigned int verbose; // in tx.c

RtpSession* create_rtp_send(const char *addr_desc, const int port)
{
	RtpSession *session;

	session = rtp_session_new(RTP_SESSION_SENDONLY);
	assert(session != NULL);

	rtp_session_set_scheduling_mode(session, 0);
	rtp_session_set_blocking_mode(session, 0);
	rtp_session_set_connected_mode(session, FALSE);
	if (rtp_session_set_remote_addr(session, addr_desc, port) != 0)
		abort();
	if (rtp_session_set_payload_type(session, 0) != 0)
		abort();
	if (rtp_session_set_multicast_ttl(session, 16) != 0)
		abort();
	if (rtp_session_set_dscp(session, 40) != 0)
		abort();

	return session;
}

int run_tx(snd_pcm_t *snd,
		const unsigned int channels,
		const snd_pcm_uframes_t frame,
		OpusEncoder *encoder,
		const size_t bytes_per_frame,
		const unsigned int ts_per_frame,
		RtpSession *session)
{
	for (;;) {
		int r;

		r = send_one_frame(snd, channels, frame,
				encoder, bytes_per_frame, ts_per_frame,
				session);
		if (r == -1)
			return -1;

		if (verbose > 1)
			fputc('>', stderr);
	}
}
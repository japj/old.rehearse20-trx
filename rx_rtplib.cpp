#include "rx_rtplib.h"
#include "payload_type_opus.h"

void timestamp_jump(RtpSession *session, void *a, void *b, void *user_data)
{
	unsigned int verbose = (unsigned long)user_data;
	if (verbose > 1)
		fputc('|', stderr);
	rtp_session_resync(session);
}

RtpSession *create_rtp_recv(const char *addr_desc, const int port,
							unsigned int jitter, unsigned long verbose)
{
	RtpSession *session;

	session = rtp_session_new(RTP_SESSION_RECVONLY);
	rtp_session_set_scheduling_mode(session, FALSE);
	rtp_session_set_blocking_mode(session, FALSE);
	rtp_session_set_local_addr(session, addr_desc, port, -1);
	rtp_session_set_connected_mode(session, FALSE);
	rtp_session_enable_adaptive_jitter_compensation(session, TRUE);
	rtp_session_set_jitter_compensation(session, jitter);  /* ms */
	rtp_session_set_time_jump_limit(session, jitter * 16); /* ms */

	/* set the opus event payload type to 120 in the av profile.
		opusrtp defaults to sending payload type 120
	*/
	rtp_profile_set_payload(&av_profile, 120, &payload_type_opus_mono);
	if (rtp_session_set_payload_type(session, 120) != 0)
		abort();

	if (rtp_session_signal_connect(session, "timestamp_jump",
								   timestamp_jump, (void *)verbose) != 0)
	{
		abort();
	}

	/*
	 * oRTP in RECVONLY mode attempts to send RTCP packets and
	 * segfaults (v4.3.0 tested)
	 *
	 * https://stackoverflow.com/questions/43591690/receiving-rtcp-issues-within-ortp-library
	 */

	rtp_session_enable_rtcp(session, FALSE);

	return session;
}

int run_rx(RtpSession *session,
		   OpusDecoder *decoder,
		   snd_pcm_t *snd,
		   const unsigned int channels,
		   const unsigned int rate, std::atomic<bool> &shouldStop, unsigned int verbose)
{
	int ts = 0;

	for (;;)
	{
		int r, have_more;
		char buf[32768];
		void *packet;

		r = rtp_session_recv_with_ts(session, (uint8_t *)buf,
									 sizeof(buf), ts, &have_more);
		assert(r >= 0);
		assert(have_more == 0);
		if (r == 0)
		{
			packet = NULL;
			if (verbose > 1)
				fputc('#', stderr);
		}
		else
		{
			packet = buf;
			if (verbose > 1)
				fputc('r', stderr);
		}

		r = play_one_frame(packet, r, decoder, snd, channels);
		if (r == -1)
			return -1;

		/* Follow the RFC, payload 0 has 8kHz reference rate */

		ts += r; // follow frames played *8000 / rate;

		if (shouldStop)
		{
			return 0;
		}
	}
}
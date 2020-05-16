#include "tx_rtplib.h"
#include "payload_type_opus.h"

RtpSession *create_rtp_send(const char *addr_desc, const int port, uint32_t ssrc)
{
	RtpSession *session;

	session = rtp_session_new(RTP_SESSION_SENDONLY);
	assert(session != NULL);

	rtp_session_set_scheduling_mode(session, 0);
	rtp_session_set_blocking_mode(session, 0);
	rtp_session_set_connected_mode(session, FALSE);
	if (rtp_session_set_remote_addr(session, addr_desc, port) != 0)
		abort();

	/* set the opus event payload type to 120 in the av profile.
		opusrtp defaults to sending payload type 120
	*/
	rtp_profile_set_payload(&av_profile, 120, &payload_type_opus_mono);
	if (rtp_session_set_payload_type(session, 120) != 0)
		abort();

	if (rtp_session_set_multicast_ttl(session, 16) != 0)
		abort();
	if (rtp_session_set_dscp(session, 40) != 0)
		abort();

	rtp_session_set_ssrc(session, ssrc);

	return session;
}

int run_tx(snd_pcm_t *snd,
		   const unsigned int channels,
		   const snd_pcm_uframes_t frame,
		   OpusEncoder *encoder,
		   const size_t bytes_per_frame,
		   const unsigned int ts_per_frame,
		   RtpSession *session, unsigned int auto_stop_count, unsigned int verbose)
{
	unsigned int cur_count = 0;
	while (1)
	{
		int r;

		r = send_one_frame(snd, channels, frame,
						   encoder, bytes_per_frame, ts_per_frame,
						   session);
		if (r == -1)
			return -1;

		if (verbose > 1)
			fputc('s', stderr);

		cur_count++;
		if (auto_stop_count > 0)
		{
			if (cur_count == auto_stop_count)
			{
				return 1;
			}
		}
	}
}
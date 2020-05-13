#ifndef RX_RTPLIB_H
#define RX_RTPLIB_H

#include <ortp/ortp.h>
#include "rx_alsalib.h"

RtpSession* create_rtp_recv(const char *addr_desc, const int port,
		unsigned int jitter);

int run_rx(RtpSession *session,
		OpusDecoder *decoder,
		snd_pcm_t *snd,
		const unsigned int channels,
		const unsigned int rate);

#endif
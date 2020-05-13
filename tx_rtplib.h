#ifndef TX_RTPLIB_H
#define TX_RTPLIB_H

#include <ortp/ortp.h>
#include "tx_alsalib.h"

#ifdef __cplusplus
extern "C"
{
#endif

	RtpSession *create_rtp_send(const char *addr_desc, const int port);
	int run_tx(snd_pcm_t *snd,
			   const unsigned int channels,
			   const snd_pcm_uframes_t frame,
			   OpusEncoder *encoder,
			   const size_t bytes_per_frame,
			   const unsigned int ts_per_frame,
			   RtpSession *session);

#ifdef __cplusplus
}
#endif

#endif
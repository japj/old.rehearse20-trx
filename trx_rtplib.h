#ifndef TRX_RTPLIB_H
#define TRX_RTPLIB_H

#include <ortp/ortp.h>

#ifdef __cplusplus
extern "C"
{
#endif

    RtpSession *create_rtp_sendrecv(const char *send_addr_desc, const int send_port, const char *recv_addr_desc, const int recv_port,
                                    unsigned int jitter, uint32_t ssrc);

#ifdef __cplusplus
}
#endif

#endif
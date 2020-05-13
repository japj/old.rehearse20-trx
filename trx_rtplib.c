#include "trx_rtplib.h"

#include "rx_rtplib.h"

RtpSession *create_rtp_sendrecv(const char *send_addr_desc, const int send_port, const char *recv_addr_desc, const int recv_port,
                                unsigned int jitter)
{
    RtpSession *session;

    session = rtp_session_new(RTP_SESSION_SENDRECV);
    assert(session != NULL);

    rtp_session_set_scheduling_mode(session, 0);
    rtp_session_set_blocking_mode(session, 0);
    rtp_session_set_connected_mode(session, FALSE);
    if (rtp_session_set_remote_addr(session, send_addr_desc, send_port) != 0)
        abort();
    if (rtp_session_set_payload_type(session, 0) != 0)
        abort();
    if (rtp_session_set_multicast_ttl(session, 16) != 0)
        abort();
    if (rtp_session_set_dscp(session, 40) != 0)
        abort();

    rtp_session_set_local_addr(session, recv_addr_desc, recv_port, -1);
    rtp_session_enable_adaptive_jitter_compensation(session, TRUE);
    rtp_session_set_jitter_compensation(session, jitter);  /* ms */
    rtp_session_set_time_jump_limit(session, jitter * 16); /* ms */
    if (rtp_session_signal_connect(session, "timestamp_jump",
                                   timestamp_jump, 0) != 0)
    {
        abort();
    }

    return session;
}

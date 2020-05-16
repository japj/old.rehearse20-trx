
#include <netdb.h>
#include <string.h>
#include <unistd.h>
extern char *optarg;

#include <sys/socket.h>
#include <sys/types.h>

#include <thread>

#include "tx_alsalib.h"
#include "tx_rtplib.h"
#include "rx_alsalib.h"
#include "rx_rtplib.h"
#include "trx_rtplib.h"

#include "defaults.h"
#include "device.h"
#include "notice.h"
#include "sched.h"

unsigned int verbose = DEFAULT_VERBOSE;

static void usage(FILE *fd)
{
    fprintf(fd, "Usage: trx [<parameters>]\n"
                "Real-time audio transmitter and receiver over IP\n");

    fprintf(fd, "\nAudio device (ALSA) parameters:\n");
    fprintf(fd, "  -i <dev>    Input Device name (default '%s')\n",
            DEFAULT_DEVICE);
    fprintf(fd, "  -o <dev>    Output Device name (default '%s')\n",
            DEFAULT_DEVICE);
    fprintf(fd, "  -m <ms>     Buffer time (default %d milliseconds)\n",
            DEFAULT_BUFFER);

    fprintf(fd, "\nNetwork parameters:\n");
    fprintf(fd, "  -h <addr>   IP address to send to (default %s)\n",
            DEFAULT_ADDR);
    fprintf(fd, "  -s <port>   UDP port number to send to (default %d)\n",
            DEFAULT_PORT);
    fprintf(fd, "  -p <port>   UDP port number to receive on (default %d)\n",
            DEFAULT_PORT);
    fprintf(fd, "  -j <ms>     Jitter buffer (default %d milliseconds)\n",
            DEFAULT_JITTER);
    fprintf(fd, "  -S <ssrc>   SSRC (default %d)\n",
            DEFAULT_SSRC);

    fprintf(fd, "\nEncoding parameters:\n");
    fprintf(fd, "  -r <rate>   Sample rate (default %dHz)\n",
            DEFAULT_RATE);
    fprintf(fd, "  -c <n>      Number of channels (default %d)\n",
            DEFAULT_CHANNELS);
    fprintf(fd, "  -f <n>      Frame size (default %d samples, see below)\n",
            DEFAULT_FRAME);
    fprintf(fd, "  -b <kbps>   Bitrate (approx., default %d)\n",
            DEFAULT_BITRATE);

    fprintf(fd, "\nProgram parameters:\n");
    fprintf(fd, "  -A <n>      Automatic stop after (default %d) messages, 0 = don't stop\n",
            DEFAULT_AUTOSTOPCOUNT);
    fprintf(fd, "  -v <n>      Verbosity level (default %d)\n",
            DEFAULT_VERBOSE);
    fprintf(fd, "  -D <file>   Run as a daemon, writing process ID to the given file\n");

    fprintf(fd, "\nAllowed frame sizes (-f) are defined by the Opus codec. For example,\n"
                "at 48000Hz the permitted values are 120, 240, 480 or 960.\n");
}

#define DO_SEND 1
#define DO_RECEIVE 1

int main(int argc, char *argv[])
{
    int r, error;
    size_t bytes_per_frame;
    unsigned int ts_per_frame;

#if DO_SEND
    snd_pcm_t *input_snd;
    OpusEncoder *encoder;
#endif
#if DO_RECEIVE
    snd_pcm_t *output_snd;
    OpusDecoder *decoder;
#endif

    RtpSession *sendrecv_session;

    /* command-line options */
    const char
#if DO_SEND
        *input_device = DEFAULT_DEVICE,
#endif
#if DO_RECEIVE
        *output_device = DEFAULT_DEVICE,
#endif
        *send_addr = DEFAULT_ADDR,
        *pid = NULL;
    unsigned int buffer = DEFAULT_BUFFER,
                 rate = DEFAULT_RATE,
                 jitter = DEFAULT_JITTER,
                 channels = DEFAULT_CHANNELS,
                 frame = DEFAULT_FRAME,
                 kbps = DEFAULT_BITRATE,
                 receive_port = DEFAULT_PORT,
                 send_port = DEFAULT_PORT,
                 auto_stop_count = DEFAULT_AUTOSTOPCOUNT;
    uint32_t ssrc = DEFAULT_SSRC;

    fputs(COPYRIGHT "\n", stderr);

    for (;;)
    {
        int c;

        c = getopt(argc, argv, "b:c:f:h:i:j:m:o:p:r:s:v:A:D:S:");
        if (c == -1)
            break;

        switch (c)
        {
        case 'b':
            kbps = atoi(optarg);
            break;
        case 'c':
            channels = atoi(optarg);
            break;
        case 'f':
            frame = atol(optarg);
            break;
        case 'h':
            send_addr = optarg;
            break;
        case 'i':
            input_device = optarg;
            break;
        case 'j':
            jitter = atoi(optarg);
            break;
        case 'm':
            buffer = atoi(optarg);
            break;
#if DO_RECEIVE
        case 'o':
            output_device = optarg;
            break;
#endif
        case 'p':
            receive_port = atoi(optarg);
            break;
        case 'r':
            rate = atoi(optarg);
            break;
        case 's':
            send_port = atoi(optarg);
            break;
        case 'v':
            verbose = atoi(optarg);
            break;
        case 'A':
            auto_stop_count = atoi(optarg);
            break;
        case 'D':
            pid = optarg;
            break;
        case 'S':
            ssrc = atoi(optarg);
            break;
        default:
            usage(stderr);
            return -1;
        }
    }

    encoder = opus_encoder_create(rate, channels, OPUS_APPLICATION_AUDIO,
                                  &error);
    if (encoder == NULL)
    {
        fprintf(stderr, "opus_encoder_create: %s\n",
                opus_strerror(error));
        return -1;
    }

#if DO_RECEIVE
    decoder = opus_decoder_create(rate, channels, &error);
    if (decoder == NULL)
    {
        fprintf(stderr, "opus_decoder_create: %s\n",
                opus_strerror(error));
        return -1;
    }
#endif
    bytes_per_frame = kbps * 1024 * frame / rate / 8;

    /* Follow the RFC, payload 0 has 8kHz reference rate */

    ts_per_frame = frame * 8000 / rate;

    ortp_init();
    ortp_scheduler_init();
    ortp_set_log_level_mask(NULL, ORTP_WARNING | ORTP_ERROR);

#if DO_RECEIVE
#if DO_SEND
    sendrecv_session = create_rtp_sendrecv(send_addr, send_port, "0.0.0.0", receive_port, jitter, ssrc);
#else
    sendrecv_session = create_rtp_recv("0.0.0.0", receive_port, jitter);
#endif
#else
    sendrecv_session = create_rtp_send(send_addr, send_port);
#endif
    assert(sendrecv_session != NULL);

    r = snd_pcm_open(&input_snd, input_device, SND_PCM_STREAM_CAPTURE, 0);
    if (r < 0)
    {
        aerror("snd_pcm_open", r);
        return -1;
    }
    if (set_alsa_hw(input_snd, rate, channels, buffer * 1000) == -1)
        return -1;
    if (set_alsa_sw(input_snd) == -1)
        return -1;

#if DO_RECEIVE
    r = snd_pcm_open(&output_snd, output_device, SND_PCM_STREAM_PLAYBACK, 0);
    if (r < 0)
    {
        aerror("snd_pcm_open", r);
        return -1;
    }
    if (set_alsa_hw(output_snd, rate, channels, buffer * 1000) == -1)
        return -1;
    if (set_alsa_sw(output_snd) == -1)
        return -1;
#endif

    if (pid)
        go_daemon(pid);

    go_realtime();
    /*r = run_tx(input_snd, channels, frame, encoder, bytes_per_frame,
               ts_per_frame, send_session);*/

    std::atomic<bool> shouldStop(false);

#if DO_SEND
    std::thread send_thread(run_tx, input_snd, channels, frame, encoder, bytes_per_frame,
                            ts_per_frame, sendrecv_session, auto_stop_count, verbose);
#endif
    /*r = run_rx(receive_session, decoder, output_snd, channels, rate);*/

#if DO_RECEIVE
    std::thread receive_thread(run_rx, sendrecv_session, decoder, output_snd, channels, rate, std::ref(shouldStop), verbose);
#endif

#if DO_SEND
    send_thread.join();
#endif
    printf("\nsend_thread done\n");
    shouldStop = true;
#if DO_RECEIVE
    receive_thread.join();
    printf("\nreceive_thread done\n");
#endif

    if (snd_pcm_close(input_snd) < 0)
        abort();

#if DO_RECEIVE
    if (snd_pcm_close(output_snd) < 0)
        abort();
#endif

    rtp_session_destroy(sendrecv_session);

    ortp_exit();
    ortp_global_stats_display();

    opus_encoder_destroy(encoder);
#if DO_RECEIVE
    opus_decoder_destroy(decoder);
#endif
    return r;
}
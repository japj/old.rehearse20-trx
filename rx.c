/*
 * Copyright (C) 2020 Mark Hills <mark@xwax.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#include <netdb.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>

#include "rx_rtplib.h"
#include "rx_alsalib.h"

#include "defaults.h"
#include "device.h"
#include "notice.h"
#include "sched.h"

unsigned int verbose = DEFAULT_VERBOSE;

static void usage(FILE *fd)
{
	fprintf(fd, "Usage: rx [<parameters>]\n"
		"Real-time audio receiver over IP\n");

	fprintf(fd, "\nAudio device (ALSA) parameters:\n");
	fprintf(fd, "  -d <dev>    Device name (default '%s')\n",
		DEFAULT_DEVICE);
	fprintf(fd, "  -m <ms>     Buffer time (default %d milliseconds)\n",
		DEFAULT_BUFFER);

	fprintf(fd, "\nNetwork parameters:\n");
	fprintf(fd, "  -h <addr>   IP address to listen on (default %s)\n",
		DEFAULT_ADDR);
	fprintf(fd, "  -p <port>   UDP port number (default %d)\n",
		DEFAULT_PORT);
	fprintf(fd, "  -j <ms>     Jitter buffer (default %d milliseconds)\n",
		DEFAULT_JITTER);

	fprintf(fd, "\nEncoding parameters (must match sender):\n");
	fprintf(fd, "  -r <rate>   Sample rate (default %dHz)\n",
		DEFAULT_RATE);
	fprintf(fd, "  -c <n>      Number of channels (default %d)\n",
		DEFAULT_CHANNELS);

	fprintf(fd, "\nProgram parameters:\n");
	fprintf(fd, "  -v <n>      Verbosity level (default %d)\n",
		DEFAULT_VERBOSE);
	fprintf(fd, "  -D <file>   Run as a daemon, writing process ID to the given file\n");
}

int main(int argc, char *argv[])
{
	int r, error;
	snd_pcm_t *snd;
	OpusDecoder *decoder;
	RtpSession *session;

	/* command-line options */
	const char *device = DEFAULT_DEVICE,
		*addr = DEFAULT_ADDR,
		*pid = NULL;
	unsigned int buffer = DEFAULT_BUFFER,
		rate = DEFAULT_RATE,
		jitter = DEFAULT_JITTER,
		channels = DEFAULT_CHANNELS,
		port = DEFAULT_PORT;

	fputs(COPYRIGHT "\n", stderr);

	for (;;) {
		int c;

		c = getopt(argc, argv, "c:d:h:j:m:p:r:v:");
		if (c == -1)
			break;
		switch (c) {
		case 'c':
			channels = atoi(optarg);
			break;
		case 'd':
			device = optarg;
			break;
		case 'h':
			addr = optarg;
			break;
		case 'j':
			jitter = atoi(optarg);
			break;
		case 'm':
			buffer = atoi(optarg);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'r':
			rate = atoi(optarg);
			break;
		case 'v':
			verbose = atoi(optarg);
			break;
		case 'D':
			pid = optarg;
			break;
		default:
			usage(stderr);
			return -1;
		}
	}

	decoder = opus_decoder_create(rate, channels, &error);
	if (decoder == NULL) {
		fprintf(stderr, "opus_decoder_create: %s\n",
			opus_strerror(error));
		return -1;
	}

	ortp_init();
	ortp_scheduler_init();
	session = create_rtp_recv(addr, port, jitter);
	assert(session != NULL);

	r = snd_pcm_open(&snd, device, SND_PCM_STREAM_PLAYBACK, 0);
	if (r < 0) {
		aerror("snd_pcm_open", r);
		return -1;
	}
	if (set_alsa_hw(snd, rate, channels, buffer * 1000) == -1)
		return -1;
	if (set_alsa_sw(snd) == -1)
		return -1;

	if (pid)
		go_daemon(pid);

	go_realtime();
	r = run_rx(session, decoder, snd, channels, rate);

	if (snd_pcm_close(snd) < 0)
		abort();

	rtp_session_destroy(session);
	ortp_exit();
	ortp_global_stats_display();

	opus_decoder_destroy(decoder);

	return r;
}

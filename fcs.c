/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * Author: Marco Guerri <gmarco.dev@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>
#include <net/if.h>

int 
main(int argc, char *argv[])
{
	/* Create packet socket for sending layer 2 frames. SOCK_RAW indicates packet with
 	 * link level header, passed to the device driver without any change in the packet
 	 * data. */
	int sock_fd = socket(PF_PACKET, SOCK_RAW, 0x1234);
	if(sock_fd == -1)
	{
		perror("socket");
		return EXIT_FAILURE;
	}
	
	struct sockaddr_ll link_level_hdr = {
											.sll_family = AF_PACKET,
											.sll_protocol = htons(0x1234),
											.sll_ifindex = if_nametoindex("ens20f1"),
	};

	if(link_level_hdr.sll_ifindex == 0)
	{
		perror("if_nametoindex");
		return EXIT_FAILURE;
	}

	int ret = bind(sock_fd, (struct sockaddr*)(&link_level_hdr), sizeof(link_level_hdr));
	if(ret < 0)
	{
		perror("bind");
		return EXIT_FAILURE;
	}

	unsigned int optval = 1;	
	/* Change option at socket level (SOL_SOCKET) */
	if(setsockopt(sock_fd, SOL_SOCKET, 43, (void*)&optval, sizeof(optval)) < 0)
	{
		if(errno == -ENOPROTOOPT)
			fprintf(stderr, "SO_NOFCS is not supported by the driver\n");

		perror("setsocketopt");

		return EXIT_FAILURE;
	}

	char buff[21];
 
	buff[0] = 0xFC;
	buff[1] = 0xAA;
	buff[2] = 0x14;
	buff[3] = 0xE4;
	buff[4] = 0x97;
	buff[5] = 0x14;
	
	buff[6]  = 0xAA;
	buff[7]  = 0xBB;
	buff[8]  = 0xCC;
	buff[9]  = 0xDD;
	buff[10] = 0xEE;
	buff[11] = 0xFF;

	buff[12] = 0x12;
	buff[13] = 0x13;

	buff[14] = 0x66;
	buff[15] = 0x66;
	buff[16] = 0x66;

	buff[17] = 0xDE;
	buff[18] = 0xAD;
	buff[19] = 0xBE;
	buff[20] = 0xEF;

	if(sendto(sock_fd, buff, 21, 0, NULL, 0) < 0)
	{
		perror("sendto");
		return EXIT_FAILURE;
	}	

	fprintf(stderr, "Message sent correctly\n");
	return EXIT_SUCCESS;


}


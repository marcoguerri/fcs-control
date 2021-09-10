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
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>


#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <fcntl.h>

#include "crc.h"

#define MAC_STR_LEN 17
#define MAC_BYTE_LEN 6

int8_t hex_to_dec(char c)
{
    switch(c)
    {
        case 'F':
        case 'f':
            return 0xF;
            break;
        case 'E':
        case 'e':
            return 0xE;
            break;
        case 'D':
        case 'd':
            return 0xD;
            break;
        case 'C':
        case 'c':
            return 0xC;
            break;
        case 'B':
        case 'b':
            return 0xB;
        case 'A':
        case 'a':
            return 0xA;
        case '9':
            return 0x9;
        case '8':
            return 0x8;
        case '7':
            return 0x7;
        case '6':
            return 0x6;
        case '5':
            return 0x5;
        case '4':
            return 0x4;
        case '3':
            return 0x3;
        case '2':
            return 0x2;
        case '1':
            return 0x1;
        case '0':
            return 0x0;
        default:
            return -1;
    }
}

/**
 * Converts string representation of MAC address into uint8_t[6]
 * Expecting MAC address in the form AA:BB:CC:DD:EE:FF
 */
uint8_t* str_to_mac(char *str)
{
    assert(str != NULL && strlen(str) == MAC_STR_LEN);
    uint8_t mul_factor = 0x10, out_index = 0, in_index = 0;
    uint8_t *mac = (uint8_t*)malloc(sizeof(uint8_t) * MAC_BYTE_LEN);
    memset(mac, 0x0, MAC_BYTE_LEN);

    char c = str[in_index];
    while(c) 
    {
        if(c == ':')
        {
            ++out_index;
            mul_factor = 0x10;
        }
        else
        {
            int8_t digit = hex_to_dec(c);
            if(digit == -1)
                goto err;
            mac[out_index] += digit * mul_factor;
            mul_factor = mul_factor == 0x10 ? 0x1 : 0x10;
        }
        c = str[++in_index];
    }
    if(out_index < MAC_BYTE_LEN - 1)
        goto err;
    assert(mac != NULL);
    return mac;
err:
    free(mac);
    return NULL;
}

int 
main(int argc, char *argv[])
{
    int c;
    extern char *optarg;
    extern int optind, optopt;
    
    char *intf = NULL;
    char *mac_str  = NULL;

    uint8_t *dest_mac = NULL;
    uint8_t corrupt = 0;

    crc_params_t crc_params;
    
    while ( (c = getopt(argc, argv, "i:m:c")) != -1) {
        switch (c) {
        case 'i':
            intf  = strdup(optarg);
            if(intf == NULL)
            {
                perror("strdup\n");
                return EXIT_FAILURE;
            }
            fprintf(stderr, "Interface is %s\n", intf);
            break;
        case 'm':
            mac_str = strdup(optarg);
            if(mac_str == NULL)
            {
                perror("strdup\n");
                return EXIT_FAILURE;

            }
            if(strlen(mac_str) != MAC_STR_LEN)
            {
                fprintf(stderr, "MAC address is malformed\n");
                return EXIT_FAILURE;
            }
            dest_mac = str_to_mac(mac_str);
            if(dest_mac == NULL)
            {
                fprintf(stderr, "Destination MAC is malformed\n");
                return EXIT_FAILURE;
            }
            fprintf(stderr, "Destination MAC address is %s\n", mac_str);
            break;
        case 'c':
            corrupt = 1;
            break;

        default:
            break;
        }
    }
    if (mac_str == NULL) {
        fprintf(stderr, "Dest MAC cannot be NULL (-m)\n");
        return EXIT_FAILURE;
    }
    if (intf == NULL) {
        fprintf(stderr, "Output interface cannot be NULL (-i)\n");
        return EXIT_FAILURE;
    }
    assert(intf != NULL && mac_str != NULL);

    /* Create packet socket for sending layer 2 frames. SOCK_RAW indicates packet with
     * link level header, passed to the device driver without any change in the packet
     * data. */
    int sock_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(sock_fd == -1)
    {
        perror("socket");
        return EXIT_FAILURE;
    }
    
    struct sockaddr_ll link_level_hdr = {
        .sll_family = PF_PACKET,
        .sll_protocol = htons(ETH_P_ALL),
        .sll_ifindex = if_nametoindex(intf),
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
    /* Change option at socket level (SOL_SOCKET). Frame must be at least 64 bytes 
     * in total or SOL_SOCKET will be basically ignored and CRC will be appended */

    if(setsockopt(sock_fd, SOL_SOCKET, 43, (void*)&optval, sizeof(optval)) < 0)
    {
        if(errno == -ENOPROTOOPT)
            fprintf(stderr, "SO_NOFCS is not supported by the driver\n");

        perror("setsocketopt");

        return EXIT_FAILURE;
    } 
    srand (time(NULL));

    /* Building the frame */
    uint8_t frame[64];

    /* Dest MAC address */
    memcpy(frame, dest_mac, MAC_BYTE_LEN);

    /* Receiver MAC address */
    uint8_t src_mac[MAC_BYTE_LEN] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    memcpy(frame + MAC_BYTE_LEN, src_mac, MAC_BYTE_LEN);

    /* Type */
    frame[12] = 0x12;
    frame[13] = 0x13;

    /* Minimum payload lenght for Ethernet frame is 46 bytes */
    memset((void*)frame+14, 0x00, 46);

    /* Frame Check Sequence */
    if(corrupt) 
    {
        uint32_t crc_corrupted = 0xDEADBEEF;
        memcpy(frame+60, &crc_corrupted, 4);
    } 
    else
    {
        crc_params.type = CRC32;
        crc_params.poly.poly_crc32 = 0x04C11DB7;
        crc_params.crc_init.crc32 = 0xFFFFFFFF;
        crc_params.flags = CRC_INPUT_REVERSAL |
                           CRC_OUTPUT_REVERSAL |
                           CRC_OUTPUT_INVERSION;
    
        crc_t fcs =  crc_fast(&crc_params, (uint8_t*)frame, 60);
        fprintf(stderr, "crc: %x\n", htonl(fcs.crc32));
        memcpy(frame+60, &fcs, 4);
    }

    
    if(sendto(sock_fd, frame, 64, 0, NULL, 0) < 0)
    {
        perror("sendto");
        return EXIT_FAILURE;
    }

    fprintf(stderr, "Message sent correctly\n");
    return EXIT_SUCCESS;
}


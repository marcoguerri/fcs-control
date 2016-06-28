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
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>
#include <net/if.h>


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
            mul_factor = mul_factor == 0x10 ? 0x1 : 0xF;
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

    while ( (c = getopt(argc, argv, "i:m:")) != -1) {
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
            uint8_t *mac = str_to_mac(mac_str);
            if(mac == NULL)
            {
                fprintf(stderr, "Destination MAC is malformed\n");
                return EXIT_FAILURE;
            }
            fprintf(stderr, "Destination MAC address is %s\n", mac_str);
            break;

        default:
            break;
        }
    }
    
    assert(intf != NULL && mac_str != NULL);
        
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


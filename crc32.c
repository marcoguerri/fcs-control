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

#include <stdint.h>
#include "crc.h"

uint32_t reflect32(uint32_t in_byte) {
    uint32_t temp = 0;
    uint32_t i;
    for(i = 0; i < 16; i++ )
    {
        temp = temp | ((in_byte & (0x80000000 >> i)) >> (31-2*i))
                    | ((in_byte & (0x00000001 << i)) << (31-2*i));
    }
    return temp;
}

uint8_t 
reflect8(uint8_t in_byte)
{
    uint8_t temp = 0;
    uint8_t i;
    for(i = 0; i < 8; i++ )
    {
        temp = temp | ((in_byte & (0x80 >> i)) >> (8-2*i))
                    | ((in_byte & (0x01 << i)) << (8-2*i));
    }
    return temp;
}



/**
 * First part of the CRC32 algorithm with some IEEE 802.3 specific variations
 * Requirements:
 * - Initialization of the shift register with 0xFFFFFFFF
 * - Bytewise reflection before feeding the shift register
 */

uint32_t
_crc32(char *message, uint8_t msg_len) {

    uint32_t remainder = 0xFFFFFFFF;
    //uint32_t remainder = 0x00000000;
    uint32_t poly = 0x04C11DB7;
    uint8_t current_bit, i, j,  _xor;

    for(j=0; j<msg_len; j++)
    {
        for(i=0; i<8; i++)
        {
             current_bit = (message[j] >> i) & 0x1;
            _xor = ((remainder >> 31) ^ current_bit);
            remainder = (remainder << 1);
            if(_xor == 1)
                remainder = remainder ^ poly;
        }
    }

    return remainder;
    /*
     * IEEE 802.3 does not require to mirror the CRC, i.e.
     * (remainder & 0xFF) << 24 | (remainder & 0xFF00) << 8 |
     *        (remainder & 0xFF000000) >> 24 | (remainder & 0x00FF0000) >> 8;
     */
}


/*
 * Final part of the CRC32 algorithm according to IEEE 802.3:
 * - Reflection of the result
 * - Inversion of the result
 *
 * After these steps, if the receiver feeds the shift register with the data
 * transmitted one byte at at time LSB first, as the Ethernet standard encforces
 * (apart from CRC which is transmitted MSB first) if the CRC is correct it
 * should obtain the magic number 0xC704DD7B.
 */

uint32_t
crc32_ethernet(uint32_t crc)
{
   return ~reflect32(crc);
}


uint32_t
crc32_fast(char *message, uint8_t msg_len)
{
    uint8_t j;
    uint32_t remainder = 0xFFFFFFFF;
    for(j=0; j<msg_len; j++)
        remainder = crc32_table[message[j] ^ (remainder & 0xFF)] ^ (remainder >> 8);
    
    return reflect32(remainder);
}


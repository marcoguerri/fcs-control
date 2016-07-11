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
#include <string.h>
#include <stdio.h>
#include "crc.h"
#include "test.h"

extern uint8_t *binary_test1_start;
extern uint8_t *binary_test2_start;

typedef struct {
    uint8_t const *buffer;
    uint32_t len;
    uint32_t crc32;

} test;

char *message =  "Nel mezzo del cammin di nostra vita mi ritrovai per una selva oscura";

uint32_t 
test_crc32_returns_correct_value()
{
    uint32_t temp = crc32(message, strlen(message));
    _assert(temp == 0xcf2a3882);
    return 0;
}

uint32_t
test_crc32_fast_returns_same_result_as_crc32()
{

    uint32_t temp = crc32(message, strlen(message));
    uint32_t temp2 = crc32_fast(message, strlen(message));
    printf("%x\n", temp2);
    printf("Correct %x\n", temp);
    _assert(temp == temp2);
    return 0;


}

void 
compute_crc32_table() 
{
    uint8_t temp = 0;
    while(temp != 255)
    {
        if(temp % 4 == 0)
        {
            printf("\n");
        }
        printf("0x%.8x, ", crc32((char*)&temp, 1));
        ++temp;
    }

    printf("0x%.8x, ", crc32((char*)&temp, 1));
}


int
main(int argc, char *argv[])
{

    test test1 = {
        .buffer = binary_test1_start,
        .len = 60,
        .crc32 = 0x11d056c3,
    };
    
    test test2 = {
        .buffer = binary_test2_start,
        .len = 60,
        .crc32 = 0xbb32734,
    };
    
    //compute_crc32_table();
    test_crc32_returns_correct_value();
    test_crc32_fast_returns_same_result_as_crc32();
    return 0;
}

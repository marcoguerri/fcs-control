#ifndef __CRC_H__
#define __CRC_H__

uint32_t crc32(char *message, uint8_t msg_len);
uint32_t crc32_fast(char *message, uint8_t msg_len);
uint32_t crc32_ethernet(uint32_t crc32);
extern uint32_t crc32_table[];
uint32_t reflect32(uint32_t in_byte);
#endif

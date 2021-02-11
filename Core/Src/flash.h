/*
 * flash.h
 *
 *  Created on: 2021/01/26
 *      Author: tenda_000
 */

#ifndef SRC_FLASH_H_
#define SRC_FLASH_H_

typedef enum {false, true} boolean;

#define FLASH_BASE_ADDR (0x08000000UL)
#define BLOCK_SIZE (0x800)
#define FLASH_ADDR(rel_addr) (FLASH_BASE_ADDR + (uintptr_t)rel_addr)

#define FLAH_BLOCK_PTR(block, num) (uint16_t *)FLASH_ADDR(block * BLOCK_SIZE + num * 2)
#define FLASH_BLOCK_ACCESS(block, num) (*FLAH_BLOCK_PTR(block, num))


void flash_init(void);
void flash_lock(void);
boolean flash_write(uint16_t dat, uint16_t *addr);
boolean flash_erase_at(uint16_t *addr);
boolean flash_write_32(uint32_t dat, uint32_t *addr);
boolean flash_erase_at_32(uint32_t *addr);
boolean flash_erase_all(void);

#endif /* SRC_FLASH_H_ */

/*
 * flash.c
 *
 *  Created on: 2021/01/26
 *      Author: tenda_000
 */

#include "main.h"
#include "flash.h"

// inhouse flash page7

#define PAGE_NUM 7

void flash_init(void) {
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;
}

void flash_lock(void) {
	FLASH->CR |= FLASH_CR_LOCK;
}

inline static void wait_for_busy_is_clr(void) {
	while( FLASH->SR & FLASH_SR_BSY );
}

inline static boolean check_eop(void) {
	if( FLASH->SR & FLASH_SR_EOP ) {
		FLASH->SR &= ~FLASH_SR_EOP;
		return true;
	}
	return false;
}

boolean flash_write(uint16_t dat, uint16_t *addr) {
	if( *addr != 0xFFFF ) {
		return false;
	}
	wait_for_busy_is_clr();
	FLASH->CR |= FLASH_CR_PG;

	*addr = dat;
	wait_for_busy_is_clr();
	FLASH->CR &= ~FLASH_CR_PG;

	return check_eop();
}


boolean flash_erase_at(uint16_t *addr) {
	wait_for_busy_is_clr();
	FLASH->CR |= FLASH_CR_PER;
	FLASH->AR = FLASH_ADDR(addr);
	FLASH->CR |= FLASH_CR_STRT;

	wait_for_busy_is_clr();
	FLASH->CR &= ~FLASH_CR_PER;

	return check_eop();
}

boolean flash_write_32(uint32_t dat, uint32_t *addr) {
	if( flash_write(dat & 0xFFFF, (uint16_t *)addr) == false ||
			flash_write(dat >> 16, ((uint16_t *)addr + 1)) == false ) {
		return false;
	}
	return true;
}

boolean flash_erase_at_32(uint32_t *addr) {
	if( flash_erase_at((uint16_t *)addr) == false || flash_erase_at((uint16_t *)addr + 1) == false ) {
		return false;
	}
	return true;
}

boolean flash_erase_all(void) {
	wait_for_busy_is_clr();
	FLASH->CR |= FLASH_CR_MER;
	FLASH->CR |= FLASH_CR_STRT;

	wait_for_busy_is_clr();
	FLASH->CR &= ~FLASH_CR_MER;
	return check_eop();
}


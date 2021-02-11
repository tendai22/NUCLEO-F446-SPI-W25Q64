//
//
//
//
//

#include "main.h"

int __io_putchar (int ch)
{
	while ((USART2->SR & USART_SR_TXE) != USART_SR_TXE)
		;
	USART2->DR = ch;
	return ch;
}

int __io_getchar (void)
{
	while (!(USART2->SR & USART_SR_RXNE))
		;
	return USART2->DR;
}

int getchar_pol(void)
{
	if (USART2->SR & USART_SR_RXNE) {
		return USART2->DR;
	} else {
		return -1;
	}
}

/* XMODEM serial I/O
 * the prototypes of the input/output functions are:
 * int _inbyte(unsigned short timeout); // msec timeout
 * void _outbyte(int c);
 */

int _inbyte (int timeout)
{
	long tm = timeout * 4000;
	while (tm-- > 0) {
		if ((USART2->SR & USART_SR_RXNE) != 0) {
			return USART2->DR;
		}
		//for (int volatile i = 0; i < 1; ++i);
	}
	return -1;
}

void _outbyte (int c)
{
	__io_putchar (c);
}

void myDelay (int counts)
{
	long ctr = counts * 10;
	while (ctr-- > 0);
}

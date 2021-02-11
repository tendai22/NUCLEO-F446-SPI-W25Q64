/*
 * monitor.c
 *
 *  Created on: 2020/11/18
 *      Author: tenda_000
 */

/*
 * monitor.c ...
 */

#include <stdio.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>

//#include "z80.h"
//#include "z80onSTM32.h"
#include "monitor.h"
//#include "F8onSTM32.h"

//
// external function declarations
//
int xmodemReceive (uint8_t *dest, int destsz);
int getchar_pol (void);

//
// intel hex format reader
//
int unget_buf = -1;

static int getch (void)
{
	uint8_t c;
	if (unget_buf >= 0) {
		c = unget_buf;
		unget_buf = -1;
		return c;
	}
	//putchar ('X');fflush(stdout);
	c = getchar();
	//printf ("[%02x]", c);fflush(stdout);
	//putchar (c); fflush(stdout);
	return c;
}

static int ungetch (uint8_t c)
{
  unget_buf = c;
  return c;
}

static int debug_switch = 0;

static void debug (const char *format, ...)
{
  if (!debug_switch)
    return;
  va_list ap;
  va_start (ap, format);
  vfprintf (stdout, format, ap);
  fflush (stdout);
  va_end (ap);
}

static int c2h (uint8_t c)
{
	//printf ("<%02x>", c); fflush (stdout);
  if ('0' <= c && c <= '9')
    return c - '0';
  if ('A' <= c && c <= 'F')
    return c - 'A' + 10;
  if ('a' <= c && c <= 'f')
    return c - 'a' + 10;
  return -1;
}

static int readhex (void)
{
  int c, d;
  d = 0;
  if ((c = c2h (getch())) < 0)
    return -1;
  d = c << 4;
  if ((c = c2h (getch())) < 0)
    return -1;
  d += c;
  //printf ("readhex: %02x\r\n", d);
  return d;
}

static uint16_t hex_start = 0xffff;
static uint16_t hex_end = 0;

int do_hex_format (uint8_t *sram, int endram)
{
  int count, address, type, c;
  uint8_t sum = 0;
  //uint8_t cc = 0;

  if (getch() != ':') {
    return -1;
  }
  printf (">"); fflush(stdout);

  if ((count = readhex ()) < 0)
    return -1;
  debug ("count = %2d\r\n", count);
  sum += count;
  if ((c = readhex ()) < 0)
    return -1;
  address = (c << 8);
  sum += c;
  if ((c = readhex ()) < 0)
    return -1;
  address += c;
  debug ("address = %04X\r\n", address);

  printf ("%04X %02d\r", address, count);

  if (address > endram)
    return -1;
  if (address + count - 1 > endram)
    return -1;
  sum += c;
  if ((type = readhex ()) < 0)
    return -1;
  sum += type;
  debug ("type = %02X\r\n", type);
  if (type != 0x0 && type != 0x01)
    return -1;	// unexpected type

  uint16_t ptr = address;
  while (ptr != endram && ptr < address + count) {
    if ((c = readhex ()) < 0)
      return -1;
    sram[ptr++] = c;
    sum += c;
    debug ("%02X ", c);
  }
  if ((c = readhex ()) < 0)
    return -1;
  sum += c;
  debug ("sum = %02X, c = %02X\r\n", sum, c);
  if (type == 0x01)
	return 0;
  if (sum != 0)
    return -1;
  while ((c = getch()) != '\n' && c != '\r') {
    putchar (c); fflush(stdout);
  }
  if (address < hex_start)
    hex_start = address;
  if (ptr - 1 > hex_end)
    hex_end = ptr - 1;
  return ptr - address;
}

int readline (uint8_t *buf, int len)
{

	uint8_t *p = buf;
	int c;
	while (p < (buf + len - 1) && (c = getch ()) != '\r' && c != 'n') {
		//printf ("%02X", c);
		if (c == 0x08 || c == 0x7f) {
			if (p > buf) {
				printf ("\x08 \x08");
				--p;
			}
		} else {
			putchar (c);
			*p++ = c;
		}
	}
	*p = '\0';
	return p - buf;
}
/*
 * dump ... hex dump in sram buffer
 */
static void dump (const uint8_t *sram, uint16_t start, uint16_t end)
{
  int i = 0;
  uint16_t ptr = start;
  int flag = 0;
  for (i = 0; i < 16; ++i)
	  if (sram[ptr + i])
		  flag = 1;
  if (flag == 0)
	  return;
  printf ("%04X ", start);
  for (i = 0; ptr < end && i < 16 ; ++i, ++ptr)
    printf ("%02X ", sram[ptr]);
  while (i++ < 16)
    printf ("   ");
  printf (" ");
  ptr = start;
  for (i = 0; ptr < end && i < 16 ; ++i, ++ptr)
    printf ("%c", isprint(sram[ptr]) ? sram[ptr] : '.');
  printf ("\r\n");
}

int parse_args (uint8_t *line, int *argc, uint8_t **argv, int maxargs)
{
	//printf ("parse_args: %s\r\n", line);
	int nargs;
	if (argc == 0)
		return 0;
	if (argv == 0) {
		*argc = 0;
		return 0;
	}
	if (line == 0 || *line == 0) {
		*argc = 0;
		return 0;
	}
	uint8_t *p = line, *p0;
	nargs = 0;
	while (*p && nargs < maxargs - 1) {
		while (*p && isspace (*p))
			p++;
		if (*p == '\0') {
			break;
		}
		p0 = p;
		while (*p && !isspace (*p))
			p++;
		if (*p)
			*p++ = '\0';
		argv[nargs] = p0;
		//printf ("argv[%d] %s\r\n", nargs, argv[nargs]);
		nargs++;
	}
	if (nargs < maxargs)
		argv[nargs] = NULL;
	*argc = nargs;
	//printf ("argc = %d\r\n", *argc);
	//for (int i = 0; i < *argc; ++i) {
	//	printf ("<%s>", argv[i]);
	//}
	//printf ("\r\n");
	return nargs;
}

void do_dump (uint32_t saddr, uint32_t eaddr, int mod, uint8_t *sram)
{
	printf ("do_dump: %lX, %lX, %d\r\n", saddr, eaddr, mod);
	//
	// saddr, eaddr in 0000-FFFF, refer sram[], otherwise refer STM32 address space
	//
	uint32_t p = saddr, e = eaddr;
	//
	// hexdump
	//
	uint32_t p0 = p;
	while (p < e) {
		if (p == p0 || (p & 0xf) == 0) {
			p0 = p;
			if (p < 0x10000) {
				printf ("%04X ", (uint16_t)p);
			} else {
				printf ("%lX ", (uint32_t)p);
			}
		}
		switch (mod) {
		case 1:
			printf ("%02X ", *((uint8_t *)(p < 0x10000 ? sram + p : (uint8_t *)p))); p++; break;
		case 2:
			printf ("%04X ", *((uint16_t *)(p < 0x10000 ? sram + p : (uint8_t *)p))); p += 2; break;
		case 4:
			printf ("%08lX ", *((uint32_t  *)(p < 0x10000 ? sram + p : (uint8_t *)p))); p += 4; break;
		}
		if (p & 0xf)
			continue;
		p = p0;
		for (int i = 0; i < 16; ++i, ++p) {
			uint8_t *q = (uint8_t *)(p < 0x10000 ? sram + p : (uint8_t *)p);
			printf ("%c", ((' ' <= *q && *q < 0x7f) ? *q : '.'));
		}
		printf ("\r\n");
	}
	if (p & 0xf) {
		for (int i = 0; i < 16; ++i, ++p) {
			uint8_t *q = (uint8_t *)(p < 0x10000 ? sram + p : (uint8_t *)p);
			printf ("%c", ((' ' <= *q && *q < 0x7f) ? *q : '.'));
		}
		printf ("\r\n");
	}
}

uint32_t do_setcmd (uint32_t saddr, int mod, uint8_t *sram)
{
	uint8_t buf[16];
	int c;
	printf ("do_setcmd: %lX, %d\r\n", saddr, mod);
	saddr &= ~(mod - 1);
	while (1) {
		printf ("%04lX ", saddr);
		uint32_t val;
		switch (mod) {
		case 1: printf ("%02X ", *((uint8_t *)(saddr < 0x10000 ? sram + saddr : (uint8_t *)saddr))); break;
		case 2: printf ("%04X ", *((uint16_t *)(saddr < 0x10000 ? sram + saddr : (uint8_t *)saddr))); break;
		case 4: printf ("%08lX ", *((uint32_t *)(saddr < 0x10000 ? sram + saddr : (uint8_t *)saddr))); break;
		}
		if ((c = getch ()) == '.') {
			putchar (c);
			break;
		} else if (c == '\r' || c == '\n') {
			printf ("\r\n");
			saddr++;
			continue;
		}
		ungetch (c);
		readline (buf, sizeof buf - 1);
		val = strtoul ((char *)buf, NULL, 16);
		switch (mod) {
		case 1: *((uint8_t *)(saddr < 0x10000 ? sram + saddr : (uint8_t *)saddr)) = val; break;
		case 2: *((uint16_t *)(saddr < 0x10000 ? sram + saddr : (uint8_t *)saddr)) = val; break;
		case 4: *((uint32_t *)(saddr < 0x10000 ? sram + saddr : (uint8_t *)saddr)) = val; break;
		}
		saddr += mod;
		printf ("\r\n");
	}
	return saddr;
}

static void do_load (uint32_t saddr, int maxlen, uint8_t *sram)
{
	int len;
	uint8_t *top;
	top = (saddr < 0x10000) ? sram + saddr : (uint8_t *)saddr;
	printf ("downLoad on %04X, start XMODEM\r\n", (uint16_t)saddr);
	len = xmodemReceive (top, maxlen);
	printf ("download %d(0x%x) bytes\r\n", len, len);
}

static void strtoupper (char *dest, const char *src, int len)
{
	char *end = dest + len - 1;
	for (; *src; src++, dest++) {
		if (dest >= end) {
			*end = '\0';
			return;
		}
		*dest = toupper (*src);
	}
	*dest = '\0';
	return;
}

static int strnccmp (const char *s1, const char *s2)
{
	char buf1[80], buf2[80];
	strtoupper (buf1, s1, sizeof buf1);
	strtoupper (buf2, s2, sizeof buf2);
	return strcmp (buf1, buf2);
}

/*
 * monitor commands
 * D
 * D <src>
 * D <src> <end>
 * dump memory, from <s> to <e>, or 128bytes
 *
 * S
 * S <addr>
 * set a byte to memory, a period('.') stops to input.
 *
 * G
 * G <addr>
 * jump to <addr>
 *
 * L ... load code segment
 * L <num>
 * load code segment, if no argument, last number of the code is selected
 */

#define BADADDR 0xffffffff

static int modifier = 1;	// 1: byte, 2: short, 4: long
int get_modifier (const uint8_t *s)
{
	if (*s == '.')
		s++;
	switch (toupper(*s)) {
	case 'B': modifier = 1; return modifier;
	case 'W': modifier = 2; return modifier;
	case 'L': modifier = 4; return modifier;
	default: return modifier;
	}
}

static uint32_t nextaddr = 0;
static uint32_t prevaddr = 0;

int do_command (int ac, uint8_t **av, uint8_t *sram, struct codeinfo *codes)
{
	if (ac == 0 || av == 0 || *av == 0 || *av[0] == 0)
		return 0;
	uint8_t c = *av[0], *p = av[0];
	uint32_t saddr, eaddr = BADADDR;
	int mod, index;
	static int last_index = 0;

	// multicharacter commands
	if (strnccmp ((const char *)(av[0]), "load") == 0) {
		if (ac == 0) {
			saddr = prevaddr;
		}
		if (ac > 1) {
			if ((saddr = strtoul ((char *)av[1], NULL, 16)) == BADADDR)
				saddr = nextaddr;
		}
		if (saddr == BADADDR) {
			printf ("bad addr %s\r\n", av[1]);
			return -1;
		}
		do_load (saddr, 0x10000, sram);
		return 0;
	}
	// singlecharacter commands
	switch (toupper (c)) {
	case 'D':
		mod = get_modifier (p + 1);
		if (ac == 1) {
			saddr = nextaddr;
		}
		if (ac > 1) {
			if ((saddr = strtoul ((char *)av[1], NULL, 16)) == BADADDR)
				saddr = nextaddr;
		}
		if (saddr == BADADDR) {
			printf ("bad addr %s\r\n", av[1]);
			return -1;
		}
		// round saddr
		saddr = saddr / mod * mod;
		// eaddr
		eaddr = ac > 2 ? strtoul ((char *)av[2], NULL, 16) : BADADDR;
		if (eaddr == BADADDR)
			eaddr = 128;
		nextaddr = saddr + eaddr;
		do_dump (saddr, saddr + eaddr, mod, sram);
		break;
	case 'S':
		mod = get_modifier (p + 1);
		if (ac == 1) {
			saddr = nextaddr;
		}
		if (ac > 1) {
			if ((saddr = strtoul ((char *)av[1], NULL, 16)) == BADADDR)
				saddr = nextaddr;
		}
		if (saddr == BADADDR) {
			printf ("bad addr %s\r\n", av[1]);
			return -1;
		}
		nextaddr = do_setcmd (saddr, mod, sram);
		printf ("\r\n");
		break;
	case 'L':
		if (ac == 1) {
			index = last_index;
		}
		if (ac > 1) {
			if ((index = strtoul ((char *)av[1], NULL, 16)) == BADADDR) {
				index = last_index;
			}
		}
		int i, max_index;
		struct codeinfo *cp = codes;
		if (codes == 0) {
			printf ("no code specified, not set\r\n");
			return -1;
		}
		for (i = 0; cp && cp->code ; ++i, ++cp)
			;
		max_index = i;
		if (!(0 <= index && index < max_index)) {
			printf ("bad index %s\r\n", av[1]);
			return -1;
		}
		// copy code
		cp = &codes[index];
		uint8_t *src, *dst, *dend;
		src = cp->code;
		dst = &sram[cp->start];
		dend = dst + cp->length;
		if (dend >= sram + 0x10000) {
			dend = sram + 0x10000;
		}
		while (dst < dend) {
			*dst++ = *src++;
		}
		break;
	}
	return 0;
}

/*
 * monitor ... read chars to parse and execute corresponting command.
 */
int monitor (uint8_t *sram, struct codeinfo *codes)
{
	static uint8_t linbuf[80];
	uint8_t *av[6];
	int ac;
	int c;
	int run_mode = 0;

	do {
		if (run_mode == 0) {
			printf ("> ");
		}
		c = getchar ();
		if (c != ' ') {
			if (run_mode == 0) {
				printf ("> ");
			}
			run_mode = 0;
		}
		if (c == 'i') {
			putchar (c);
			// generate INT (i8251 RxRdy emulation)
			//generate_i8251int ();		// GPIOA->ODR |= INT_Pin;
		} else if (c == 'g' || c == ' ') {
			if (run_mode == 0) {
				putchar (c);
				printf ("\r\n");
				run_mode = 1;
			}
			// start CPU, nonstop mode
			// SPC ... singlestep, g ... run (nonstop)
			while (getchar_pol() > 0);	// read out unread chars
			//runF3850_EXTI ((c == ' ') ? 1 : 0, continue_flag, &context);
		} else if (c == ':') {
			// intel hex format read
			uint32_t total;
			int n;
			ungetch (c);
			total = 0;
			while ((n = do_hex_format (sram, 0x10000 - 1)) > 0)
				total += n;
			for (uint16_t ptr = hex_start; ptr <= hex_end; ptr += 16)
				dump (sram, ptr, hex_end - 1);
			printf ("total = %ld\r\n", total);
		} else {
			// gather chars in a line
			ungetch (c);
			readline (linbuf, sizeof linbuf);
			printf ("\r\n");
			//printf ("lin: %s\r\n", linbuf);
			parse_args (linbuf, &ac, av, 6);
			do_command (ac, av, sram, codes);
		}
	} while (1);
	return c;
}

#ifdef TEST_MONITOR
int main (int ac, char **av)
{
	static uint8_t sram[65536];
	setbuf (stdout, NULL);
	setbuf (stdin, NULL);
	system ("stty raw -echo");
	monitor(sram);
	system ("stty sane");
}
#endif



/*
 * monitor.h
 *
 *  Created on: Oct 19, 2020
 *      Author: tenda_000
 */

#ifndef SRC_MONITOR_H_
#define SRC_MONITOR_H_

struct codeinfo {
	uint8_t *code;
	uint16_t start;
	int length;
};

extern struct codeinfo codes[];

extern int monitor (uint8_t *sram, struct codeinfo *codes);

#endif /* SRC_MONITOR_H_ */

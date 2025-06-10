/*
 * lwf - Wireless Information Library - Utility Headers
 *
 *   Copyright (C) 2010 Jo-Philipp Wich <xm@subsignal.org>
 *
 * The lwf library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * The lwf library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the lwf library. If not, see http://www.gnu.org/licenses/.
 */

#ifndef __LWF_UTILS_H_
#define __LWF_UTILS_H_

#include <sys/socket.h>
#include <net/if.h>

#include "lwf.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#define LOG10_MAGIC	1.25892541179

int lwf_ioctl(int cmd, void *ifr);

int lwf_dbm2mw(int in);
int lwf_mw2dbm(int in);
static inline int lwf_mbm2dbm(int gain)
{
	return gain / 100;
}

int lwf_ifup(const char *ifname);
int lwf_ifdown(const char *ifname);
int lwf_ifmac(const char *ifname);

void lwf_close(void);

struct lwf_hardware_entry * lwf_hardware(struct lwf_hardware_id *id);

int lwf_hardware_id_from_mtd(struct lwf_hardware_id *id);

void lwf_parse_rsn(struct lwf_crypto_entry *c, uint8_t *data, uint8_t len,
					  uint8_t defcipher, uint8_t defauth);

#endif

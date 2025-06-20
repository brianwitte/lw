/*
 * lwf - Wireless Information Library - NL80211 Headers
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

#ifndef __LWF_NL80211_H_
#define __LWF_NL80211_H_

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <sys/un.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>

#include "lwf.h"
#include "lwf/utils.h"
#include "api/nl80211.h"

struct nl80211_state {
	struct nl_sock *nl_sock;
	struct nl_cache *nl_cache;
	struct genl_family *nl80211;
	struct genl_family *nlctrl;
};

struct nl80211_msg_conveyor {
	struct nl_msg *msg;
	struct nl_cb *cb;
};

struct nl80211_event_conveyor {
	uint32_t wait[(NL80211_CMD_MAX / 32) + !!(NL80211_CMD_MAX % 32)];
	int recv;
};

struct nl80211_group_conveyor {
	const char *name;
	int id;
};

struct nl80211_rssi_rate {
	int16_t rate;
	int rate_samples;
	int8_t  rssi;
	int rssi_samples;
};

struct nl80211_array_buf {
	void *buf;
	int count;
};

#endif

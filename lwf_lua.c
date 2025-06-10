/*
 * lwf - Wireless Information Library - Lua Bindings
 *
 *   Copyright (C) 2009 Jo-Philipp Wich <xm@subsignal.org>
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

#include "lwf/lua.h"
#include <stdbool.h>

/* Determine type */
static int lwf_L_type(lua_State *L)
{
	const char *ifname = luaL_checkstring(L, 1);
	const char *type = lwf_type(ifname);

	if (type)
		lua_pushstring(L, type);
	else
		lua_pushnil(L);

	return 1;
}

/* Shutdown backends */
static int lwf_L__gc(lua_State *L)
{
	lwf_finish();
	return 0;
}

/*
 * Build a short textual description of the crypto info
 */

static char * lwf_crypto_print_ciphers(int ciphers)
{
	static char str[128] = { 0 };
	char *pos = str;

	if (ciphers & LWF_CIPHER_WEP40)
		pos += sprintf(pos, "WEP-40, ");

	if (ciphers & LWF_CIPHER_WEP104)
		pos += sprintf(pos, "WEP-104, ");

	if (ciphers & LWF_CIPHER_TKIP)
		pos += sprintf(pos, "TKIP, ");

	if (ciphers & LWF_CIPHER_CCMP)
		pos += sprintf(pos, "CCMP, ");

	if (ciphers & LWF_CIPHER_WRAP)
		pos += sprintf(pos, "WRAP, ");

	if (ciphers & LWF_CIPHER_AESOCB)
		pos += sprintf(pos, "AES-OCB, ");

	if (ciphers & LWF_CIPHER_CKIP)
		pos += sprintf(pos, "CKIP, ");

	if (!ciphers || (ciphers & LWF_CIPHER_NONE))
		pos += sprintf(pos, "NONE, ");

	*(pos - 2) = 0;

	return str;
}

static char * lwf_crypto_print_suites(int suites)
{
	static char str[64] = { 0 };
	char *pos = str;

	if (suites & LWF_KMGMT_PSK)
		pos += sprintf(pos, "PSK/");

	if (suites & LWF_KMGMT_8021x)
		pos += sprintf(pos, "802.1X/");

	if (!suites || (suites & LWF_KMGMT_NONE))
		pos += sprintf(pos, "NONE/");

	*(pos - 1) = 0;

	return str;
}

static char * lwf_crypto_desc(struct lwf_crypto_entry *c)
{
	static char desc[512] = { 0 };

	if (c)
	{
		if (c->enabled)
		{
			/* WEP */
			if (c->auth_algs && !c->wpa_version)
			{
				if ((c->auth_algs & LWF_AUTH_OPEN) &&
				    (c->auth_algs & LWF_AUTH_SHARED))
				{
					sprintf(desc, "WEP Open/Shared (%s)",
						lwf_crypto_print_ciphers(c->pair_ciphers));
				}
				else if (c->auth_algs & LWF_AUTH_OPEN)
				{
					sprintf(desc, "WEP Open System (%s)",
						lwf_crypto_print_ciphers(c->pair_ciphers));
				}
				else if (c->auth_algs & LWF_AUTH_SHARED)
				{
					sprintf(desc, "WEP Shared Auth (%s)",
						lwf_crypto_print_ciphers(c->pair_ciphers));
				}
			}

			/* WPA */
			else if (c->wpa_version)
			{
				switch (c->wpa_version) {
					case 3:
						sprintf(desc, "mixed WPA/WPA2 %s (%s)",
							lwf_crypto_print_suites(c->auth_suites),
							lwf_crypto_print_ciphers(
								c->pair_ciphers | c->group_ciphers));
						break;

					case 2:
						sprintf(desc, "WPA2 %s (%s)",
							lwf_crypto_print_suites(c->auth_suites),
							lwf_crypto_print_ciphers(
								c->pair_ciphers | c->group_ciphers));
						break;

					case 1:
						sprintf(desc, "WPA %s (%s)",
							lwf_crypto_print_suites(c->auth_suites),
							lwf_crypto_print_ciphers(
								c->pair_ciphers | c->group_ciphers));
						break;
				}
			}
			else
			{
				sprintf(desc, "None");
			}
		}
		else
		{
			sprintf(desc, "None");
		}
	}
	else
	{
		sprintf(desc, "Unknown");
	}

	return desc;
}

/* Build Lua table from crypto data */
static void lwf_L_cryptotable(lua_State *L, struct lwf_crypto_entry *c)
{
	int i, j;

	lua_newtable(L);

	lua_pushboolean(L, c->enabled);
	lua_setfield(L, -2, "enabled");

	lua_pushstring(L, lwf_crypto_desc(c));
	lua_setfield(L, -2, "description");

	lua_pushboolean(L, (c->enabled && !c->wpa_version));
	lua_setfield(L, -2, "wep");

	lua_pushinteger(L, c->wpa_version);
	lua_setfield(L, -2, "wpa");

	lua_newtable(L);
	for (i = 0, j = 1; i < ARRAY_SIZE(LWF_CIPHER_NAMES); i++)
	{
		if (c->pair_ciphers & (1 << i))
		{
			lua_pushstring(L, LWF_CIPHER_NAMES[i]);
			lua_rawseti(L, -2, j++);
		}
	}
	lua_setfield(L, -2, "pair_ciphers");

	lua_newtable(L);
	for (i = 0, j = 1; i < ARRAY_SIZE(LWF_CIPHER_NAMES); i++)
	{
		if (c->group_ciphers & (1 << i))
		{
			lua_pushstring(L, LWF_CIPHER_NAMES[i]);
			lua_rawseti(L, -2, j++);
		}
	}
	lua_setfield(L, -2, "group_ciphers");

	lua_newtable(L);
	for (i = 0, j = 1; i < ARRAY_SIZE(LWF_KMGMT_NAMES); i++)
	{
		if (c->auth_suites & (1 << i))
		{
			lua_pushstring(L, LWF_KMGMT_NAMES[i]);
			lua_rawseti(L, -2, j++);
		}
	}
	lua_setfield(L, -2, "auth_suites");

	lua_newtable(L);
	for (i = 0, j = 1; i < ARRAY_SIZE(LWF_AUTH_NAMES); i++)
	{
		if (c->auth_algs & (1 << i))
		{
			lua_pushstring(L, LWF_AUTH_NAMES[i]);
			lua_rawseti(L, -2, j++);
		}
	}
	lua_setfield(L, -2, "auth_algs");
}


/* Wrapper for mode */
static int lwf_L_mode(lua_State *L, int (*func)(const char *, int *))
{
	int mode;
	const char *ifname = luaL_checkstring(L, 1);

	if ((*func)(ifname, &mode))
		mode = LWF_OPMODE_UNKNOWN;

	lua_pushstring(L, LWF_OPMODE_NAMES[mode]);
	return 1;
}

static void set_rateinfo(lua_State *L, struct lwf_rate_entry *r, bool rx)
{
	lua_pushnumber(L, r->rate);
	lua_setfield(L, -2, rx ? "rx_rate" : "tx_rate");

	lua_pushboolean(L, r->is_ht);
	lua_setfield(L, -2, rx ? "rx_ht" : "tx_ht");

	lua_pushboolean(L, r->is_vht);
	lua_setfield(L, -2, rx ? "rx_vht" : "tx_vht");

	lua_pushnumber(L, r->mhz);
	lua_setfield(L, -2, rx ? "rx_mhz" : "tx_mhz");

	if (r->is_ht)
	{
		lua_pushboolean(L, r->is_40mhz);
		lua_setfield(L, -2, rx ? "rx_40mhz" : "tx_40mhz");

		lua_pushnumber(L, r->mcs);
		lua_setfield(L, -2, rx ? "rx_mcs" : "tx_mcs");

		lua_pushboolean(L, r->is_short_gi);
		lua_setfield(L, -2, rx ? "rx_short_gi" : "tx_short_gi");
	}
	else if (r->is_vht)
	{
		lua_pushnumber(L, r->mcs);
		lua_setfield(L, -2, rx ? "rx_mcs" : "tx_mcs");

		lua_pushnumber(L, r->nss);
		lua_setfield(L, -2, rx ? "rx_nss" : "tx_nss");

		lua_pushboolean(L, r->is_short_gi);
		lua_setfield(L, -2, rx ? "rx_short_gi" : "tx_short_gi");
	}
}

/* Wrapper for assoclist */
static int lwf_L_assoclist(lua_State *L, int (*func)(const char *, char *, int *))
{
	int i, len;
	char rv[LWF_BUFSIZE];
	char macstr[18];
	const char *ifname = luaL_checkstring(L, 1);
	struct lwf_assoclist_entry *e;

	lua_newtable(L);
	memset(rv, 0, sizeof(rv));

	if (!(*func)(ifname, rv, &len))
	{
		for (i = 0; i < len; i += sizeof(struct lwf_assoclist_entry))
		{
			e = (struct lwf_assoclist_entry *) &rv[i];

			sprintf(macstr, "%02X:%02X:%02X:%02X:%02X:%02X",
				e->mac[0], e->mac[1], e->mac[2],
				e->mac[3], e->mac[4], e->mac[5]);

			lua_newtable(L);

			lua_pushnumber(L, e->signal);
			lua_setfield(L, -2, "signal");

			lua_pushnumber(L, e->noise);
			lua_setfield(L, -2, "noise");

			lua_pushnumber(L, e->inactive);
			lua_setfield(L, -2, "inactive");

			lua_pushnumber(L, e->rx_packets);
			lua_setfield(L, -2, "rx_packets");

			lua_pushnumber(L, e->tx_packets);
			lua_setfield(L, -2, "tx_packets");

			set_rateinfo(L, &e->rx_rate, true);
			set_rateinfo(L, &e->tx_rate, false);

			if (e->thr) {
				lua_pushnumber(L, e->thr);
				lua_setfield(L, -2, "expected_throughput");
			}

			lua_setfield(L, -2, macstr);
		}
	}

	return 1;
}

/* Wrapper for tx power list */
static int lwf_L_txpwrlist(lua_State *L, int (*func)(const char *, char *, int *))
{
	int i, x, len;
	char rv[LWF_BUFSIZE];
	const char *ifname = luaL_checkstring(L, 1);
	struct lwf_txpwrlist_entry *e;

	memset(rv, 0, sizeof(rv));

	if (!(*func)(ifname, rv, &len))
	{
		lua_newtable(L);

		for (i = 0, x = 1; i < len; i += sizeof(struct lwf_txpwrlist_entry), x++)
		{
			e = (struct lwf_txpwrlist_entry *) &rv[i];

			lua_newtable(L);

			lua_pushnumber(L, e->mw);
			lua_setfield(L, -2, "mw");

			lua_pushnumber(L, e->dbm);
			lua_setfield(L, -2, "dbm");

			lua_rawseti(L, -2, x);
		}

		return 1;
	}

	return 0;
}

/* Wrapper for scan list */
static int lwf_L_scanlist(lua_State *L, int (*func)(const char *, char *, int *))
{
	int i, x, len = 0;
	char rv[LWF_BUFSIZE];
	char macstr[18];
	const char *ifname = luaL_checkstring(L, 1);
	struct lwf_scanlist_entry *e;

	lua_newtable(L);
	memset(rv, 0, sizeof(rv));

	if (!(*func)(ifname, rv, &len))
	{
		for (i = 0, x = 1; i < len; i += sizeof(struct lwf_scanlist_entry), x++)
		{
			e = (struct lwf_scanlist_entry *) &rv[i];

			lua_newtable(L);

			/* BSSID */
			sprintf(macstr, "%02X:%02X:%02X:%02X:%02X:%02X",
				e->mac[0], e->mac[1], e->mac[2],
				e->mac[3], e->mac[4], e->mac[5]);

			lua_pushstring(L, macstr);
			lua_setfield(L, -2, "bssid");

			/* ESSID */
			if (e->ssid[0])
			{
				lua_pushstring(L, (char *) e->ssid);
				lua_setfield(L, -2, "ssid");
			}

			/* Channel */
			lua_pushinteger(L, e->channel);
			lua_setfield(L, -2, "channel");

			/* Mode */
			lua_pushstring(L, LWF_OPMODE_NAMES[e->mode]);
			lua_setfield(L, -2, "mode");

			/* Quality, Signal */
			lua_pushinteger(L, e->quality);
			lua_setfield(L, -2, "quality");

			lua_pushinteger(L, e->quality_max);
			lua_setfield(L, -2, "quality_max");

			lua_pushnumber(L, (e->signal - 0x100));
			lua_setfield(L, -2, "signal");

			/* Crypto */
			lwf_L_cryptotable(L, &e->crypto);
			lua_setfield(L, -2, "encryption");

			lua_rawseti(L, -2, x);
		}
	}

	return 1;
}

/* Wrapper for frequency list */
static int lwf_L_freqlist(lua_State *L, int (*func)(const char *, char *, int *))
{
	int i, x, len;
	char rv[LWF_BUFSIZE];
	const char *ifname = luaL_checkstring(L, 1);
	struct lwf_freqlist_entry *e;

	lua_newtable(L);
	memset(rv, 0, sizeof(rv));

	if (!(*func)(ifname, rv, &len))
	{
		for (i = 0, x = 1; i < len; i += sizeof(struct lwf_freqlist_entry), x++)
		{
			e = (struct lwf_freqlist_entry *) &rv[i];

			lua_newtable(L);

			/* MHz */
			lua_pushinteger(L, e->mhz);
			lua_setfield(L, -2, "mhz");

			/* Channel */
			lua_pushinteger(L, e->channel);
			lua_setfield(L, -2, "channel");

			/* Restricted (DFS/TPC/Radar) */
			lua_pushboolean(L, e->restricted);
			lua_setfield(L, -2, "restricted");

			lua_rawseti(L, -2, x);
		}
	}

	return 1;
}

/* Wrapper for crypto settings */
static int lwf_L_encryption(lua_State *L, int (*func)(const char *, char *))
{
	const char *ifname = luaL_checkstring(L, 1);
	struct lwf_crypto_entry c = { 0 };

	if (!(*func)(ifname, (char *)&c))
	{
		lwf_L_cryptotable(L, &c);
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

/* Wrapper for hwmode list */
static int lwf_L_hwmodelist(lua_State *L, int (*func)(const char *, int *))
{
	const char *ifname = luaL_checkstring(L, 1);
	int hwmodes = 0;

	if (!(*func)(ifname, &hwmodes))
	{
		lua_newtable(L);

		lua_pushboolean(L, hwmodes & LWF_80211_A);
		lua_setfield(L, -2, "a");

		lua_pushboolean(L, hwmodes & LWF_80211_B);
		lua_setfield(L, -2, "b");

		lua_pushboolean(L, hwmodes & LWF_80211_G);
		lua_setfield(L, -2, "g");

		lua_pushboolean(L, hwmodes & LWF_80211_N);
		lua_setfield(L, -2, "n");

		lua_pushboolean(L, hwmodes & LWF_80211_AC);
		lua_setfield(L, -2, "ac");

		return 1;
	}

	lua_pushnil(L);
	return 1;
}

/* Wrapper for htmode list */
static int lwf_L_htmodelist(lua_State *L, int (*func)(const char *, int *))
{
	const char *ifname = luaL_checkstring(L, 1);
	int i, htmodes = 0;

	if (!(*func)(ifname, &htmodes))
	{
		lua_newtable(L);

		for (i = 0; i < ARRAY_SIZE(LWF_HTMODE_NAMES); i++)
		{
			lua_pushboolean(L, htmodes & (1 << i));
			lua_setfield(L, -2, LWF_HTMODE_NAMES[i]);
		}

		return 1;
	}

	lua_pushnil(L);
	return 1;
}

/* Wrapper for mbssid_support */
static int lwf_L_mbssid_support(lua_State *L, int (*func)(const char *, int *))
{
	const char *ifname = luaL_checkstring(L, 1);
	int support = 0;

	if (!(*func)(ifname, &support))
	{
		lua_pushboolean(L, support);
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

/* Wrapper for hardware_id */
static int lwf_L_hardware_id(lua_State *L, int (*func)(const char *, char *))
{
	const char *ifname = luaL_checkstring(L, 1);
	struct lwf_hardware_id ids;

	if (!(*func)(ifname, (char *)&ids))
	{
		lua_newtable(L);

		lua_pushnumber(L, ids.vendor_id);
		lua_setfield(L, -2, "vendor_id");

		lua_pushnumber(L, ids.device_id);
		lua_setfield(L, -2, "device_id");

		lua_pushnumber(L, ids.subsystem_vendor_id);
		lua_setfield(L, -2, "subsystem_vendor_id");

		lua_pushnumber(L, ids.subsystem_device_id);
		lua_setfield(L, -2, "subsystem_device_id");
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

/* Wrapper for country list */
static char * lwf_L_country_lookup(char *buf, int len, int iso3166)
{
	int i;
	struct lwf_country_entry *c;

	for (i = 0; i < len; i += sizeof(struct lwf_country_entry))
	{
		c = (struct lwf_country_entry *) &buf[i];

		if (c->iso3166 == iso3166)
			return c->ccode;
	}

	return NULL;
}

static int lwf_L_countrylist(lua_State *L, int (*func)(const char *, char *, int *))
{
	int len, i;
	char rv[LWF_BUFSIZE], alpha2[3];
	char *ccode;
	const char *ifname = luaL_checkstring(L, 1);
	const struct lwf_iso3166_label *l;

	lua_newtable(L);
	memset(rv, 0, sizeof(rv));

	if (!(*func)(ifname, rv, &len))
	{
		for (l = LWF_ISO3166_NAMES, i = 1; l->iso3166; l++)
		{
			if ((ccode = lwf_L_country_lookup(rv, len, l->iso3166)) != NULL)
			{
				sprintf(alpha2, "%c%c",
					(l->iso3166 / 256), (l->iso3166 % 256));

				lua_newtable(L);

				lua_pushstring(L, alpha2);
				lua_setfield(L, -2, "alpha2");

				lua_pushstring(L, ccode);
				lua_setfield(L, -2, "ccode");

				lua_pushstring(L, l->name);
				lua_setfield(L, -2, "name");

				lua_rawseti(L, -2, i++);
			}
		}
	}

	return 1;
}

#ifdef USE_NL80211
/* NL80211 */
LUA_WRAP_INT_OP(nl80211,channel)
LUA_WRAP_INT_OP(nl80211,frequency)
LUA_WRAP_INT_OP(nl80211,frequency_offset)
LUA_WRAP_INT_OP(nl80211,txpower)
LUA_WRAP_INT_OP(nl80211,txpower_offset)
LUA_WRAP_INT_OP(nl80211,bitrate)
LUA_WRAP_INT_OP(nl80211,signal)
LUA_WRAP_INT_OP(nl80211,noise)
LUA_WRAP_INT_OP(nl80211,quality)
LUA_WRAP_INT_OP(nl80211,quality_max)
LUA_WRAP_STRING_OP(nl80211,ssid)
LUA_WRAP_STRING_OP(nl80211,bssid)
LUA_WRAP_STRING_OP(nl80211,country)
LUA_WRAP_STRING_OP(nl80211,hardware_name)
LUA_WRAP_STRING_OP(nl80211,phyname)
LUA_WRAP_STRUCT_OP(nl80211,mode)
LUA_WRAP_STRUCT_OP(nl80211,assoclist)
LUA_WRAP_STRUCT_OP(nl80211,txpwrlist)
LUA_WRAP_STRUCT_OP(nl80211,scanlist)
LUA_WRAP_STRUCT_OP(nl80211,freqlist)
LUA_WRAP_STRUCT_OP(nl80211,countrylist)
LUA_WRAP_STRUCT_OP(nl80211,hwmodelist)
LUA_WRAP_STRUCT_OP(nl80211,htmodelist)
LUA_WRAP_STRUCT_OP(nl80211,encryption)
LUA_WRAP_STRUCT_OP(nl80211,mbssid_support)
LUA_WRAP_STRUCT_OP(nl80211,hardware_id)
#endif


#ifdef USE_NL80211
/* NL80211 table */
static const luaL_reg R_nl80211[] = {
	LUA_REG(nl80211,channel),
	LUA_REG(nl80211,frequency),
	LUA_REG(nl80211,frequency_offset),
	LUA_REG(nl80211,txpower),
	LUA_REG(nl80211,txpower_offset),
	LUA_REG(nl80211,bitrate),
	LUA_REG(nl80211,signal),
	LUA_REG(nl80211,noise),
	LUA_REG(nl80211,quality),
	LUA_REG(nl80211,quality_max),
	LUA_REG(nl80211,mode),
	LUA_REG(nl80211,ssid),
	LUA_REG(nl80211,bssid),
	LUA_REG(nl80211,country),
	LUA_REG(nl80211,assoclist),
	LUA_REG(nl80211,txpwrlist),
	LUA_REG(nl80211,scanlist),
	LUA_REG(nl80211,freqlist),
	LUA_REG(nl80211,countrylist),
	LUA_REG(nl80211,hwmodelist),
	LUA_REG(nl80211,htmodelist),
	LUA_REG(nl80211,encryption),
	LUA_REG(nl80211,mbssid_support),
	LUA_REG(nl80211,hardware_id),
	LUA_REG(nl80211,hardware_name),
	LUA_REG(nl80211,phyname),
	{ NULL, NULL }
};
#endif

/* Common */
static const luaL_reg R_common[] = {
	{ "type", lwf_L_type },
	{ "__gc", lwf_L__gc  },
	{ NULL, NULL }
};

LUALIB_API int luaopen_lwf(lua_State *L) {
	luaL_register(L, LWF_META, R_common);


#ifdef USE_NL80211
	luaL_newmetatable(L, LWF_NL80211_META);
	luaL_register(L, NULL, R_common);
	luaL_register(L, NULL, R_nl80211);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_setfield(L, -2, "nl80211");
#endif

	return 1;
}

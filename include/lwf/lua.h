/*
 * lwf - Wireless Information Library - Lua Headers
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

#ifndef __LWF_LUALUB_H_
#define __LWF_LUALIB_H_

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "lwf.h"


#define LWF_META			"lwf"

#ifdef USE_NL80211
#define LWF_NL80211_META	"lwf.nl80211"
#endif


#define LUA_REG(type,op) \
	{ #op, lwf_L_##type##_##op }

#define LUA_WRAP_INT_OP(type,op)						\
	static int lwf_L_##type##_##op(lua_State *L)		\
	{													\
		const char *ifname = luaL_checkstring(L, 1);	\
		int rv;											\
		if( !type##_ops.op(ifname, &rv) )				\
			lua_pushnumber(L, rv);						\
		else											\
			lua_pushnil(L);								\
		return 1;										\
	}

#define LUA_WRAP_STRING_OP(type,op)						\
	static int lwf_L_##type##_##op(lua_State *L)		\
	{													\
		const char *ifname = luaL_checkstring(L, 1);	\
		char rv[LWF_BUFSIZE];						\
		memset(rv, 0, LWF_BUFSIZE);					\
		if( !type##_ops.op(ifname, rv) )				\
			lua_pushstring(L, rv);						\
		else											\
			lua_pushnil(L);								\
		return 1;										\
	}

#define LUA_WRAP_STRUCT_OP(type,op)						\
	static int lwf_L_##type##_##op(lua_State *L)		\
	{													\
		return lwf_L_##op(L, type##_ops.op);			\
	}

#endif

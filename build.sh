#!/usr/bin/env bash

make BACKENDS=nl80211 CFLAGS="-I/usr/include/libnl3 -I/home/bkz/local/luajit-05JUN2024/include/luajit-2.1" LDFLAGS="-lnl-3 -lnl-genl-3" LUA=luajit

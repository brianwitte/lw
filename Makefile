LUA=lua5.1
PREFIX=/usr/local

LWF_CFLAGS      = $(CFLAGS) -std=gnu99 -fstrict-aliasing -Iinclude -I/usr/include/libnl3 -I/usr/include/lua5.1
LWF_LDFLAGS     = $(LDFLAGS) -lnl-3 -lnl-genl-3

LWF_LIB         = liblwf.so
LWF_LIB_LDFLAGS = -shared
LWF_LIB_OBJ     = lwf_utils.o lwf_lib.o

LWF_LUA         = lwf.so
LWF_LUA_LDFLAGS = -shared -L. -llwf -l$(LUA)
LWF_LUA_OBJ     = lwf_lua.o

LWF_CLI         = lwf
LWF_CLI_LDFLAGS = -L. -llwf
LWF_CLI_OBJ     = lwf_cli.o

LWF_CFLAGS  += -DUSE_NL80211
LWF_LIB_OBJ += lwf_nl80211.o

%.o: %.c
	$(CC) $(LWF_CFLAGS) -fPIC -c -o $@ $<

compile: clean $(LWF_LIB_OBJ) $(LWF_CLI_OBJ) $(LWF_LUA_OBJ)
	$(CC) -o $(LWF_LIB) $(LWF_LIB_OBJ) $(LWF_LDFLAGS) $(LWF_LIB_LDFLAGS)
	$(CC) -o $(LWF_CLI) $(LWF_CLI_OBJ) $(LWF_LDFLAGS) $(LWF_CLI_LDFLAGS)
	$(CC) -o $(LWF_LUA) $(LWF_LUA_OBJ) $(LWF_LDFLAGS) $(LWF_LUA_LDFLAGS)

clean:
	rm -f *.o $(LWF_LIB) $(LWF_LUA) $(LWF_CLI)

install: compile
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/lib
	install -m 0755 $(LWF_CLI) $(DESTDIR)$(PREFIX)/bin/
	install -m 0755 $(LWF_LIB) $(DESTDIR)$(PREFIX)/lib/
	install -m 0755 $(LWF_LUA) $(DESTDIR)$(PREFIX)/lib/

.PHONY: compile clean install

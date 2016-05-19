#Easy Makefile

APPPRJNAME = vt
LIBPRJNAME = 
HMASTER    = 
#LIBSO      = -lm -lX11
LIBSO      = -lm
LIBSO	  += $(CL)
LIBA       =
APPIPATH   = /usr/bin/
LIBIPATH   = /usr/lib/
IHPATH     = /usr/include/
APPCFLAGS  = -Wall -Wextra -O3
APPDFLAGS  = -Wall -Wextra -O3
LIBCFLAGS  = -Wall -g 
LIBDFLAGS  = -g
APPDEFINE  = -DXORG -D_APP -D__DEBUG=1 -D__DEBUG_LEVEL=9 -D__DEBUG_TERMINAL -D__ASSERT=1
#APPDEFINE  = $(DC)
LIBDEFINE  = -D_LIB

APPCC      = gcc
APPLK      = gcc
LIBCC      = gcc
LIBLK      = ld
CFILE      = $(shell echo *.c)
HFILE      = $(shell echo *.h)
APPOBJPATH = ./obj/
LIBOBJPATH = ./oobj/
APPOFILE   = $(patsubst %.c,$(APPOBJPATH)%.o,$(CFILE)) 
LIBOFILE   = $(patsubst %.c,$(LIBOBJPATH)%.o,$(CFILE)) 

all:
	@echo use app or lib

app: $(APPPRJNAME)
	@echo end make $(APPPRJNAME)

$(APPPRJNAME): $(APPOFILE)
	$(APPLK) $(APPDFLAGS) -o $(APPPRJNAME) $(APPOFILE) $(LIBA) $(LIBSO)

$(APPOBJPATH)%.o: %.c $(HFILE)
	$(APPCC) $(APPCFLAGS) -c $< -o $@ $(APPDEFINE) 

lib: $(LIBPRJNAME)
	@echo end make $(LIBPRJNAME)

$(LIBPRJNAME): $(LIBOFILE)
	$(LIBLK) $(LIBDFLAGS) -shared -o $(LIBPRJNAME) $(LIBOFILE) $(LIBA) $(LIBSO)

$(LIBOBJPATH)%.o: %.c $(HFILE)
	$(LIBCC) $(LIBCFLAGS) -fpic -c $< -o $@ $(LIBDEFINE)

appclean:
	rm -rf $(APPOBJPATH)*.o
	rm -rf ./$(APPPRJNAME)

libclean:
	rm -rf $(LIBOBJPATH)*.o
	rm -rf ./$(LIBPRJNAME)

appinstall:
	cp ./$(APPPRJNAME) $(APPIPATH)$(APPPRJNAME)

libinstall:
	cp ./$(LIBPRJNAME) $(LIBIPATH)$(LIBPRJNAME)
	cp ./$(HMASTER) $(IHPATH)$(HMASTER)

help:
	@echo "enable debug:"
	@echo "\tDC='-D__DEBUG=1 -D__DEBUG_LEVEL=9 -D__DEBUG_TERMINAL'"
	@echo "enable mouse mode:"
	@echo "\tDC=-DXORG CL=-lX11"

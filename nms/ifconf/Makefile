
BASEDIR = ../
include $(addprefix $(BASEDIR), Makefile.pub)

CFLAGS = $(CCFLAGS)

TESTFLAGS = -D__ZHZ_TEST_IFCONF__
TCFLAGS = -static $(INCLUDE) $(TESTFLAGS)

DDEP = $(addprefix $(LOCALINCDIR),debug.h list.h ifconf.h probe.h)

STUB = $(notdir $(CURDIR))
TAG = $(addsuffix .o,$(STUB))

$(TAG): %.o: %.c $(DDEP)
	$(CC) -c $(CFLAGS) $(addsuffix .c,$(basename $@)) 
	@echo ------------------- $@ finished --------------------

ddep:
	@echo $(TAG) " dep on ---- " $(DDEP)

ifconf: %: %.c list.o $(DDEP)
	$(CC) $(addprefix $(UTILDIR), $<) $(addsuffix .c,$@) $(TCFLAGS) -o $@

list.o:
	make -C $(UTILDIR) $@
	
test: clean ifconf

clean:
	-$(RM) -f *.o core ifconf

.PHONY: ddep clean


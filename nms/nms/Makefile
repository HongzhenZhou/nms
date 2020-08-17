
BASEDIR = ../
include $(addprefix $(BASEDIR), Makefile.pub)

CFLAGS = $(CCFLAGS)

DDEP = $(addprefix $(LOCALINCDIR),debug.h list.h probe.h send.h snmp.h nms.h queue.h trap.h)

STUB = $(notdir $(CURDIR))
TAG = $(addsuffix .o,$(STUB))

$(TAG): %.o: %.c $(DDEP)
	$(CC) -c $(CFLAGS) $(addsuffix .c,$(basename $@)) 
	@echo ------------------- $@ finished --------------------

ddep:
	@echo $(TAG) " dep on ---- " $(DDEP)

clean:
	-$(RM) -f *.o core

.PHONY: ddep clean



BASEDIR = ../
include $(addprefix $(BASEDIR), Makefile.pub)

CFLAGS = $(DEBUGFLAGS) $(CCFLAGS) 
#CFLAGS = $(DEBUGFLAGS) $(CCFLAGS) -D__ZHZ_NOOP__

DDEP = $(addprefix $(LOCALINCDIR),debug.h list.h queue.h)
		
UTILS_O = list.o queue.o

$(UTILS_O): list.c queue.c $(DDEP) 
	$(CC) -c $(CFLAGS) $(addsuffix .c, $(basename $@)) $< 
	@echo ---------------- $@ finished --------------------

ddep:
	@echo "dep on ----" $(DDEP)
	
clean:
	-$(RM) -f *.o core

.PHONY: ddep clean


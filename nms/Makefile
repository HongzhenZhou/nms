
BASEDIR = ./
include $(addprefix $(BASEDIR), Makefile.pub)

CFLAGS = $(DFLAGS) $(LFLAGS)

UTILS = list.o queue.o
OBJS = ifconf.o probe.o echo.o nms.o send.o snmp.o lock.o trap.o
OBJDIRS = $(addsuffix /,$(basename $(OBJS)))
BINDIR = bin/
DIRS = $(UTILDIR) $(OBJDIRS) 
ADIRS = $(DIRS) $(BINDIR)

ALL_DEP = $(UTILS) $(OBJS) 

vpath nei-nms $(BINDIR)

all: nei-nms

nei-nms: $(ALL_DEP)
	$(CC) -o $(addprefix $(BINDIR), $@) $(CFLAGS) \
		$(addprefix $(UTILDIR), $(UTILS)) $(join $(OBJDIRS), $(OBJS))
	@echo ------------------- $@ finished --------------------
	
$(UTILS): force
	$(MAKE) -C $(UTILDIR) $@

$(OBJS): force
	$(MAKE) -C $(basename $@) $@

force: 

ddep:
	for d in $(DIRS); do $(MAKE) -C $$d $@; done
	@echo "all dep on ---- " $(ALL_DEP)
	
clean:
	for d in $(ADIRS); do $(MAKE) -C $$d $@; done
	-$(RM) -f *.o core nei-nms

.PHONY: all ddep clean

#make DFLAGS='-D__SEND_TRUE__ -DNDEBUG'
#make DFLAGS='-D__SEND_TRUE__'
#make DFLAGS='-D__ZHZ_DEBUG__ -D__SEND_TRUE__'
#make DFLAGS=-D__ZHZ_DEBUG__

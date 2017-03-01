files := $(shell echo *.c)
objs := $(patsubst %.c, %.o, $(files))
targetname := $(basename $(files))
CFLAGS := -Wall -O -g
CC := gcc 

$(targetname) : $(objs)
	$(CC) -o $@ $^
$(objs) : $(files)
	$(CC) $(CFLAGS) -c $(files)
.PHONY : install uninstall clean
install : $(targetname)
	sudo cp $(targetname) /usr/bin/$(targetname)
uninstall :
	sudo rm /usr/bin/$(targetname)
clean :
	rm $(targetname) $(objs)
 


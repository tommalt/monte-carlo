cc=cc
cxx=c++
cflags=-Wall -g -ggdb 
libs=-lgsl -lgslcblas -lm 
opt=0
OUTPUTDIR=build

objs=$(OUTPUTDIR)/walk.o 
.PHONY: $(OUTPUTDIR)/walk

$(OUTPUTDIR)/walk: $(objs)
	$(cxx) $(objs) -o $(OUTPUTDIR)/walk $(cflags) $(libs) -O$(opt)
	ln -sf $(OUTPUTDIR)/walk walk

$(OUTPUTDIR)/walk.o: walk.c
	$(cc) -c walk.c -o $(OUTPUTDIR)/walk.o $(cflags) -O$(opt)

clean:
	./clean.sh

install:
	./install.sh

uninstall:
	./uninstall.sh


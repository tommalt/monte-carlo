cc=clang
cxx=clang++
override cflags+=-Wall -g -ggdb 
libs=-lgsl -lgslcblas -lm 
opt=0
OUTPUTDIR=build

objs=$(OUTPUTDIR)/portfolio-simulation.o 
.PHONY: $(OUTPUTDIR)/portfolio-simulation

$(OUTPUTDIR)/portfolio-simulation: $(objs)
	$(cc) $(objs) -o $(OUTPUTDIR)/portfolio-simulation $(cflags) $(libs) -O$(opt)
	ln -sf $(OUTPUTDIR)/portfolio-simulation portfolio-simulation

$(OUTPUTDIR)/portfolio-simulation.o: portfolio-simulation.c
	$(cc) -c portfolio-simulation.c -o $(OUTPUTDIR)/portfolio-simulation.o $(cflags) -O$(opt)

clean:
	./clean.sh

install:
	./install.sh

uninstall:
	./uninstall.sh


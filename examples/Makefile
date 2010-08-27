CFLAGS=$(shell pkg-config --cflags libgcal)
LDLIBS=$(shell pkg-config --libs libgcal)

all: addeditdel addnew getallcontacts updatecal

clean:
	rm -f *.o
	rm -f addeditdel addnew getallcontacts updatecal

addeditdel: addeditdel.o

addnew: addnew.o

getallcontacts: getallcontacts.o

updatecal: updatecal.o


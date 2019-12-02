DEPS = cliente.h gestor.h utils.h comum.h

all : cliente gestor verificador

cliente : cliente.o utils.o comum.o
	gcc cliente.o utils.o comum.o -o cliente -lcurses -pthread

cliente.o : cliente.c
	gcc cliente.c -c

gestor : gestor.o utils.o comum.o
	gcc gestor.o utils.o comum.o -o gestor -pthread -lcurses

gestor.o : gestor.c
	gcc gestor.c -c

comum.o : comum.c
	gcc comum.c -c

verificador : verificador.c
	gcc verificador.c -o verificador

utils.o : utils.c
	gcc utils.c -c

clean : 
	rm -f *.o

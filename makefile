all : cliente gestor verificador

cliente : cliente.o utils.o
	gcc cliente.o utils.o -o cliente -lcurses

cliente.o : cliente.c
	gcc cliente.c -c

gestor : gestor.o utils.o
	gcc gestor.o utils.o -o gestor -pthread

gestor.o : gestor.c
	gcc gestor.c -c

verificador : verificador.c
	gcc verificador.c -o verificador

utils.o : utils.c
	gcc utils.c -c

clean : 
	rm -f *.o

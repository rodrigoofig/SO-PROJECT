CC = gcc
FLAGS = -Wall -g -pthread

OBJS1 = main.o systemManager.o taskManager.o edgeServers.o maintenance.o monitor.o signals.o
PROG1 = offload_simulator

OBJS2 = mobile_nodes.o systemManager.o taskManager.o edgeServers.o maintenance.o monitor.o signals.o
PROG2 = mobile_nodes 

all: 	${PROG1} ${PROG2}

clean:
	rm ${OBJS1} *- ${PROG1}
	rm ${OBJS2} *- ${PROG2}

${PROG1}:	${OBJS1}
		${CC} ${FLAGS} ${OBJS1} -o $@

${PROG2}:	${OBJS2}
		${CC} ${FLAGS} ${OBJS2} -o $@


.c.o:
		${CC} ${FLAGS} $< -c -o $@

######################
main.o: main.c functions.h
systemManager.o: systemManager.c functions.h
tasksManager.o: taskManager.c functions.h
edgeServers.o: edgeServers.c functions.h
maintenance.o: maintenance.c functions.h
monitor.o: monitor.c functions.h
signals.o: signals.c functions.h
mobile_nodes.o: mobile_nodes.c functions.h


offload_simulator: main.o systemManager.o taskManager.o edgeServers.o maintenance.o monitor.o signals.o
mobile_nodes: mobile_nodes.o systemManager.o taskManager.o edgeServers.o maintenance.o monitor.o signals.o

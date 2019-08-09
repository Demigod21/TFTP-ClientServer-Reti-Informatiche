#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define TESTO 0
#define BINARIO 1

#define RRQ 1
#define WRQ 2
#define ACK 3
#define DATA 4
#define ERRORE 5


struct pacchetto{
	uint16_t operazione;
	uint16_t nBlocco;
	uint16_t dimPac;
	char bufPac[516];
};


uint16_t ClientRichiesta(struct pacchetto*, char*, char);

void ServerAnalizzaRichiesta(struct pacchetto*, char*, char*);

int ClientSpacchetta(struct pacchetto*, char*, int);

int ClientAck(struct pacchetto*, char*);

uint16_t ServerDati(struct pacchetto*, char*, int);

int ServerAck(struct pacchetto*, char*);

uint16_t ServerErroreNonTrovato(struct pacchetto*, char*);

uint16_t ServerErroreNonRRQ(struct pacchetto*, char*);



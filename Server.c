#include "utile.h"

int main (int argc, char *argv[])
{
	//dichiaro le variabili che mi servono mano mano
	char portaAscolto[516];
	char nDirectory[516];
	char bufferServer[516]; //buffer per inviare e ricevere


	int sd, ret, new_sd;

	int LunErrore = 0;

	struct sockaddr_in indClient;
	struct sockaddr_in indServer;
	socklen_t lungIndirizzo;


	pid_t pid;

	

	struct pacchetto pServer;
	char mod = BINARIO; //la modalita predefinita e binario

	FILE *fileServer;

	
	int nBloccoS = 0;

	uint16_t lunghezzaPreparati = 0;

	//mi prendo la porta di ascolto e il nome della directory
	strcpy(portaAscolto, argv[1]);
	strcpy(nDirectory, argv[2]);

	printf("La porta di ascolto selezionata e' %s, mentre la directory e' %s\n", portaAscolto, nDirectory);

	//creo il socket
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	printf("Socket creato!! \n");

	memset(&indClient, 0, sizeof(indClient)); //pulizia dell'indirizzo del client
	memset(&indServer, 0, sizeof(indServer)); //pulizia server

	indServer.sin_family=AF_INET;
	indServer.sin_port=htons(atoi(argv[1]));
	indServer.sin_addr.s_addr= INADDR_ANY;


	lungIndirizzo = sizeof(indServer);
	//Bindo
	ret = bind(sd, (struct sockaddr*)&indServer, sizeof(indServer));
	
	if (ret<0)
	{
		perror("Bind non riuscita :  ");
		exit(-1);
	}
	printf("Bind riuscita!!!! \n");

	while (1)
	{
		memset(bufferServer, 0, 516); //pulisco il buffer 
		printf("In attesa di una richiesta da parte del client \n");

		ret = recvfrom(sd, bufferServer, 516, 0, (struct sockaddr*)&indClient, &lungIndirizzo);
		if (ret < 0)
		{
			perror("Richiesta fallita!!!! \n");
			continue;
		}
		printf("Ricevuta richiesta \n");
		
		pid=fork();

		if (pid == 0) //FIGLIO
		{
			char ipClient[516];
			inet_ntop(AF_INET, &indClient.sin_addr.s_addr, ipClient, 516);
			printf("Datagramma ricevuto da %s \n", ipClient);
			close(sd); //qua posso tranquillamente chiudere il listener socket, sono nel figlio

			new_sd = socket(AF_INET, SOCK_DGRAM, 0);
			ServerAnalizzaRichiesta(&pServer, bufferServer, &mod);

			if(pServer.operazione != RRQ){
				
				LunErrore = ServerErroreNonRRQ(&pServer, bufferServer);
				ret = sendto(new_sd, bufferServer, LunErrore, 0, (struct sockaddr*)&indClient, sizeof(indClient));
				if(ret<0)
				{
					perror("Invio errore di richiesta non consona non riuscito!!!! \n");
				}
				close(new_sd);
				exit(1);
			}

			//ogni volta all'avvio mi scordo lo slash finale, cosi controllo ci sia'
			
			if (nDirectory[strlen(nDirectory) - 1]!= '/')
			{
				strcat(nDirectory, "/");
			}
	
			strcat(nDirectory, pServer.bufPac);  //all'inizio del buffer ho il filenane, lo concaetno alla directory per avere il nome completo
		
			memset(pServer.bufPac, 0, 516);

			if (mod == BINARIO)
			{
				fileServer = fopen(nDirectory, "rb");
			}else
			{
				fileServer = fopen(nDirectory, "r");
			}

			if(!fileServer){

				LunErrore = ServerErroreNonTrovato(&pServer, bufferServer);
				ret = sendto(new_sd, bufferServer, LunErrore, 0, (struct sockaddr*)&indClient, sizeof(indClient));
				if(ret<0)
				{
					perror("Invio errore di file non trovato non riuscito!!!! \n");
				}
				close(new_sd);
				exit(1);
			}

			while (1)
			{
				memset(&pServer,0, sizeof(struct pacchetto));
	
				pServer.dimPac = fread(pServer.bufPac, sizeof(char), 512, fileServer);
		
				nBloccoS = (nBloccoS + 1) % 65536;

				pServer.nBlocco = nBloccoS;

				// printf("Questo e' il nr del blocco previsto %d \n", nBloccoS);
				
				//qua preparo il file!!!

				lunghezzaPreparati = ServerDati(&pServer, bufferServer, nBloccoS);


				ret = sendto(new_sd, bufferServer, lunghezzaPreparati, 0, (struct sockaddr*)&indClient, sizeof(indClient));

				if (ret < 0)
				{
					perror("Data non inviato \n");
					fclose(fileServer);
					close(new_sd);
					break;
					
				}
				printf("Nr blocco pacchetto inviato %d \n", pServer.nBlocco);
				memset(bufferServer, 0, 516);
		
				//ora qua dovrebbe arrivare l'ack

				ret  = recvfrom(new_sd, bufferServer, 516, 0, (struct sockaddr*)&indClient, &lungIndirizzo);

				if (ret < 0)
				{
					perror("La ricezione dell'ACK non e' andata a buon fine \n");
					fclose(fileServer);
					close(new_sd);
					break;
				}

				ret = ServerAck(&pServer, bufferServer);

				if(pServer.nBlocco != nBloccoS)
				{
					printf("Il numero del blocco ricevuto non e' corretto \n");
					fclose(fileServer);
					close(new_sd);
					break;
				}
				
				if (pServer.operazione != ACK)
				{
					printf("E' arrivato qualcosa ma non e' un ACK \n");
					fclose(fileServer);
					close(new_sd);
					break;
				} 

				//qui tutto ok
				printf("ACK ricevuto \n");
				printf("Stampiamo la dimensione del pacchetto che ho inviato %d \n", pServer.dimPac);
				if(pServer.dimPac < 512)
				{
					printf("Fine \n");
					fclose(fileServer);
					close(new_sd);
					break;
				}

			}
			exit(0);

		}else if (pid > 0) //PADRE
		{
			memset(bufferServer, 0, 516);
			continue;
		}else
		{
			printf("Fork fallita!!!! \n");
			exit(-1);
		}
	}
}


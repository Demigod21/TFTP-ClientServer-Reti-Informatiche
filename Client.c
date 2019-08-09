#include "utile.h"


void elencoComandi(){
	printf("Sono dispnibili i seguenti comandi:\n!help --> mostra l elenco dei comandi disponibili\n!mode {txt|bin} --> impsota il modo di trasferimento dei files (testo o binario)\n!get filename nome_locale --> richiede al server il nome del file <filename> e lo salva localmente con il nome <nome_locale>\n!quit --> termina il client\n\n");
}

int main (int argc, char *argv[])
{
	//dichiarazioni variabili
	char IPServer[516];
	char portaServer[516];
	
	char rigaComando[516];
	char comando[516];
	char primo[516]; //questo puo essere la modalita (se viene da !mode) oppure il filename
	char secondo[516]; //questo e il nome locale del path dove salvero la roba

	char bufferClient [516];

	FILE *fileClient;

	int sd, ret;

	struct sockaddr_in indServer;
	socklen_t lungIndirizzo;

	struct pacchetto pClient;

	char modClient = BINARIO;

	int lungh;

	int lDatiBuffer;

	lungIndirizzo = sizeof(indServer);

	int bloccoPrevisto = 0;

	//mi prendo ip e porta dagli argomenti
	strcpy(IPServer, argv[1]);
	strcpy(portaServer, argv[2]);

	//testo per vedere se ho preso correttamente gli argomenti
	printf("Indirizzo IP server: %s", IPServer);
	printf("\nPorta server: %s\n", portaServer);
	elencoComandi();


	while (1)
	{
		//creo il socket
		sd = socket(AF_INET, SOCK_DGRAM, 0);
		//pulizia
		memset(&indServer, 0, sizeof(indServer));
		//creo l'indirizzo del server
		indServer.sin_family=AF_INET;
		indServer.sin_port=htons(atoi(portaServer));
		inet_pton(AF_INET, IPServer, &indServer.sin_addr);
		printf("Si prega di inserire un comando, se non si conosce i comandi disponibile, digitare !help\n\n");

		//leggo comando
		fgets(rigaComando, 516, stdin);
		sscanf(rigaComando, "%s",comando); //leggo solo la prima parola, ovvero il comando, esercizio 3 socket
		
		//inizio ad analizzare i comandi
		if (strcmp(comando, "!help") == 0)  //HA CHIESTO HELP
		{
			elencoComandi();
			
		} else if(strcmp(comando, "!mode") == 0) //HA CHIESTO MODE
		{
			sscanf(rigaComando, "%s %s", comando, primo);
			//QUA DEVO CAMBIARE MODALITA
			
			if (strcmp(primo, "txt")==0)
			{
				modClient = TESTO;
				printf("Siete in modalita' testo \n");
			}else if (strcmp(primo, "bin")==0)
			{
				modClient = BINARIO;
				printf("Siete in modalita' binario \n");
			}else
			{
				printf("Errore : scrivere txt o bin!!! \n");
			}

		}else if (strcmp(comando, "!get") == 0) //HA CHIESTO GET, PARTE PRINCIPALE!!!!
		{
			sscanf(rigaComando, "%s %s %s", comando, primo, secondo); //in primo ho flename, in secondo ho nome_locale del path
			bloccoPrevisto = 0;
			
			if (modClient == TESTO)
			{
				fileClient = fopen(secondo,"a");
	
			}else
			{
				fileClient = fopen(secondo, "ab");				
			}

			if (!fileClient)
			{
				close(sd);
				break;
			}

			strcpy(pClient.bufPac, primo); //mi salvo nel pacchetto il filename	del file da richiedere		
			
			lungh = ClientRichiesta(&pClient, bufferClient, modClient);
			ret = sendto(sd, bufferClient, lungh, 0, (struct sockaddr*)&indServer, sizeof(indServer));

			if(ret<0)
			{	
				printf("Send non riuscita ");
				fclose(fileClient);
				close(sd);
				break;
			}
			printf("Richiesta file %s al server in corso.\n ", primo);
			//qui ho mandato la richiesta, nel while successivo dovrebbe arrivare la risposta, si spera
			while (1)
			{
				memset(&pClient, 0, sizeof(struct pacchetto));
				ret = recvfrom(sd, bufferClient, 516, 0, (struct sockaddr*)&indServer, &lungIndirizzo);
				lDatiBuffer = ret - 4; //in ret ho tutto, in lDatiBuffer ci metto la luunghezza del contenuto del buffer
				//quando dal server mando il tutto, nel buffer ho 16 bit (uint16) del tipo dell'operazione, poi 16 bit del nr del blocco. quindi 32 ovvero 4 byte che non sono del dato vero e proprio
				if(ret == 0) //il server è spento
				{
					printf("Server spento \n");
					fclose(fileClient);
					close(sd);
					break;
				}
				if (ret < 0)
				{
					perror("Receive non riuscita ");
					fclose(fileClient);
					close(sd);
					break;
				}

				//se arrivo qua, tutto bene, devo spacchettare messaggio
				bloccoPrevisto = (bloccoPrevisto + 1) % 65536;

				printf("Blocco atteso %d \n ", bloccoPrevisto);

				ret = ClientSpacchetta(&pClient, bufferClient, lDatiBuffer);
				printf("Blocco ricevuto %d \n ", pClient.nBlocco);
				if (bloccoPrevisto != pClient.nBlocco)
				{
					printf("Il blocco arrivato non e' il blocco previsto\n");
					fclose(fileClient);
					close(sd);
					break;
				}else if (pClient.operazione == ERRORE)
				{
					printf("Errore ");
					fclose(fileClient);
					close(sd);
					break;
				} 

				//se arrivo qua dovrebbe andare tutto bene
				if (modClient == TESTO)
				{
					fputs(pClient.bufPac, fileClient);
				}else
				{
					fwrite(pClient.bufPac, sizeof(char), lDatiBuffer , fileClient);
				}
				

				//ora devo preparare l'ack (spostata in utile)
				ret = ClientAck(&pClient, bufferClient);
				//mando l'ack preparato
				ret = sendto(sd, bufferClient, 4, 0, (struct sockaddr*)&indServer, lungIndirizzo);

				if (ret < 0)
				{
					perror("ACK non inviato ");
					fclose(fileClient);
					close(sd);
					break;
				}
				printf("ACK mandato \n");
				//mi rimane da controllare se sono arrivato alla fine, ovvero se la lunghezza dell'ultimo dato ricevuto e' minore di 512, ovvero 516 - 4.
				//la lunghezza del messaggio l'avevo salvata dopo la receive in lDatiBuffer
				if(lDatiBuffer < 512)
				{
					printf("Trasferimento completato (%d/%d blocchi)\n", bloccoPrevisto, bloccoPrevisto);
					printf("Salvataggio %s completato\n", primo);
					fclose(fileClient);
					close(sd);
					break;
				}
				

			}
			
		}else if (strcmp(comando, "!quit") == 0) //HA CHIESTO QUIT
		{
			printf("Chiusura del programma\n");
			exit(0);
		}else //errore
		{
			printf("Inserito comando errato \n");
		}
	}

	close(sd); 
	return 0;
}

#include "utile.h"


uint16_t ClientRichiesta(struct pacchetto* p2, char* buffer2, char mod2){
			uint16_t pos = 0;
			memset(buffer2, 0, 516);
			//struttura di una richiesta richiesta
			//operazione -> nomefile -> 1 byte vuoto -> modalita			

			//metto il nome dell'operazione ad inizio pacchetto e scorro
			p2->operazione = htons(RRQ);
			memcpy (buffer2, &p2->operazione, sizeof(uint16_t));
			pos = pos + sizeof(uint16_t);
			
			//metto il filename dopo il nome operazione e scorro
			strcpy((buffer2+pos), p2->bufPac);  //nel buffer del pacchetto, c'e' adesso "primo", ovvero il primo argomento della riga di comando, ovvero il filename
			pos = pos + strlen(p2->bufPac) + 1;
	
			//dopo byte vuoto, metto modalita
			if(mod2 == BINARIO){
					strcpy((buffer2 + pos), "BINARIO");
					pos = pos + (strlen("BINARIO") + 1);
			}else
			{
					strcpy((buffer2 + pos), "TESTO");
					pos = pos + (strlen("TESTO") + 1);
			}

			return pos;

}


void ServerAnalizzaRichiesta(struct pacchetto* p2, char* buffer2, char* mod2){
			uint16_t pos = 0;
		
			//la richiesta che mi arriva e' in buffer e la metto nel pacchetto
			//la struttura della richeista e' come sopra
			memcpy(&p2->operazione, buffer2, sizeof(uint16_t));
			pos = pos + sizeof(uint16_t); //pos ora "punta" al file name
			p2->operazione = ntohs(p2->operazione);

			
			strcpy(p2->bufPac, (buffer2 + pos));
			pos = pos + strlen(p2->bufPac) + 1; //scorro pos
			printf("%s\n", p2->bufPac);

			//buffer2+pos ora punta al tipo di operazione che ho preparato sopra, quindi vedo qual'e' e aggiorno la modalita
			if(strcmp((buffer2 + pos), "BINARIO") == 0){
				*mod2 = BINARIO;			
			}else
			{
				*mod2 = TESTO;
			}
			

};


int ClientSpacchetta(struct pacchetto* p2, char* buffer2, int posDati){
	uint16_t pos = 0;
	printf("Sto spacchettando \n");

	memcpy(&p2->operazione, buffer2, sizeof(uint16_t));
	pos = pos + sizeof(uint16_t);
	p2->operazione = ntohs(p2->operazione);

	memcpy(&p2->nBlocco, (buffer2 + pos), sizeof(uint16_t));
	pos = pos + sizeof(uint16_t);
	p2->nBlocco = ntohs(p2->nBlocco);

	memcpy(&p2->bufPac,(buffer2 + pos), posDati);
	return 0;

};


int ClientAck(struct pacchetto* p2, char* buffer2){

	uint16_t pos = 0;
	memset(buffer2, 0, 516);

	//printf("ACK prima di conversione e' nr %d'\n", p2->nBlocco);
	p2->operazione = htons(ACK);
	memcpy (buffer2, &p2->operazione, sizeof(uint16_t));
	pos = pos + sizeof(uint16_t);

	p2->nBlocco = htons(p2->nBlocco);
	memcpy ((buffer2 + pos), &p2->nBlocco, sizeof(uint16_t));
	pos = pos + sizeof(uint16_t);

	return 0;


}


uint16_t ServerDati(struct pacchetto* p2, char* buffer2, int nrBlocco){
	uint16_t pos = 0;

	
	p2->operazione = (htons(DATA));
	memcpy(buffer2, &p2->operazione, sizeof(uint16_t));
	pos = pos + sizeof(uint16_t);
	p2->operazione = (ntohs(DATA));

	p2->nBlocco = (htons(p2->nBlocco));
	memcpy ((buffer2 + pos), &p2->nBlocco, sizeof(uint16_t));
	pos = pos + sizeof(uint16_t);
	p2->nBlocco = (ntohs(p2->nBlocco));

	memcpy((buffer2 + pos), &p2->bufPac, p2->dimPac);
	pos = pos + p2->dimPac;

	return pos;
}

int ServerAck(struct pacchetto* p2, char* buffer2){

		uint16_t pos = 0;
		//int blocco = 0;
		int ris = 0;

		//printf("Sto guardando l'ACK ricevuto \n");

		memcpy(&p2->operazione, buffer2, sizeof(uint16_t));
		pos = pos + sizeof(uint16_t); 
		p2->operazione = ntohs(p2->operazione);

		//if(p2->operazione != ACK) ris=-1;

		memcpy(&p2->nBlocco, (buffer2 + pos), sizeof(uint16_t));
		pos = pos + sizeof(uint16_t);
		p2->nBlocco = ntohs(p2->nBlocco);

		// if(blocco != p2->nBlocco) 	ris=-2;

		printf("ACK arrivato %d \n" ,p2->nBlocco);
		

		return ris;

}



uint16_t ServerErroreNonRRQ(struct pacchetto* p2, char* buffer2){
	uint16_t pos = 0;
	
	p2->operazione = (htons(ERRORE));
	memcpy(buffer2, &p2->operazione, sizeof(uint16_t));
	pos = pos + sizeof(uint16_t);
	p2->operazione = (ntohs(ERRORE));

	strcpy((buffer2 + pos), "OPERAZIONE TFTP NON PERMESSA");
	pos = pos + (strlen("OPERAZIONE TFTP NON PERMESSA") + 1);

	printf("OPERAZIONE TFTP NON PERMESSA!! \n");

	return pos;

}

uint16_t ServerErroreNonTrovato(struct pacchetto* p2, char* buffer2){
	uint16_t pos = 0;
	
	p2->operazione = (htons(ERRORE));
	memcpy(buffer2, &p2->operazione, sizeof(uint16_t));
	pos = pos + sizeof(uint16_t);
	p2->operazione = (ntohs(ERRORE));

	strcpy((buffer2 + pos), "FILE NON TROVATO!");
	pos = pos + (strlen("FILE NON TROVATO!") + 1);

	printf("FILE NON TROVATO!! \n");

	return pos;

}




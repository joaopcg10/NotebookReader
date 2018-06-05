#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include "clist.h"

#define READ 0
#define WRITE 1
#define STDIN 0
#define STDOUT 1

#define OPEN ">>>"
#define CLOSE "<<<"

int size = 0;

int spacecount (char string[]){
	int i, count = 0;

	for (i = 0; string[i] != '\0'; i++)
		if (string[i] == ' ') count++;

	return count;
}

char* execList(CommandList comandos){
	int p[2];
	char buf[512];
	int i = 0;
	int x = 0;

	while (comandos && x < size){
		pipe(p);

		if (fork() == 0){

			int i = 0;
			char *temp = strtok(comandos->comando," ");
			char *arg[spacecount(comandos->comando)+2];

			while (temp){
					arg[i++] = temp;
					temp = strtok(NULL," ");
			}
			arg[i] = NULL;

			dup2(p[WRITE],STDOUT);

			close(p[WRITE]);
			close(p[READ]);

			execvp(arg[0],arg);

		} else {
			close(p[WRITE]);
			wait(NULL);

			i = 0;
			while(read(p[READ],&buf[i],1) == 1)
				i++;
			buf[i] = '\0';

			close(p[READ]);

			pipe(p);
			write(p[WRITE],buf,i);
			dup2(p[READ],STDIN);


			close(p[WRITE]);
			close(p[READ]);
			comandos = comandos -> prox;
			x++;
			}
	}
	buf[i-1] = '\0';
	return strdup(buf);		
}

void writeString (char string[], int file){
	int i = 0;
	
	while (string[i] != '\0')
		write(file,&string[i++],1);

	write(file,"\n",1);
}

int readString (int file, char string[]){
	int i = 0;

	while (read(file,&string[i],1) == 1){
		if (string[i] == '\n') break;
		i++;
	}
	string[i] = '\0';

	return i;
}

int decide (char string[]){
	if (string[0] == '$'){
		if (string[1] == '|') return 2;
			else return 1;
	} else return 0;
}

char* cropString (char string []){
	if (string[1] == '|')
		return (string+3);
	else return (string+2);
}

void swapContents (int realfile, int tempfile){
	char aux;

	while (read(tempfile,&aux,1) == 1)
		write(realfile,&aux,1);
}

int main (int argc, char *argv[]){
	if (argc < 2 || argc > 2){
		printf("ERROR! Número errado de argumentos!\n");
		return -1;
	}

	char *filename = argv[1];
	int tag = 0, nb_file, tempfile;

	if ((nb_file = open(filename, O_RDONLY)) < 0){
		printf("ERROR! Não conseguiu abrir o ficheiro\n");
		return -1;
	}
	tempfile = open("temp.txt", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

	CommandList list, aux;
	const char* cropped;
	aux = (CommandList) malloc(sizeof(struct clist));

	char string[512], *output;

	while (readString(nb_file,string) != 0){
		switch (decide(string)){
			case 0:
				if(strcmp(string,OPEN) == 0) tag = 1;
				if(tag == 0) writeString(string,tempfile);
				if(strcmp(string,CLOSE) == 0) tag = 0;

				break;

			case 1:
			    cropped = cropString(string);
				aux = realloc(aux,sizeof(struct clist)+strlen(cropped)+1);
				strcpy(aux->comando,cropped);
				aux->prox = NULL;
				list = aux;
				size = 1;

				output = execList(list);
				writeString(string,tempfile);
				writeString(OPEN,tempfile);
				writeString(output,tempfile);
				writeString(CLOSE,tempfile);
				break;

			case 2:
				cropped = cropString(string);
				aux = realloc(aux,sizeof(struct clist)+strlen(cropped)+1);
				strcpy(aux->comando, cropped);
				aux->prox = NULL;

				addToList(list,aux);
				size++;

				output = execList(list);
				writeString(string,tempfile);
				writeString(OPEN,tempfile);
				writeString(output,tempfile);
				writeString(CLOSE,tempfile);
				break;

			default:
				printf("ERROR!\n");
				return -1;
		}
	}
	free(aux);
	nb_file = open(filename,O_WRONLY | O_TRUNC);
	tempfile = open("temp.txt", O_RDONLY);

	swapContents(nb_file,tempfile);

	close(nb_file);
	close(tempfile);

	printf("end\n");
	execlp("rm","rm","temp.txt",NULL);
}
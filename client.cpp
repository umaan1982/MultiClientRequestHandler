#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <pthread.h>

void errorinwrite();
void errorinread();
void errorininput();
bool checkalphanum(char S[]);
void *clientrec(void* ptr);

int sock;

int main(int argc, char *argv[])
	{
	struct sockaddr_in server;
	struct hostent *hp;
	char buf[1024];
    pthread_t thread;

	/* Create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("opening stream socket");
		exit(1);
	}
	/* Connect socket using name specified by command line. */
	server.sin_family = AF_INET;
	hp = gethostbyname(argv[1]);
	if (hp == 0) {
		fprintf(stderr, "%s: unknown host\n", argv[1]);
		exit(2);
	}
	bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(atoi(argv[2]));

	if (connect(sock,(struct sockaddr *) &server,sizeof(server)) < 0) {
		perror("connecting stream socket");
		exit(1);
	}
    write(STDOUT_FILENO,"1.Add command ---> add <list>\n",sizeof("1.Add command ---> add <list>\n"));
    write(STDOUT_FILENO,"2.Subtract command --> sub <list>\n",sizeof("2.Subtract command --> sub <list>\n"));
    write(STDOUT_FILENO,"3.Multiply command --> mult <list>\n",sizeof("3.Multiply command --> mult <list>\n"));
    write(STDOUT_FILENO,"4.Divide command --> div <list>\n",sizeof("4.Divide command --> div <list>\n"));
    write(STDOUT_FILENO,"5.Run command --> run <program name>\n",sizeof("5.Run command --> run <program name>\n"));
    write(STDOUT_FILENO,"6.Kill command --> kill<name>/<pid>\n",sizeof("6.Kill command --> kill<name>/<pid>\n"));
    write(STDOUT_FILENO,"7.List command --> list/ list all\n",sizeof("7.List command --> list/ list all\n"));
    write(STDOUT_FILENO,"8.Exit command --> exit\n",sizeof("8.Exit command --> exit\n"));
    pthread_attr_t attribute;
    pthread_attr_init( &attribute);
    pthread_attr_setdetachstate(&attribute,PTHREAD_CREATE_DETACHED);
    int t1= pthread_create(&thread,&attribute,&clientrec,NULL);
    if(t1 != 0)
        perror("Thread error");
    int chk2 = write(STDOUT_FILENO,"In client side program, write command\n",sizeof("In client side program, write command\n"));
    if(chk2 == -1)
        errorinwrite();
    while(true){
        char reader[300];
        int nu = read(STDIN_FILENO,reader,300);
        reader[nu-1]=NULL;
        if(nu == -1)
            errorinread();
        int wr = write(sock,reader,300);
        if(wr == -1)
            perror("Cant send data to server");
        
    }
	close(sock);
}

void errorinwrite(){
    perror("Error in Write");
}
void errorinread(){
    perror("Error in read");
}
void errorininput(){
   perror("Alphabet/non-numerical character detected! Write input again!");
}

void* clientrec(void* ptr){
    while(true){
        char sum[500];
        int rd = read(sock,sum,500);
        sum[rd]=NULL;
        if(strcmp(sum,"Exiting server here")==0){
            write(STDOUT_FILENO,"Exiting client",sizeof("Exiting client"));
            write(STDOUT_FILENO,"\n",sizeof("\n"));
            exit(0);
        }
        if(rd == -1)
            errorinread();
        if((strcmp(sum,"Exiting server here")!=0 ) && (rd>0)){
        int c=write(STDOUT_FILENO,"Showing result\n",sizeof("Showing result\n"));
        int c2=write(STDOUT_FILENO,sum,strlen(sum));
        int c3=write(STDOUT_FILENO,"\n",sizeof("\n"));
        if(c2 == -1 || c3 == -1 || c==-1)
            errorinwrite();
        int chk = write(STDOUT_FILENO,"In client side program, write command\n",sizeof("In client side program, write command\n"));
        if(chk == -1)
            errorinwrite();
         
        }
        if(rd == 0){
            write(STDOUT_FILENO,"Exiting client\n",sizeof("Exiting client\n"));
            exit(EXIT_SUCCESS);
        }
    }
}
bool checkalphanum(char S[]){
    bool flag = false;
    for(int i =0;i < strlen(S);i++){
        if(!isalpha(S[i])){
            flag = true;
        }
        else{
            flag = false;
            break;
        }
    }
    return flag;
}
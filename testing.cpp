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
#include <arpa/inet.h>
#include <pthread.h>
#include <iostream>

struct listing{
    char pname[20];
    int pid;
    time_t Stime;
    time_t Etime;
    time_t Elapsed;
    char active[5];
};

struct clientlist{
    int serialno;
    int sockcomm;
    char ip[15];
    char recport[10];
    int connectionpid;
    char active[5];
    int pipesending;
};

void errorinwrite();
void errorinread();
void errorininput();
bool checkalphanum(char S[]);
void add(char* parcel, int msgsock);
void sub(char* parcel, int msgsock);
void mult(char* parcel, int msgsock);
void div(char* parcel, int msgsock);
void killmethod(char* parcel, int msgsock, struct listing process[], struct listing processall[]);
void list(char* parcel, int msgsock, struct listing process[], struct listing processall[]);
void run(char* parcel, int msgsock, struct listing process[], struct listing processall[]);
void exit(char* parcel, int msgsock, struct listing process[], struct listing processall[]);
void unexpected(int msgsock, char tempresult[]);
void *serverinput(void* ptr);
void *clienthandler(void* ptr);

int pno=0;
int pno2=0;
int connectioncount=0;
bool flag2;
int check[2];
struct clientlist connectionlist[100];
bool userflag[100];
int waitingpid;

static void siginthandler(int signo){
    if(signo == SIGCHLD){
        write(STDOUT_FILENO,"Client connection or Process terminated\n",sizeof("Client connection or Process terminated\n"));
        flag2 = true;
        waitingpid = waitpid(-1,NULL,WNOHANG);
        for(int i=0;i<connectioncount;i++){
            if(waitingpid == connectionlist[i].connectionpid){
                strcpy(connectionlist[i].active,"no");
                userflag[i] = false;
                flag2 =false;
                break;
            }
        }
    }
}

int main()
{
    pipe(check);
    //signal(SIGCHLD,siginthandler);
    struct listing process[100];
    struct listing processall[100];
	int sock, length;
	struct sockaddr_in server;
	int msgsock;
	char buf[1024];
	int rval;
	int i;
	/* Create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("opening stream socket");
		exit(1);
	}
	/* Name socket using wildcards */
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(50000);
	if (bind(sock, (struct sockaddr *) &server, sizeof(server))) {
		perror("binding stream socket");
		exit(1);
	}
	/* Find out assigned port number and print it out */
	length = sizeof(server);
	if (getsockname(sock, (struct sockaddr *) &server, (socklen_t*) &length)) {
		perror("getting socket name");
		exit(1);
	}
    char something[1024];
    int ssd = sprintf(something,"Listening at Port no-> %d\n",ntohs(server.sin_port));
	write(STDOUT_FILENO,something,ssd);
	fflush(stdout);
    pthread_attr_t attribute;
    pthread_attr_init( &attribute);
    pthread_attr_setdetachstate(&attribute,PTHREAD_CREATE_DETACHED);
    pthread_t thread1;
    int t1 = pthread_create(&thread1,&attribute,&serverinput,(void*) connectionlist);
    if(t1 != 0)
        perror("Thread error");
    

	/* Start accepting connections */
    signal(SIGCHLD,siginthandler);
	listen(sock, 5);
	do {
        struct sockaddr_in conn;
        //socklen_t conn_size;
        int conn_size= sizeof(conn);
		msgsock = accept(sock,(struct sockaddr *)&conn,(socklen_t *)&conn_size);
        char connresult[100] = "fsd";
        strcpy(connresult,inet_ntoa(conn.sin_addr));
        write(STDOUT_FILENO,connresult, strlen(connresult));
        char connresult2[100] = "fds";
        int connresult2_size = sprintf(connresult2," Port %d\n",ntohs(conn.sin_port));
        write(STDOUT_FILENO,connresult2,connresult2_size);
		if (msgsock == -1)
			perror("accept");
        int pid = fork();
        if(pid > 0){
            connectionlist[connectioncount].pipesending= check[1];
            userflag[connectioncount] =true;
            connectionlist[connectioncount].serialno = connectioncount +1;
            strcpy(connectionlist[connectioncount].ip,inet_ntoa(conn.sin_addr));//client ip
            sprintf(connectionlist[connectioncount].recport,"%d",ntohs(conn.sin_port));//receiving port number
            connectionlist[connectioncount].connectionpid = pid; //connection process id
            connectionlist[connectioncount].sockcomm = msgsock; //msgsock communication number
            strcpy(connectionlist[connectioncount].active,"yes"); //Activiation
            connectioncount++;
        }
        if(pid ==0){  
            if(msgsock > 0){
            char connection[50] = {};
            int st = sprintf(connection,"Connection established at %d\n",msgsock);
            write(STDOUT_FILENO,connection,st);
        }
        pthread_attr_t attribute2;
        pthread_attr_init( &attribute2);
        pthread_attr_setdetachstate(&attribute2,PTHREAD_CREATE_DETACHED);
        pthread_t thread2;
        int t2 = pthread_create(&thread2,&attribute2,&clienthandler,(void*)process);
        if(t2 == -1)
            perror("client handler thread");
        while(true){
        char tempresult[100];
        if(flag2== true){
            for(int l =0;l<pno;l++){
                if(process[l].pid == waitingpid){
                    waitingpid = -200;
                            strcpy(tempresult,process[l].pname);
                            unexpected(msgsock , tempresult);
                            processall[l].Etime= time(NULL);
                            processall[l].Elapsed = (processall[l].Etime-processall[l].Stime);
                            strcpy(processall[l].active,"no");
                            strcpy(process[l].pname,"done");
                }
            }
        }
        char reader[300];
        int nu = read(msgsock,reader,300);
        if(nu == -1)
            errorinread();
        if(nu == 0){
                char final[20];
            int c=sprintf(final,"Exiting server here");
            for(int i = 0;i<pno;i++){
                if(strcmp(process[i].pname,"done") != 0){
                    int kill_status = kill(process[i].pid,SIGTERM);
                    if(kill_status == -1)
                        perror("Error in killing all process");
                    int status;
                    int wait_chk = waitpid(process[i].pid,&status,0);
                    if(wait_chk==-1)
                        perror("Error in waitpid");
                }
            }
            write(msgsock,final,c);
            char ending[100];
            sprintf(ending,"Exiting server for connection %d\n",msgsock);
            write(STDOUT_FILENO,ending,strlen(ending));
            exit(0);
        }
        else if(nu >0){
        int chk = write(STDOUT_FILENO,"In server side program, processing \n",sizeof("In server side program,processing \n"));
        if(chk == -1)
            errorinwrite();
        char *parcel = strtok(reader," ");
        char check[10];
        sprintf(check,"%s",parcel);
        //if(strlen(tempresult)>0){
        //char unexpected[100];
        //int unexpno = sprintf(unexpected,"%s was terminated from server side",tempresult);
        //write(msgsock,unexpected,unexpno);
        //}
        if(strcmp(check,"add")==0){
            add(parcel, msgsock);
        }
        else if(strcmp(check,"sub")==0){
            sub(parcel,msgsock);
        }
        else if(strcmp(check,"mult")==0){
            mult(parcel,msgsock);
        }
        else if(strcmp(check,"div")==0){
            div(parcel, msgsock);
        }
        else if(strcmp(check,"kill")==0){
            killmethod(parcel,msgsock,process,processall);
        }
        else if(strcmp(check,"list")==0){
            list(parcel,msgsock,process,processall);
        }
        else if(strcmp(check,"run")==0){
            run(parcel,msgsock,process,processall);
        }
        else if(strcmp(check,"exit")==0){
            exit(parcel,msgsock,process,processall);
        }
        else{
            char er[40];
            int c = sprintf(er,"Invalid command");
            write(msgsock,er,c);
            char addresult[100];
            int addresult_size = sprintf(addresult,"Result sent at %d\n",msgsock);
            write(STDOUT_FILENO,addresult,addresult_size);
        }
        }
        }
        close(msgsock);
        }
		//close(msgsock);
	} while (true);
}
void errorinwrite(){
    perror("Error in Write");
}
void errorinread(){
    perror("Error in read");
}
void errorininput(){
   perror("Error in input from user,result sent!");
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

void *clienthandler(void* ptr){
 struct listing *activeprocess = (struct listing*)ptr;
 while(true){
      char buffer[100];
     int reading = read(check[0],buffer,100);
     //write(STDOUT_FILENO,"recieved",strlen("recieved"));
     char display[reading];
     strcpy(display,buffer);
     char *token = strtok(display," ");
     if(strcmp(token,"list")==0){
         char item_send[300];
                char sender_thing[300] = "";
                for(int i =0;i<pno;i++){
                    if(strcmp(activeprocess[i].pname,"done")!= 0){
                    tm TM = *localtime(&activeprocess[i].Stime);
                    int hours = TM.tm_hour;
                    int min = TM.tm_min;
                    int sec = TM.tm_sec;
                    int item_num = sprintf(item_send,"P.Name: %s PID: %d ST:%d:%d:%d ET:%ld Elapsed: %ld sec Active: %s \n",activeprocess[i].pname,activeprocess[i].pid,hours,min,sec,activeprocess[i].Etime,activeprocess[i].Elapsed,activeprocess[i].active);        
                    strcat(sender_thing,item_send);
                    }
                }
                if(strlen(sender_thing)>0){
                    write(STDOUT_FILENO,sender_thing,strlen(sender_thing));
                }
                else if(strlen(sender_thing) == 0){
                    write(STDOUT_FILENO,"No list available\n",strlen("No list available\n"));
                }
     }
 }
}

void *serverinput(void* ptr){
    struct clientlist *connlist = (struct clientlist*)ptr;
    write(STDOUT_FILENO,"Server side commands are:\n",strlen("Server side commands are:\n"));
    write(STDOUT_FILENO,"1.list <To list process of every client currently active>\n",strlen("1.list <To list process of every client currently active>\n"));
    write(STDOUT_FILENO,"2.list serialno <To list the active process of that client associated with the serialno>\n",strlen("2.list serialno <To list the active process of that client associated with the serialno>\n"));
    write(STDOUT_FILENO,"3.print message <Broadcast message to every client currently connected>\n",strlen("3.print message <Broadcast message to every client currently connected>\n"));
    write(STDOUT_FILENO,"4.print serialno message <Sending the message to a specific client associated with the serialno>\n",strlen("4.print serialno message <Sending the message to a specific client associated with the serialno>\n"));
    while(true){
    char buffer[1000];
    char *inputparcel;
    bool flaging = false;
    int readbytes = read(STDIN_FILENO,buffer,1000);
    buffer[readbytes-1] ='\0';
    inputparcel = strtok(buffer," ");
    if(readbytes == -1)
        perror("Read error from server");
    if(inputparcel != NULL){
    if(strcmp(inputparcel,"list")==0){
        inputparcel = strtok(NULL," ");
        if(inputparcel == NULL){ //list details of every client
            for(int i=0;i<connectioncount;i++){
                if(strcmp(connlist[i].active,"yes")== 0){
                char something[1000];
                int byteswritten = sprintf(something,"Ip: %s serial number : %d Receiving port :%s Socket communication:%d Connection pid:%d",connlist[i].ip,connlist[i].serialno,connlist[i].recport,connlist[i].sockcomm,connlist[i].connectionpid);
                write(STDOUT_FILENO,something,byteswritten);
                write(STDOUT_FILENO,"\n",strlen("\n"));
                write(connlist[i].pipesending,"list",strlen("list"));
                sleep(1);
                }
            }
            for(int i=0;i<connectioncount;i++){
                if(userflag[i] == false){
                    flaging = false;
                }
                else{
                    flaging = true;
                    break;
                }
            }
            if(flaging == false){
                write(STDOUT_FILENO,"No client active right now\n",strlen("No client active right now\n"));
            }
        }
        else{ //list details of specific client
            char clientnumber[10];
            int number=0;
            int chk=0;
            sscanf(inputparcel,"%s",clientnumber);
            if(checkalphanum(clientnumber)== true){  
                chk = sscanf(inputparcel,"%d",&number); 
                if(chk >0){
                for(int i=0;i<connectioncount;i++){
                    if(number == connlist[i].serialno && strcmp(connlist[i].active,"yes")==0){
                        char something[100];
                        int byteswritten = sprintf(something,"Ip: %s serial number : %d Receiving port %s",connlist[i].ip,connlist[i].serialno,connlist[i].recport);
                        write(STDOUT_FILENO,something,byteswritten);
                        write(STDOUT_FILENO,"\n",strlen("\n"));
                        write(connlist[i].pipesending,"list",strlen("list"));
                        break;
                    }
                }
                }
                else{
                    write(STDOUT_FILENO,"wrong value\n",strlen("wrong value\n"));
                }
            }
            else{
                write(STDOUT_FILENO,"wrong value\n",strlen("wrong value\n"));
            }
            if(userflag[number-1]== false){
                write(STDOUT_FILENO,"client not available\n",strlen("client not available\n"));
            }
        }
    }
    else if(strcmp(inputparcel,"print")==0){//sending hello
        bool clientcheck = false;
        bool clientcheck2 = false;
        char compile[100];
        char numbercompare[10];
        inputparcel = strtok(NULL," ");
        if(inputparcel != NULL){
            char storing[100];
            sscanf(inputparcel,"%s",numbercompare);
            if(checkalphanum(numbercompare)== false){
            while(inputparcel != NULL){
            sscanf(inputparcel,"%s",storing); //storing 2nd params
            strcat(compile," ");
            strcat(compile,storing);
            inputparcel = strtok(NULL," ");
            }
        if(inputparcel == NULL && checkalphanum(numbercompare)== false){//send message to all clients
            for(int i=0;i<connectioncount;i++){
                if(strcmp(connlist[i].active,"yes")==0){
                    clientcheck = true;
                    char sendingthings[200];
                    sprintf(sendingthings,"Message :%s",compile);
                    write(connlist[i].sockcomm,sendingthings,strlen(sendingthings));
                }
            }
            bzero(compile,strlen(compile)); //Empty the message array
            bzero(storing,strlen(storing));
            if(clientcheck == false){
                write(STDOUT_FILENO,"No client active right now\n",strlen("No client active right now\n"));
            }
            else if(clientcheck = true){
                write(STDOUT_FILENO,"Message has been broadcasted\n",strlen("Message has been broadcasted\n"));
            }
        }
            }
        else if(inputparcel != NULL && checkalphanum(numbercompare)==true){//message to specfic client //2nd param client id
           char compile2[200];
           inputparcel = strtok(NULL, " ");
            while(inputparcel != NULL){
            int sizeofcmd2 = strlen(inputparcel);//storing 3rd params (message)
            char storing2[sizeofcmd2];
            sscanf(inputparcel,"%s",storing2);
            strcat(compile2," ");
            strcat(compile2,storing2);
            inputparcel = strtok(NULL," ");
            }
            int number;
            sscanf(numbercompare,"%d",&number);
        for(int i=0;i<connectioncount;i++){
            if(number == connlist[i].serialno && strcmp(connlist[i].active,"yes")==0){
                clientcheck2 = true;
                char sendingthings[300];
                sprintf(sendingthings,"Message :%s",compile2);
                write(connlist[i].sockcomm,sendingthings,strlen(sendingthings));
                break;
            }
        }
        bzero(compile2,strlen(compile2));
        if(clientcheck2 == false){
            write(STDOUT_FILENO,"Client not available\n",strlen("Client not available\n"));
        }
        else if(clientcheck2 == true){
            write(STDOUT_FILENO,"Message has been sent\n",strlen("Message has been sent\n"));
        }
            inputparcel = strtok(NULL," ");
            if(inputparcel != NULL){
                write(STDOUT_FILENO,"Too many arguments",strlen("Too many arguments"));
            }
        }
        }
        else{
            write(STDOUT_FILENO,"Invalid command, please check again\n",strlen("Invalid command, please check again\n"));
        }
    }
    else{
        write(STDOUT_FILENO,"Invalid command, please check again\n",strlen("Invalid command, please check again\n"));
    }
    }
    else{
        write(STDOUT_FILENO,"No input,write again\n",strlen("No input,write again\n"));
    }
    }
}

void add(char* parcel, int msgsock){
    int sum =0;
            int chk2=0;
            int number= 0;
            char final[20];
            bool flagcheck = false;
            parcel = strtok(NULL," ");
            while(parcel != NULL){
                int size = strlen(parcel);
                char S[size];
                sscanf(parcel,"%s",S);
                if (checkalphanum(S)== true){
                chk2 = sscanf(parcel,"%d",&number);
                if (chk2 > 0)
                    sum = number + sum;         
                }
                else{
                    errorininput();
                    write(msgsock,"Alphabet/non-numerical character detected! Write input again!",strlen("Alphabet/non-numerical character detected! Write input again!"));                    
                    sum=0;
                    flagcheck = true;
                    break;
                }
                parcel = strtok(NULL, " ");
            }
            if(flagcheck == false){
            sprintf(final,"The ans is %d",sum);
            int send = write(msgsock,final,20);
            if(send == -1)
                errorinwrite();
            }
            char subresult[100];
            int subresult_size = sprintf(subresult,"Result sent at %d\n",msgsock);
            write(STDOUT_FILENO,subresult,subresult_size);
}

void sub(char* parcel, int msgsock){
    int sum =0;
            int chk2=0;
            int number= 0;
            char final[20];
            parcel = strtok(NULL," ");
            sscanf(parcel,"%d",&sum);
            parcel = strtok(NULL," ");
            bool flagcheck = false;
            while(parcel != NULL){
                int size = strlen(parcel);
                char S[size];
                sscanf(parcel,"%s",S);
                if (checkalphanum(S)== true){
                chk2 = sscanf(parcel,"%d",&number);
                if (chk2 > 0)
                    sum = sum - number;         
                }
                else{
                    errorininput();
                    sum=0;
                    write(msgsock,"Alphabet/non-numerical character detected! Write input again!",strlen("Alphabet/non-numerical character detected! Write input again!"));
                    flagcheck=true;
                    break;
                }
                parcel = strtok(NULL, " ");
            }
            if(flagcheck == false){
            sprintf(final,"The ans is %d",sum);
            int send = write(msgsock,final,20);
            if(send == -1)
                errorinwrite();
            }
            char multresult[100];
            int multresult_size = sprintf(multresult,"Result sent at %d\n",msgsock);
            write(STDOUT_FILENO,multresult,multresult_size);
}

void mult(char* parcel, int msgsock){
    int sum =1;
            int chk2=0;
            int number= 0;
            char final[20];
            bool flagcheck = false;
            parcel = strtok(NULL," ");
            while(parcel != NULL){
                int size = strlen(parcel);
                char S[size];
                sscanf(parcel,"%s",S);
                if (checkalphanum(S)== true){
                chk2 = sscanf(parcel,"%d",&number);
                if (chk2 > 0)
                    sum = number * sum;         
                }
                else{
                    errorininput();
                    write(msgsock,"Alphabet/non-numerical character detected! Write input again!",strlen("Alphabet/non-numerical character detected! Write input again!"));
                    flagcheck = true;
                    sum=0;
                    break;
                }
                parcel = strtok(NULL, " ");
            }
            if(flagcheck == false){
            sprintf(final,"The ans is %d",sum);
            int send = write(msgsock,final,20);
            if(send == -1)
                errorinwrite();
            }
            char runresult[100];
            int runresult_size = sprintf(runresult,"Result sent at %d\n",msgsock);
            write(STDOUT_FILENO,runresult,runresult_size);
}

void div(char* parcel, int msgsock){
    float sum =1;
            int chk2=0;
            int number= 0;
            char final[20];
            bool flagcheck = false;
            parcel = strtok(NULL," ");
            sscanf(parcel,"%f",&sum);
            char check[strlen(parcel)];
            sscanf(parcel,"%s",check);
            int checkforzero = strlen(parcel);
            char zerocheck[checkforzero];
            int z;
            sscanf(parcel,"%d",&z);
            if(z == 0){
                write(msgsock,"The ans is 0",sizeof("The ans is 0"));
            }
            if(checkalphanum(check)== true){
            parcel = strtok(NULL," ");
            while(parcel != NULL){
                int size = strlen(parcel);
                char S[size];
                sscanf(parcel,"%s",S);
                if (checkalphanum(S)== true){
                chk2 = sscanf(parcel,"%d",&number);
                if(number !=0){
                if (chk2 > 0)
                    sum = sum/number;        
                else{
                    errorininput();
                    write(msgsock,"Alphabet/non-numerical character detected! Write input again!",strlen("Alphabet/non-numerical character detected! Write input again!"));
                    flagcheck = true;
                    sum=0;
                    break;
                }
                }
                else if(number == 0){
                    write(msgsock,"Error, 0 entered, Undefined",sizeof("Error, 0 entered, Undefined"));
                    sum =0;
                    break;
                }
                }
                else if(checkalphanum(S)== false){
                    errorininput();
                    write(msgsock,"Alphabet/non-numerical character detected! Write input again!",strlen("Alphabet/non-numerical character detected! Write input again!"));
                    flagcheck = true;
                    sum=0;
                    break;
                }
                parcel = strtok(NULL, " ");
            }
        }
        else{
            errorininput();
            sum =0;
        }
            if(sum > 0 && flagcheck == false){
            sprintf(final,"The ans is %f",sum);
            int send = write(msgsock,final,20);
            if(send == -1)
                errorinwrite();
            }
            char divresult[100];
            int divresult_size = sprintf(divresult,"Result sent at %d\n",msgsock);
            write(STDOUT_FILENO,divresult,divresult_size);
}

void killmethod(char* parcel, int msgsock, struct listing process[], struct listing processall[]){
    parcel = strtok(NULL," ");
            if(parcel !=NULL){
            bool flag = false;
            if(parcel != NULL){
            int s = strlen(parcel);
            int checkert;
            char dir[s];
            int st;
           
            sprintf(dir,"%s",parcel);
            sscanf(parcel,"%d",&st);
            if(checkalphanum(dir) == false){
                for(int i=0;i<pno;i++){
                    if(strcmp(process[i].pname,dir)==0){
                        int kill_chk=kill(process[i].pid,SIGTERM);
                        if(kill_chk==-1)
                            perror("Error in killing process");
                        int status;
                        int wait_chk = waitpid(process[i].pid,&status,0);
                        if(wait_chk==-1)
                            perror("Error in waitpid");
                        flag = true;
                        processall[i].Etime= time(NULL);
                        processall[i].Elapsed = (processall[i].Etime-processall[i].Stime);
                        strcpy(processall[i].active,"no");
                        strcpy(process[i].pname,"done");
                    }
                }
            }
            else if(checkalphanum(dir) == true){
                int kill_chk=kill(st,SIGTERM);
                bool killingflag = false;
                if(kill_chk==-1)
                {
                    perror("Error in killing process");
                    killingflag = true;
                }
                int status;
                int wait_chk = waitpid(st,&status,0);
                if(wait_chk==-1)
                    perror("Error in waitpid");
                for(int i=0;i<pno;i++){
                    if(process[i].pid == st && killingflag == false)
                        {
                            flag = true;
                            processall[i].Etime= time(NULL);
                            processall[i].Elapsed = (processall[i].Etime-processall[i].Stime);
                            strcpy(processall[i].active,"no");
                            strcpy(process[i].pname,"done");
                        }
                }
            }
            }
            else{
                write(msgsock,"Wrong command",sizeof("Wrong command"));
                char killresult1[100];
            int killresult_size1 = sprintf(killresult1,"Result sent at %d\n",msgsock);
            write(STDOUT_FILENO,killresult1,killresult_size1);
            }
            if(flag == true){
                        write(msgsock,"Termination done",sizeof("Termination done"));
                        char killresult2[100];
            int killresult_size2 = sprintf(killresult2,"Result sent at %d\n",msgsock);
            write(STDOUT_FILENO,killresult2,killresult_size2);
            }
            else if (flag== false){
                write(msgsock,"Invalid pid/name",sizeof("Invalid pid/name"));
                char killresult3[100];
            int killresult_size3 = sprintf(killresult3,"Result sent at %d\n",msgsock);
            write(STDOUT_FILENO,killresult3,killresult_size3);
            }
            }
            else{
                write(msgsock,"Wrong command",sizeof("Wrong command"));
                char killresult4[100];
            int killresult_size4 = sprintf(killresult4,"Result sent at %d\n",msgsock);
            write(STDOUT_FILENO,killresult4,killresult_size4);
            }
}

void list(char *parcel, int msgsock, struct listing process[],struct listing processall[]){
    parcel = strtok(NULL," ");
            if(parcel != NULL){
            int s = strlen(parcel);
            char dir[s];
            sprintf(dir,"%s",parcel);
            if(strcmp(dir,"all")==0){
                char item_send[500];
                char sender_thing[500] = "";
                for(int i =0;i<pno2;i++){
                    tm TM = *localtime(&processall[i].Stime);
                    int hours = TM.tm_hour;
                    int min = TM.tm_min;
                    int sec = TM.tm_sec;
                    if(strcmp(processall[i].active,"no")==0){
                    tm TM2 = *localtime(&processall[i].Etime);
                    int hours2 = TM2.tm_hour;
                    int min2 = TM2.tm_min;
                    int sec2 = TM2.tm_sec;
                    int item_num = sprintf(item_send,"P.Name: %s PID: %d ST:%d:%d:%d ET:%d:%d:%d Elapsed: %ld sec Active: %s \n",processall[i].pname,processall[i].pid,hours,min,sec,hours2,min2,sec2,processall[i].Elapsed,processall[i].active);      
                    }
                    else if(strcmp(processall[i].active,"yes")==0){
                        int item_num = sprintf(item_send,"P.Name: %s PID: %d ST:%d:%d:%d ET:%ld Elapsed: %ld sec Active: %s \n",processall[i].pname,processall[i].pid,hours,min,sec,processall[i].Etime,processall[i].Elapsed,processall[i].active);    
                    }  
                    strcat(sender_thing,item_send);
                }
                if(strlen(sender_thing)>0){
                write(msgsock,sender_thing,strlen(sender_thing));
                char listresult1[100];
            int listresult1_size = sprintf(listresult1,"Result sent at %d\n",msgsock);
            write(STDOUT_FILENO,listresult1,listresult1_size);
                }
                else if (strlen(sender_thing)==0 ){
                    write(msgsock,"No list available,",strlen("No list available"));
                    char listresult2[100];
            int listresult2_size = sprintf(listresult2,"Result sent at %d\n",msgsock);
            write(STDOUT_FILENO,listresult2,listresult2_size);
                }
            }
            else if(strlen(dir)>0){
                write(msgsock,"Wrong command",strlen("Wrong command"));
                char listresult3[100];
            int listresult3_size = sprintf(listresult3,"Result sent at %d\n",msgsock);
            write(STDOUT_FILENO,listresult3,listresult3_size);
            }
            }
            else{
                char item_send[300];
                char sender_thing[300] = "";
                for(int i =0;i<pno2;i++){
                    if(strcmp(process[i].pname,"done")!= 0){
                    tm TM = *localtime(&process[i].Stime);
                    int hours = TM.tm_hour;
                    int min = TM.tm_min;
                    int sec = TM.tm_sec;
                    int item_num = sprintf(item_send,"P.Name: %s PID: %d ST:%d:%d:%d ET:%ld Elapsed: %ld sec Active: %s \n",process[i].pname,process[i].pid,hours,min,sec,process[i].Etime,process[i].Elapsed,process[i].active);        
                    strcat(sender_thing,item_send);
                    }
                }
                if(strlen(sender_thing)>0){
                write(msgsock,sender_thing,strlen(sender_thing));
                char listresult4[100];
            int listresult4_size = sprintf(listresult4,"Result sent at %d\n",msgsock);
            write(STDOUT_FILENO,listresult4,listresult4_size);
                }
                else if (strlen(sender_thing)==0 ){
                    write(msgsock,"No list available,",strlen("No list available"));
                    char listresult5[100];
            int listresult5_size = sprintf(listresult5,"Result sent at %d\n",msgsock);
            write(STDOUT_FILENO,listresult5,listresult5_size);
                }
            }
}

void run(char *parcel, int msgsock, struct listing process[], struct listing processall[]){
    parcel = strtok(NULL," ");
    if(parcel != NULL){
            int s = strlen(parcel);
            char dir[s];
            sprintf(dir,"%s",parcel);
            int pipefd3[2];
            int pipe_chk =pipe2(pipefd3,O_CLOEXEC);
            if(pipe_chk == -1){
                perror("In pipe");
            }
            time_t stime = time(0);
            int pi = fork();
            if(pi == 0){ //Child-new process
                close(pipefd3[0]);
                int a = execlp(dir,dir,NULL);
                int write_chk = write(pipefd3[1],"Process not working",sizeof("Process not working"));
                if(write_chk == -1)
                    perror("write error");
            exit(0);
            }
            else if(pi > 0){
                close(pipefd3[1]);
                char si[20];
                int read_chk = read(pipefd3[0],si,20);
                if(read_chk == 0){
                char final[19];
                int c=sprintf(final,"Process successful");
                write(msgsock,final,c);
                char runresult3[100];
                int runresult3_size = sprintf(runresult3,"Result sent at %d\n",msgsock);
                write(STDOUT_FILENO,runresult3,runresult3_size);
                process[pno].pid = pi;
                strcpy(process[pno].pname,dir);
                process[pno].Stime = stime;
                process[pno].Etime= NULL;
                process[pno].Elapsed =NULL;
                strcpy(process[pno].active,"yes");
                processall[pno2].pid=pi;
                processall[pno2].Stime=stime;
                processall[pno2].Etime=NULL;
                processall[pno2].Elapsed=NULL;
                strcpy(processall[pno2].pname,dir);
                strcpy(processall[pno2].active,"yes");
                pno++; 
                pno2++;
                }
                else if(read_chk >0){
                char final[22];
                int c=sprintf(final,"Wrong process command");
                write(msgsock,final,c);
                char runresult1[100];
                int runresult1_size = sprintf(runresult1,"Result sent at %d\n",msgsock);
                write(STDOUT_FILENO,runresult1,runresult1_size);
                }
            }
            else if(pi < 0){
                perror("Fork failed");
            }
    }
    else{
        write(msgsock,"Invalid command",strlen("Invalid command"));
        char runresult2[100];
            int runresult2_size = sprintf(runresult2,"Result sent at %d\n",msgsock);
            write(STDOUT_FILENO,runresult2,runresult2_size);
    }
}

void exit(char *parcel, int msgsock, struct listing process[], struct listing processall[]){
    char final[20];
            int c=sprintf(final,"Exiting server here");
            for(int i = 0;i<pno;i++){
                if(strcmp(process[i].pname,"done") != 0){
                    int kill_status = kill(process[i].pid,SIGTERM);
                    if(kill_status == -1)
                        perror("Error in killing all process");
                    int status;
                    int wait_chk = waitpid(process[i].pid,&status,0);
                    if(wait_chk==-1)
                        perror("Error in waitpid");
                }
            }
            write(msgsock,final,c);
            char ending[100];
            sprintf(ending,"Exiting server for connection %d\n",msgsock);
            write(STDOUT_FILENO,ending,strlen(ending));
            exit(0);
}

void unexpected(int msgsock, char tempresult[]){
    char unexpected[100];
    int unexpno = sprintf(unexpected,"%s was terminated from server side\n",tempresult);
    write(msgsock,unexpected,unexpno);
}

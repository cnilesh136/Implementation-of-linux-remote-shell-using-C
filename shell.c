#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<netinet/in.h>
#include<string.h>
#include<stdio.h>
#include<netdb.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<arpa/inet.h>


char curr_dir[1000];
char inp_buff[1024];
char history_file_path[500];
char *split[512];
char message[1024] = {0};
char client_message[200]={0};
char recvBuffer[1000],sendBuffer[1000],executeBuffer[1000]={0};

//Fuctions

void command();
int server();


void re_initialize()
{
	curr_dir[0]='\0';
	inp_buff[0]='\0';
}

void printinit()
{
	char dir[1000];
	printf("\033[1;31m");
	printf("MyShell@Nilesh:");
	printf("\033[1;32m");
	if (getcwd(curr_dir, sizeof(curr_dir)) != NULL)
        {
          strcpy(dir, curr_dir);
          strcat(dir, "$ ");
          printf("%s", dir);
        }
   else
       perror("Error in getting current Directory");
	printf("\033[0m");
}

void write_history()
{
	int hist_fd,len,result;
	char hist_data[2000];
	strcpy(hist_data,inp_buff);
	strcat(hist_data,"\n");
	len=strlen(hist_data);

	hist_fd=open(history_file_path,O_WRONLY|O_APPEND|O_CREAT,S_IRUSR|S_IWUSR);
	result=write(hist_fd,hist_data,len);
	if(result<0)
	{
		printf("Error in Writing");
	}
	close(hist_fd);


}

void print_history_file()
{
	int hist_fd;
	char read_data[1024];
	hist_fd=open(history_file_path,O_RDONLY);

	while(read(hist_fd,read_data,sizeof(read_data))!=0)
		printf("%s",read_data);

	printf("\n");
}

int remote()
{
		int socketID,connStatus,length;

		struct sockaddr_in serverAdd;
		char sendBuffer[1000],recvBuffer[1000];

		int pid;

		bzero(&serverAdd,sizeof(serverAdd));

		serverAdd.sin_family=AF_INET;
		serverAdd.sin_addr.s_addr=inet_addr(split[1]);
		serverAdd.sin_port=htons(atoi(split[2]));


		socketID=socket(AF_INET,SOCK_STREAM,0);


		connStatus=connect(socketID,(struct sockaddr*)&serverAdd,sizeof(serverAdd));
		if(connStatus<0)
		{
			printf("Error In Connection..");
			return -1;
		}
		printf("\nConnected...\n");
		pid=fork();
		if(pid==0)
		{

			while(1)
			{
				bzero(&sendBuffer,sizeof(sendBuffer));
				fgets(sendBuffer,10000,stdin);
				length=strlen(sendBuffer);
				sendBuffer[length-1]='\0';
				if(strcmp(sendBuffer,":exit")==0)
				{
					printf("Disconnected From Remote...\n");
					close(socketID);
					return 1;
				}
				send(socketID,sendBuffer,strlen(sendBuffer)+1,0);
				fflush(stdin);
			}
		}
		else
		{
			while(1)
			{
				bzero(&recvBuffer,sizeof(recvBuffer));
				recv(socketID,recvBuffer,sizeof(recvBuffer),0);
				printf("%s\n",recvBuffer);
			}
		}
		return 0;
}
void change_directory()
{
	char *h="/home";
	if(split[1]==NULL)
		    chdir(h);
	else if ((strcmp(split[1], "~")==0) || (strcmp(split[1], "~/")==0))
		    chdir(h);
	else if(chdir(split[1])<0)
		printf("bash: cd: %s: No such file or directory\n", split[1]);

}

void print_directory()
{
	if (getcwd(curr_dir, sizeof(curr_dir)) != NULL)
		    {
		     printf("%s\n", curr_dir );
		    }
	else
		   perror("getcwd() error");
}

void command_execute()
{
	int pid;
	pid=fork();
	if(pid==0)
	{
		if(execvp(split[0], split)<0) printf("%s: command not found\n", split[0]);
	}
	else
	{
		waitpid(pid,0,0);
	}
	return;
}

void execute_it()
{
	int n=1,success;

	split[0]=strtok(inp_buff," ");
	while((split[n]=strtok(NULL," "))!=NULL)
		n++;
	split[n]=NULL;
	if(strcmp("cd",split[0])==0)
					{
					change_directory();
					return ;
					}
	else if(strcmp("pwd",split[0])==0)
					{
					print_directory();
					return ;
					}
	if(strcmp(split[0],"remote")==0)
	{
		if(n>2)
		{
				success=remote();
		}
		else
		{
			printf("Needs more arguments!!\n");
		}
	}
	else
	{
		command_execute();
	}

}



void print_shell_dist()
{
	printf("\033[1;36m");
	printf("\n\n=============================Welcome TO MY SHELL==================================\n\n");
	printf("\033[0m");
}

int main()
{
	re_initialize();
	getcwd(curr_dir,sizeof(curr_dir));
	strcpy(history_file_path,curr_dir);
	strcat(history_file_path,"/history.txt");
	int len,ser;
	if(fork()==0)
	{
		ser=server();
	}
	print_shell_dist();
	while(1)
	{
		re_initialize();
		printinit();
		fgets(inp_buff,1024,stdin);

		if(strcmp(inp_buff,"\n")==0)
		{
			continue;
		}
		len=strlen(inp_buff);
		inp_buff[len-1]='\0';

		if(strcmp(inp_buff,"exit")==0)
		{
			printf("EXITING...\nBye...\n\n");
			exit(0);
		}
		if(strcmp(inp_buff,"history")==0)
		{
			print_history_file();
			continue;
		}

		write_history();
		execute_it();

	}
}


int server()
{
	    int clientSocketID,socketID;
		pid_t childpid;
		struct sockaddr_in serverAddress,clientAddress;
		socklen_t clientLength;


		pid_t cpid;
		bzero(&serverAddress,sizeof(serverAddress));
		serverAddress.sin_family=AF_INET;
		serverAddress.sin_addr.s_addr=htonl(INADDR_ANY);
		serverAddress.sin_port=htons(8070);
		socketID=socket(AF_INET,SOCK_STREAM,0);

		bind(socketID,(struct sockaddr*)&serverAddress,sizeof(serverAddress));

		listen(socketID,5);
		//printf("%s\n","Server is running ...");

		while(1)
		{
			clientSocketID=accept(socketID,(struct sockaddr*)&clientAddress,&clientLength);
			if(clientSocketID < 0)
			{
            	exit(1);
        	}
        	printf("One Device Connected: %s",inet_ntoa(clientAddress.sin_addr));

			if((childpid = fork()) == 0)
			{
				close(socketID);
				while(1)
				{


					recv(clientSocketID,recvBuffer,sizeof(recvBuffer),0);
					 if(strcmp(recvBuffer, ":exit") == 0)
		            {
		             	printf("Disconnected from %s:\n", inet_ntoa(clientAddress.sin_addr));
		                break;
		            }
		            else
		            {
						command();
						send(clientSocketID,executeBuffer,strlen(executeBuffer)+1,0);
						bzero(&recvBuffer,sizeof(recvBuffer));
						executeBuffer[0]='\0';
					}
				}
			}
		}
		close(clientSocketID);
		return 0;
}


void command()
{

	char buff[200],remote_command[500];
	FILE *fp;
	char *h="/home";
	char *split1 [100];
	int k=1;
	executeBuffer[0]='\0';
	strcpy(remote_command,recvBuffer);
	split1[0]=strtok(remote_command," ");
	while((split1[k]=strtok(NULL," "))!=NULL)
		k++;
	split[k]=NULL;
	printf("%s",split1[0]);
	if(strcmp(split1[0],"cd")==0)
	{
		
		if(split1[1]==NULL)
				chdir(h);
		else if ((strcmp(split1[1], "~")==0) || (strcmp(split1[1], "~/")==0))
				chdir(h);
		else if(chdir(split1[1])<0)
				strcat(executeBuffer,"NO Such Directory");
			//printf("bash: cd: %s: No such file or directory\n", split1[1]);
	}
	else
	{
		fp = popen(recvBuffer,"r");
		while ( fgets( buff, 200, fp ) != NULL )
		{
			strcat(executeBuffer,buff);
			printf("\n%s",buff);
		}
		
	}
	strcat(executeBuffer,"Current Directory is:");
		
		if (getcwd(curr_dir, sizeof(curr_dir)) != NULL)
		    {
		     printf("\n%s\n", curr_dir );
		    }
		else
		   perror("getcwd() error");
		strcat(executeBuffer,curr_dir);
}

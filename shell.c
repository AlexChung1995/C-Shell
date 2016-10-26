#include <fcntl.h>
#include <math.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
//
//
// Please compile with the -lm flag so that math.h can be linked. Ie gcc filename.c -lm
//
pid_t *jobpids;
char ***bgs;
int cmdNum = 0;
char ***cmds;




void assignPids(pid_t pid, pid_t *pids)//adds process ID to array when background process starts
{

	for (int i = 9; i > 0; i--)
	{
		*(pids+i)=*(pids+i-1);
	}
	*pids = pid;
}

void exitShell()//closing proceduress
{
	free(cmds);
	free(jobpids);
	free(bgs);
	for (int i = 0; i<10; i++)
	{
		kill(*(jobpids+i),SIGKILL);
	}
	exit(3);
}

void sigfunc(int sig)//catches signal from child instructing parent to close
{
	if (sig == SIGUSR1)
	{
		exitShell();
	}
}

void cd(char* dir)//function for changing directory
{
	chdir(dir);
	exit(3);
}
void pwd(size_t bufsize)//print working directory
{
	char * dir;
	if (getcwd(dir,bufsize)==NULL)
	{
		pwd(2*bufsize);
	}
	else
	{
		printf("%s\n",dir);
		exit(3);
	}
}
void printJobs()//formated print for position in array of job, pid, and job name
{
	for (int j = 0; j<10; j++)
	{
		printf("%d %d ",j,*(jobpids+j));
		for (int i = 0; i< 20;i++)
		{
			printf("%s ",*(*(bgs+j)+i));
		}
		printf("\n");
	}
}

void removeCmd(int z,char *** cmdss)//remove cmd from array
{
	for (z;z<9;z++)
	{
		for (int k = 0; k < 20; k++)
		{
			*(*(cmdss+z)+k) = *(*(cmdss+z+1)+k);
		}
	}

	*(*(cmdss+9))="";
}
int removePID(pid_t pid, pid_t *pids)//
{
	int i = 0;
	for (i; i<10; i++)
	{
		if (*(pids+i) == pid)
		{
			break;
		}
	}
	int j = i;
	for (j;j<9;j++)
	{
		*(pids+j) = *(pids+j+1);
	}
	*(pids+9)=(pid_t)0;
	return i;
}

void fg(int i)//foreground process sends SIGCONT in case choice process has stopped
{
	kill((pid_t)(*(jobpids+i)),SIGCONT);
	waitpid((pid_t)(*(jobpids+i)),NULL,0);
	int z = removePID((pid_t)(*(jobpids+i)),jobpids);
	removeCmd(z,bgs);
}
void sigchild(int sig)
{
	if (sig == SIGCHLD)
	{
		for (int i = 0; i<10; i++)
		{
			if ((*(jobpids+i)) != ((pid_t)0))
			{
				int status;
				int k = waitpid(*(jobpids+i),&status,WNOHANG|WUNTRACED);
				if(WIFSTOPPED(status)||WIFEXITED(status))
				{
					removePID((pid_t)*(jobpids+i),jobpids);
					removeCmd(i,bgs);
				}
			}
		}
	}
}
void assignCmd(char **args1,char***cmdss)
{
	for (int i = 9; i>0 ; i--)
	{
		for (int k = 0; k<20;k++)
		{
			*(*(cmdss+i)+k) = *(*(cmdss+(i-1))+k);
		}

	}
	int k = 0;
	for ( k = 0; k < 20; k++)
	{
		*((*cmdss)+k)=*(args1+k);
	}
	*(*(cmdss+0)+k)='\0';
}
int assignArgs(char **args,int j)
{
	int k;
	for ( k = 0; k<20; k++)
	{
		if (*(*(cmds+j)+k) == NULL)
		{
			*(args+k) = NULL;
			return k;
		}

		*(args+k)=*(*(cmds+j)+k);
	}
	return k;
}
int  piping(char ** args)
{
	for (int i = 0; i < 20; i ++)
	{
		if(*(args + i) ==NULL)
		{
			break;
		}
		if (strcmp(*(args+i),"|")==0)
		{
			*(args+i) = '\0';
			return i;
		}
	}
	return 0;
}
int oRedirect(char ** args)
{
	for (int i = 0; i < 20; i ++)
	{
		if(*(args + i) ==NULL)
		{
			break;
		}
		if (strcmp(*(args+i),">")==0)
		{
			*(args+i) = '\0';
			return i;
		}
	}
	return 0;
}
void redirectProc(char ** args, int oRed)
{
	int fd = open(*(args+oRed+1), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	dup2(fd,1);
	close(fd);
}
void pipeProc(char ** args, int pStatus, int fd[])
{
	pipe(fd);
	int status;
	char *childargs[20];

	pid_t pid = fork();
	if (pid == 0)
	{
		int i = 0;
		for (i = 0; i<= pStatus; i++)
		dup2(fd[1],1);
		close(fd[0]);
    }
	else
	{
		close(fd[1]);
		char parentargs[20];
		int i = pStatus + 1;
		dup2(fd[0],0);
	}
}
int getcmd(char *prompt, char **args, int *background)
{
	cmdNum ++;
	int length, i = 0;
	char *token, *loc;
	char *line = NULL;
	size_t linecap = 0;
	printf("%s", prompt);
	length = getline(&line, &linecap, stdin);

	if (length <= 0)
	{
		exit(-1);
	}
	if (line[0]=='!')
	{
			int num = 0;
			int n = strlen(line)-2;
                        for (int j = 1; j <= n;j++)
                        {
                                char c = line[j];
                                if (('0'<= c) && (c <= '9'))
                                {
                                        int x = c - '0';
                                        num = num + (x*((int) pow(10,n-j)));
                                }
                                else
                                {
                                        printf("Invalid input");

                                }

                        }

			if ((cmdNum - num) > 10|| (cmdNum - num)<0)
			{
				printf("There have been %d commands. Please request one of the last 10 commands.", cmdNum);
				return 0;
			}
			i = assignArgs(args,(cmdNum - num - 1));
	}
	else
	{
		// Check if background is specified..
		if ((loc = index(line, '&')) != NULL)
		{
			*background = 1;
			*loc = ' ';
		}
		else
		{
			 *background = 0;
		}
		while ((token = strsep(&line, " \t\n")) != NULL)
		{
			for (int j = 0; j < strlen(token); j++)
			{
				if (token[j] <= 32)
				{
					token[j] = '\0';
				}
			}
			if (strlen(token)>0)
			{
				*(args+i) = token;
				i++;
			}
		}
		*(args + i) = NULL;
	}

	assignCmd(args,cmds);

	return i;
}


int main(void)
{
	signal(SIGUSR1,sigfunc);
	signal(SIGCHLD,sigchild);
	cmds= (char***)malloc(sizeof(char**)*10);
	for (int i = 0; i<10;i++)
	{
		*(cmds + i) = (char **)malloc(sizeof(char*)*20);
		for (int j = 0; j<20;j++)
		{
			*(*(cmds+i)+j)=(char *)malloc(10);
		}
	}
	bgs = (char***)malloc(sizeof(char**)*10);
	for (int i = 0; i<10;i++)
	{
		*(bgs + i) = (char **)malloc(sizeof(char*)*20);
		for (int j = 0; j<20;j++)
		{
			*(*(bgs+i)+j)=(char *)malloc(10);
		}
	}
	jobpids = (pid_t*)malloc(sizeof(pid_t)*10);
	int bg,status;

	while(1)
	{
		int fd[2];
		char **args = (char **)malloc(20*(sizeof(char *)));
		for (int i = 0; i<20; i++)
		{
			*(args+i) = (char *) malloc(10);
		}
		bg = 0;
		int cnt = getcmd("\n>>", args, &bg);
		pid_t pid , waitstatus;
		int oRed = oRedirect(args);
		int pStatus = piping(args);
		if ((strcmp(*(args),"fg"))==0)
		{
			fg(atoi(*(args+1)));
		}
		else
		{
			pid = fork();

			if (pid < 0)
			{
				printf("ERROR: CREATING CHILD PROCESS");
				exit(1);
			}

			else if (pid == 0)
			{

				if (oRed > 0)
				{
					redirectProc(args,oRed);
				}
				if (pStatus > 0)
				{
					printf("Entering pipe\n");
					pipeProc(args,pStatus,fd);
				}
				else{
				if ( execvp(*(args),args) < 0 )
				{
					if (cnt = 1)
					{
						if (strcmp(*args,"pwd")==0)
						{
							size_t bufsize = 30;
							pwd(bufsize);
							exit(3);
						}
						else if (strcmp(*args,"jobs")==0)
						{
							printJobs();
							exit(3);
						}
						else if(strcmp(*args,"exit")==0)
						{
							kill(getppid(),SIGUSR1);
						}
						else if(strcmp(*args,"history")==0)
						{
							for (int i = 0; i<10; i++)
							{
								printf("%d ",cmdNum - i);
								for (int j = 0; j<20;j++)
								{
									printf(" %s ",*(*(cmds+i)+j));
								}
								printf("\n");
							}
							exit(3);
						}
					}
					if (cnt = 2)
					{
						if (strcmp(*args,"cd")==0)
						{
							cd(*(args+1));
						}
					}
					else
					{
						printf("Command input does not exist. Please enter a valid command");
						exit(3);
					}
				}
				}
			}
			else
			{
				if (bg != 1)
				{

					waitstatus = waitpid(pid,&status,0);
					if (waitstatus == -1)
					{
						exit(EXIT_FAILURE);
					}
					if (WIFEXITED(waitstatus))
					{
						printf("exited, status = %d\n",WEXITSTATUS(waitstatus));
					}
					else if(WIFSIGNALED(waitstatus))
					{
						printf("killed by signal %d\n", WTERMSIG(waitstatus));
					}
				}
				else
				{
					assignPids(pid,jobpids);
					assignCmd(args,bgs);
				}
			}
			free(args);
		}
	}
}

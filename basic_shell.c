#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<errno.h>
#include<wait.h>
#include<pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<termios.h>
int errno;
int curpid=-1;
char*arg[100],*res;
int argc,status;
struct process
{
    pid_t pid;
    char cname[100][100];
    int stat;
    int argcnt;
    int stop;

};
struct process parr[1000];
int parrptr=0;
void pinfo(int pid)
{
    printf("pid -- %d\n",pid);
    char d[100],c[100],buf[33],exname[100],a[2]=" ",line[10],vm[100],path[100];
    int i;
    for(i=0;i<100;i++)
	path[i]='\0';
    strcpy(c,"/proc/");
    sprintf(buf,"%d",pid);
    strcat(c,buf);
    strcpy(d,c);
    strcat(c,"/status");
    strcat(d,"/exe");
    readlink(d,path,100);
    int f=open(c,0444);
    if(f<0)
    {
	printf("no such process\n");
	return;
    }
    read(f,line,5);
    while(a[0]==' ')
    {
	read(f,a,1);
    }
    exname[0]=a[0];
    int p=1;
    while(a[0]!='\n')
    {
	read(f,a,1);
	exname[p++]=a[0];
    }
    exname[p]='\0';
    read(f,line,6);
    line[6]='\0';
    strcpy(a," ");
    char status;
    while(!(a[0]>='A'&&a[0]<='Z'))
    {
	read(f,a,1);
    }
    status=a[0];
    printf("Process Status -- %c\n",status);
    int lcnt=0;
    while(lcnt!=10)
    {
	read(f,a,1);
	if(a[0]=='\n')
	    lcnt++;
    }
    read(f,line,7);
    line[7]='\0';

    strcpy(a," ");
    if(!strcmp(line,"VmSize:"))
    {
	while(a[0]==' ')
	{
	    read(f,a,1);
	}
	vm[0]=a[0];
	int p=1;
	while(a[0]!='\n')
	{
	    read(f,a,1);
	    vm[p++]=a[0];
	}
	vm[p]='\0';
	printf("memory -- %s\n",vm);
    }
    else
	printf("memory -- no information available\n");
    printf("Executable Path -- %s\n",path);
}
void sig_handler(int signum)
{
    int i;
    if(signum==SIGCHLD)
    {
	int pid,stat;
	pid=waitpid(WAIT_ANY,&stat,WNOHANG);
	if(pid>0)
	{
	    for(i=0;i<parrptr;i++)
		if(parr[i].pid==pid)
		    break;
	    if(stat!=0)
	    {
		printf("\n UNKNOWN COMMAND(press enter to continue)\n");
		if(i<parrptr)
		    parr[i].stat=0;
	    }
	    else
	    {
		if(i<parrptr)
		{
		    parr[i].stat=0;
		    if(WIFEXITED(stat))
			printf("\n%s %d terminated normally with exit status %d \n",parr[i].cname[0],parr[i].pid,WEXITSTATUS(stat));
		    if(WIFSIGNALED(stat))
			printf("\n%s %d terminated by signal  %d \n",parr[i].cname[0],parr[i].pid,WTERMSIG(stat));

		}
	    }
	}
	fflush(stdout);
	signal(SIGCHLD,sig_handler);
	return;
    }
    if(signum==SIGTSTP)
    {
	if(curpid!=-1)
	{

	    printf("received SIGTSTP\n");
	    kill(getpid(),SIGTSTP);
	}
    }
    fflush(stdout);
}
void check_status()
{
    int j=0,pst;
    for(j=0;j<parrptr;j++)
    {
	//	    printf("hi check %d\n",j);
	if(parr[j].stat==1)
	{
	    pid_t rid=waitpid(parr[j].pid,&pst,WNOHANG);
	    if(rid==-1 || rid>0&&rid==parr[j].pid)
	    {
		parr[j].stat=0;
		if(WIFEXITED(pst))
		    printf("\n%s %d terminated normally with exit status %d \n",parr[j].cname[0],parr[j].pid,WEXITSTATUS(pst));
		if(WIFSIGNALED(pst))
		    printf("\n%s %d terminated by signal  %d \n",parr[j].cname[0],parr[j].pid,WTERMSIG(pst));
		//		printf("(press enter to continue)\n");

		fflush(stdout);
	    }
	}
    }

}
int main(void)
{
    signal(SIGTSTP,sig_handler);
    pid_t spgid;
    int isinter,ret,shell=STDIN_FILENO;
    struct termios shell_tmodes;
    isinter=isatty(shell);
    if(isinter)
    {
	while(tcgetpgrp(shell)!=(spgid=getpgrp()))
	    kill(-spgid,SIGTTIN);
	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGTTOU,SIG_IGN);
	//        signal(SIGCHLD,sig_handler);
	spgid=getpid();
	if(setpgid(spgid,spgid)<0)
	{
	    perror("cant put shell in its process grp");
	    exit(1);
	}
	tcsetpgrp(shell,spgid);
	tcgetattr(shell,&shell_tmodes);
    }

    pid_t pid;
    int curpid,len,i,j,k,status,background=0;
    char c,home[100],cmd[1000],prompt[400],cwd[100],uname[100],hname[100];
    getlogin_r(uname,100);
    gethostname(hname,100);
    prompt[0]='<';
    prompt[1]='\0';
    strcat(prompt,uname);
    prompt[strlen(prompt)+1]='\0';
    prompt[strlen(prompt)]='@';
    strcat(prompt,hname);
    prompt[strlen(prompt)+1]='\0';
    prompt[strlen(prompt)]=':';
    len=strlen(prompt);
    const char *homedir = getenv("HOME");
    chdir(homedir);
    while(1)
    {
	check_status();
	background=0;
	if(getcwd(cwd,100)!=NULL)
	{
	    strcpy(home,cwd);
	    home[strlen(homedir)]='\0';
	    char*q=cwd;
	    char*p=q+strlen(homedir);
	    if(!strcmp(home,homedir))
	    {
		cwd[0]='~';
		cwd[1]='\0';
		strcat(cwd,p);
	    }
	    prompt[len]='\0';
	    strcat(prompt,cwd);
	}
	else
	    perror("getcwd() error");
	prompt[strlen(prompt)+1]='\0';
	prompt[strlen(prompt)]='>';
	do
	{
	    check_status();
	    cmd[0]='\0';
	    printf("%s",prompt);
	    scanf("%[^\n]",cmd);
	    getchar();
	    for(i=0;i<100;i++)
		arg[i]=NULL;
	    i=0;
	    res=strtok(cmd," \t");
	    while(res!=NULL)
	    {	
		arg[i++]=res;
		res=strtok(NULL," \t");
	    }
	    argc=i;
	}while(arg[0]==NULL);
	//	for(i=0;i<argc;i++)
	//	    printf("%s ",arg[i]);
	printf("\n");
	if(!strcmp(arg[argc-1],"&"))
	{
	    arg[argc-1]=NULL;
	    background=1;
	}
	if(!strcmp(arg[0],"quit"))
	    break;
	if(!strcmp(arg[0],"cd")){
	    if(arg[1]==NULL || strcmp(arg[1],"~")==0  ||strcmp(arg[1],"~/")==0|| strcmp(arg[1],"")==0)
	    {
		if(chdir(getenv("HOME"))<0)
		{
		    perror("error changing");
		}
	    }
	    else
	    {
		ret=chdir(arg[1]);
		if(ret<0)
		    perror("directory does not exist");
	    }
	    continue;	
	}
	if(!strcmp(arg[0],"pinfo")&&arg[1]==NULL){
	    int procid=getpid();
	    pinfo(procid);
	    continue;
	}	    
	if(!strcmp(arg[0],"pinfo")&&arg[1]!=NULL&&arg[2]==NULL){
	    pinfo(atoi(arg[1]));
	    continue;
	}
	if(!strcmp(arg[0],"jobs")&&arg[1]==NULL){
	    //printf("enter jobs\n");
	    check_status();
	    //			printf("checked stat\n");
	    int j=0,k,n=1;
	    for(j=0;j<parrptr;j++)
	    {
		if(parr[j].stat)
		{

		    //			printf("printng job %d\n",n);
		    printf("[%d] ",n);
		    for(k=0;k<parr[j].argcnt;k++)
			printf("%s ",parr[j].cname[k]);
		    printf("[%d]\n",parr[j].pid);
		    n++;
		}
	    }
	    continue;
	}
	if(!strcmp(arg[0],"kjob")&&arg[1]!=NULL&&arg[2]!=NULL&&arg[3]==NULL)
	{
	    int r,id,i,a,b,n;
	    a=atoi(arg[1]);
	    b=atoi(arg[2]);
	    n=0;
	    for(i=0;n<a&&i<parrptr;i++)
	    {
		if(parr[i].stat==1)
		    n++;
	    }
	    if(a!=n)
		printf("no such job\n");
	    else
	    {
		id=parr[i-1].pid;
		//		printf("%d\n",id);
		r=kill(id,b);
		if(r==-1)
		    perror("signal not sent");
		else if(r==0)
		    printf("signal sent\n");
	    }
	    continue;
	}
	if(!strcmp(arg[0],"fg")&&arg[1]!=NULL&&arg[2]==NULL)
	{
	    int st,id,i,a,n;
	    a=atoi(arg[1]);
	    n=0;
	    for(i=0;n<a&&i<parrptr;i++)
	    {
		if(parr[i].stat==1)
		    n++;
	    }
	    if(a!=n)
		printf("no such job\n");
	    else
	    {
		id=parr[i-1].pid;

		while(wait(&st)!=id);
	    }
	    continue;
	}
	if(!strcmp(arg[0],"overkill")&&arg[1]==NULL)
	{
	    int i;
	    for(i=0;i<parrptr;i++)
	    {
		if(parr[i].stat)
		{
		    kill(parr[i].pid,SIGKILL);
		}
	    }
	    continue;
	}
	int flag;
	for(i=0;i<argc;i++)
	    if(!strcmp(arg[i],">")||!strcmp(arg[i],"<")||!strcmp(arg[i],">>"))
		flag=1;

	    else if(flag==1)
	    {
		if(fork())
		{
		    int pid;
		    wait(&pid);
		}
		else
		{
		    int f1,f2,f3;
		    int i,j1,j2,j3;
		    int rt=0;
		    int fd;
		    char* arr[100];
		    for(i=0;i<100;i++)
			arr[i]=NULL;
		    for(i=0;i<argc;i++)
		    {
			if(!strcmp(arg[i],"<"))
			{
			    j1=i;
			    f1=1;
			    fd=open(arg[j1+1],O_RDONLY,0666);
			    dup2(fd,0);
			}
			else if(!strcmp(arg[i],">"))
			{
			    j2=i;
			    f2=1;
			    fd=open(arg[j2+1],O_WRONLY|O_CREAT,0666);
			    dup2(fd,1);
			}
			else if(!strcmp(arg[i],">>"))
			{
			    j3=i;
			    f3=1;
			    fd=open(arg[j3+1],O_WRONLY|O_APPEND|O_CREAT,0666);
			    dup2(fd,1);
			}
			if(!f1&&!f2&&!f3)
			{
			    strcpy(arr[rt],arg[i]);
			    rt++;
			}
		    }
		    int j;
		    for(j=0;j<rt;j++)
			printf("%s\n",arr[i]);
		    execvp(arr[0],arr);
		    exit(1);
		}
	    }
	    else
	    {
		pid_t pid1=fork();
		if(pid1==-1){
		    perror("Error in creating fork : ");
		    exit(-1);
		}
		else if(pid1==0){
		    ret=execvp(arg[0],arg);
		    if (ret == -1)
		    {
			perror("no such command");
			fflush(stdout);
			_exit(1);
		    }
		}
		else if(pid1>0)
		{
		    //	printf("bg: %d\n",background);
		    if(background==1 && ret !=-1)
		    {
			curpid=-1;
			for(j=0;j<100;j++)
			    parr[parrptr].cname[j][0]='\0';
			parr[parrptr].pid=pid1;
			for(j=0;j<argc-1;j++)
			{
			    strcpy(parr[parrptr].cname[j],arg[j]);
			}
			parr[parrptr].stat=1;
			parr[parrptr].argcnt=argc-1;
			parrptr++;

		    }
		    if(background==0)
		    {

			waitpid(pid1,&status,WUNTRACED);
			if(WIFSTOPPED(status)) 
			{
			    parr[parrptr].pid = pid1;
			    parr[parrptr].stat = 1; 
			    for(i=0;i<argc;i++)
				strcpy(parr[parrptr].cname[i],arg[i]);
			    parr[parrptr].argcnt=argc;
			    parrptr++; 
			}
			/* parr[parrptr].pid=curpid;
			   for(i=0;i<argc;i++)
			   strcpy(parr[parrptr].cname[i],arg[i]);
			   parr[parrptr].stat=1;
			   parr[parrptr].argcnt=argc;
			   */ //    while(wait(&status)!=pid1)
			//	;
		    }
		}
	    }
	check_status();
    }
    for(i=0;i<parrptr;i++)
    {
	if(parr[i].stat)
	{
	    kill(parr[i].pid,SIGKILL);
	}
    }
    return 0;
}



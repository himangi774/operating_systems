#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>
#include<string.h>
typedef struct f course;
typedef struct p student; 
typedef struct f
{
    int num[4];
    int st[60];
    int grp;
}course;
typedef struct p
{
    int allocated;
    int branch;
    int crs[4];
    int pref[8];
}student;
course c[100];
student s[5000];
sem_t mutex[100];
void*allot(void*arg)
{
    int cnt=0,i;
    int n=((int)arg);
    int c0,c1,c2,c3,q1[8],q2[8],q3[8],q4[8];
    c0=c1=c2=c3=0;
    for(i=0;i<8;i++)
    {
//	printf("%d %d\n",n,s[n].pref[i]);
	switch((s[n].pref[i])%4)
	{
	    case 0:q1[c0++]=s[n].pref[i];
		   break;
	    case 1:q2[c1++]=s[n].pref[i];
		   break;
	    case 2:q3[c2++]=s[n].pref[i];
		   break;
	    case 3:q4[c3++]=s[n].pref[i];
		   break;
	}
    }
    if(!c0||!c1||!c2||!c3)
    {
	s[n].allocated=0;
	pthread_exit(NULL);
    }
    int f1,f2,f3,f4;
    f1=0;
    for(i=0;i<c0;i++)
    {
	sem_wait(&mutex[q1[i]]);
	if(s[n].branch==2&&c[q1[i]].num[s[n].branch]>=24)
	{
	    sem_post(&mutex[q1[i]]);
	    continue;
	}
	else if(c[q1[i]].num[s[n].branch]>=12)
	{
	    sem_post(&mutex[q1[i]]);
	    continue;
	}
	else
	{
	    f1=1;
	    c[q1[i]].num[s[n].branch]++;
//	    printf("1:%d %d\n",q1[i],c[q1[i]].num[s[n].branch]);
	    s[n].crs[cnt++]=q1[i];
	    sem_post(&mutex[q1[i]]);
	    break;
	}
    }
    if(!f1)
    {
	s[n].allocated=0;
	pthread_exit(NULL);
    }
    f2=0;
    for(i=0;i<c1;i++)
    {
	sem_wait(&mutex[q2[i]]);
	if(s[n].branch==2&&c[q2[i]].num[s[n].branch]>=24)
	{sem_post(&mutex[q2[i]]);
	    continue;
	}
	else if(c[q2[i]].num[s[n].branch]>=12)
	{
	    sem_post(&mutex[q2[i]]);
	    continue;
	}
	else
	{
	    f2=1;
	    c[q2[i]].num[s[n].branch]++;
	    //printf("2:%d %d\n",q2[i],c[q2[i]].num[s[n].branch]);
	    s[n].crs[cnt++]=q2[i];
	    sem_post(&mutex[q2[i]]);
	    break;
	}
    }
    if(!f2)
    {
	s[n].allocated=0;
	pthread_exit(NULL);
    }
    f3=0;
    for(i=0;i<c2;i++)
    {
	sem_wait(&mutex[q3[i]]);
	if(s[n].branch==2&&c[q3[i]].num[s[n].branch]>=24)
	{
	    sem_post(&mutex[q3[i]]);
	    continue;
	}
	else if(c[q3[i]].num[s[n].branch]>=12)

	{
	    sem_post(&mutex[q3[i]]);
	    continue;
	}
	else
	{
	    f3=1;
	    c[q3[i]].num[s[n].branch]++;
	    //printf("3:%d %d\n",q3[i],c[q3[i]].num[s[n].branch]);
	    s[n].crs[cnt++]=q3[i];
	    sem_post(&mutex[q3[i]]);
	    break;
	}
    }
    if(!f3)
    {
	s[n].allocated=0;
	pthread_exit(NULL);
    }
    f4=0;
    for(i=0;i<c3;i++)
    {
	sem_wait(&mutex[q4[i]]);
	if(s[n].branch==2&&c[q4[i]].num[s[n].branch]>=24)
	{
	    sem_post(&mutex[q4[i]]);
	    continue;
	}
	else if(c[q4[i]].num[s[n].branch]>=12)
	{
	    sem_post(&mutex[q4[i]]);
	    continue;
	}
	else
	{
	    f4=1;
	    c[q4[i]].num[s[n].branch]++;
	    //printf("4:%d %d\n",q4[i],c[q4[i]].num[s[n].branch]);
	    s[n].crs[cnt++]=q4[i];
	    sem_post(&mutex[q4[i]]);
	    break;
	}
    }
    if(!f4)
    {
	s[n].allocated=0;
	pthread_exit(NULL);
    }
    if(f1&&f2&&f3&&f4)
	s[n].allocated=1;
    pthread_exit(NULL);
}
int main(int argc,char**argv)
{
    int i,students,j,courses;
    pthread_t grp[1000];
    if(argc<3)
	exit(EXIT_FAILURE);
    students=atoi(argv[1]);
    courses=atoi(argv[2]);
    for(i=0;i<courses;i++)
    {
	for(j=0;j<4;j++)
	    c[i].num[j]=0;
	sem_init(&mutex[i],0,1);
	c[i].grp=i%4;
    }
    for(i=0;i<students;i++)
    {
	//printf("%d pref :",i);
	s[i].branch=rand()%4;
	for(j=0;j<8;j++)
	{ s[i].pref[j]=rand()%courses;
	   //printf("%d ",s[i].pref[j]);
	}
	//printf("\n");
	pthread_create(&grp[i],NULL,&allot,(void *)i);
    }
    for(i=0;i<students;i++)
    {
	pthread_join(grp[i],NULL);
    }
    printf("Students who got left\n");
    for(i=0;i<students;i++)
	if(s[i].allocated==0)
	    printf("%d\n",i);
    int dest;
    if ((dest = creat("./allocation.txt", 0600)) < 0) { //create destination file
	fprintf(stderr, "error creating dest file");//if file cant be created display error
	exit(-1);
    }
    int l;
    char buf[1000];
    for(i=0;i<courses;i++)
    {
	buf[0]='\0';
	int total;
	total=c[i].num[0]+c[i].num[1]+c[i].num[2]+c[i].num[3];
	if(total>60)
	    total=0;
	sprintf(buf,"course %d : %d\n",i,total);
	l=strlen(buf);
	write(dest,buf,l);
    }
    return 0;
}

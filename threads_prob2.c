#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>
#include<string.h>
sem_t mutex;
int s,wG=0,wNG=0;
void BoardBridge()
{
}
void GoBridge()
{
}
void* GeekArrives(void*arg)
{
    int si=0;
    sem_wait(&mutex);
    wG++;
    if(wG>=4)
    {
	wG-=4;
	sem_post(&mutex);
	printf("4 geeks\n");
	BoardBridge();
    }
    else if((wG>=2)&&(wNG>=2))
    {
	wG-=2;
	wNG-=2;
	sem_post(&mutex);
	printf("2 geeks 2 non-geeks\n");
	BoardBridge();
    }
    else if(s>0&&wG==3)
    {
	wG-=3;
	s--;
	sem_post(&mutex);
	printf("3 geeks 1 singer\n");
	BoardBridge();
    }
    else if(s>0&&(wG==1)&&(wNG>=2))
    {
	s--;
	wNG-=2;
	wG--;
	sem_post(&mutex);
	printf("1 geek 2 non-geeks 1 singer\n");
	BoardBridge();
    }
    else
    {
	sem_post(&mutex);
	return;
    }
    GoBridge();
}
void* NonGeekArrives(void*arg)
{
    sem_wait(&mutex);
    wNG++;
    if(wNG==4)
    {
	wNG-=4;
	sem_post(&mutex);
	printf("4 non-geeks\n");
	BoardBridge();
    }
    else if((wNG>=2)&&(wG>=2))
    {
	wNG-=2;
	wG-=2;
	sem_post(&mutex);
	printf("2 geeks 2 non-geeks\n");
	BoardBridge();
    }
    else if(s>0&&wNG==3)
    {
	s--;
	wNG-=3;
	sem_post(&mutex);
	printf("3 non-geeks 1 singer\n");
	BoardBridge();

    }
    else if((s>0)&&(wNG==1)&&(wG>=2))
    {
	s--;
	wG-=2;
	wNG--;
	sem_post(&mutex);
	printf("2 geeks 1 non-geek 1 singer\n");
	BoardBridge();
    }
    else
    {
	sem_post(&mutex);
	return;
    }
    GoBridge();
}

int main(int argc,char** argv)
{
    sem_init(&mutex,0,1);
    pthread_t G[1000],NG[1000];
    if(argc<4)
	exit(EXIT_FAILURE);
    int i,g,n;
    s=atoi(argv[3]);
    n=atoi(argv[2]);
    g=atoi(argv[1]);
    for(i=0;i<g;i++)
	pthread_create(&G[i],NULL,&GeekArrives,NULL);
    for(i=0;i<n;i++)
	pthread_create(&NG[i],NULL,&NonGeekArrives,NULL);
    for(i=0;i<g;i++)
    {
	pthread_join(G[i],NULL);
    }
    for(i=0;i<n;i++)
    {
	pthread_join(NG[i],NULL);
    }
    printf("left geeks:%d left non_geeks:%d left singers:%d\n",wG,wNG,s);
    return 0;
}


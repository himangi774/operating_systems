#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>
int sites[1000]={0},en,rxn=0,n,max_sites,H_l,O_l;
sem_t a_block,sites_block;
void* make_water()
{
    int i,tmp=-1,e=0;
    while(tmp==-1)
    {
	e=0;
	sem_wait(&sites_block);

	if(sites[0]==0&&sites[1]==0)
	    tmp=0;
	for(i=1;tmp==-1&&i<n-1;i++)
	{
	    if(!sites[i-1]&&!sites[i]&&!sites[i+1])
		tmp=i;
	}
	if(tmp==-1&&!sites[n-2]&&!sites[n-1])
	    tmp=n-1;
	for(i=0;i<n;i++)
	    if(sites[i])
	    e++;
	if(tmp!=-1)
	{
	    if(rxn<en)
	    {
		rxn++;
		sites[tmp]=1;
	    }
	    else
		tmp=-1;
	}
	sem_post(&sites_block);
    }
    sem_wait(&a_block);
    H_l-=2;
    O_l-=1;
    sleep(3);
    e++;
    printf("site:%d H_left:%d O_left:%d energy produced:%d\n",tmp,H_l,O_l,e);
    sem_post(&a_block);
    sem_wait(&sites_block);
    sites[tmp]=0;
    rxn--;
    sem_post(&sites_block);
    pthread_exit(NULL);
}
int main(int argc,char**argv)
{
    int i;
    pthread_t grp[1000];
    if(argc<5)
	exit(EXIT_FAILURE);
    int water,H,O;
    H=atoi(argv[1]);
    O=atoi(argv[2]);
    n=atoi(argv[3]);
    en=atoi(argv[4]);
    max_sites=en<(n+1)/2?en:(n+1)/2;
    water=H/2<O?H/2:O;
    H_l=H;
    O_l=O;
    sem_init(&sites_block,0,1);
    sem_init(&a_block,0,1);
    for(i=0;i<water;i++)
    {
	pthread_create(&grp[i],NULL,&make_water,NULL);
    }
    for(i=0;i<water;i++)
    {
	pthread_join(grp[i],NULL);
    }
}

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int P,Q,N,seed; double B1,B2,D1,D2,R;
typedef struct{double *a;int h,t,n,c;} Queue;

void input(){
    printf("Packet length (bytes): ");scanf("%d",&P);
    printf("Source-to-router bandwidth (bps): ");scanf("%lf",&B1);
    printf("Router-to-destination bandwidth (bps): ");scanf("%lf",&B2);
    printf("Source-to-router propagation delay: ");scanf("%lf",&D1);
    printf("Router-to-destination propagation delay: ");scanf("%lf",&D2);
    printf("Router processing delay: ");scanf("%lf",&R);
    printf("Router queue capacity: ");scanf("%d",&Q);
    printf("Packets per experiment: ");scanf("%d",&N);
    printf("Random seed: ");scanf("%d",&seed);
}

void sim(double l,double rho,char *name,FILE *f,double t1,double t2){
    Queue q={(double*)malloc(Q*sizeof(double)),0,0,0,Q};
    double now=0,ready=0,qd=0,e2e=0; int drop=0,max=0,del=0;

    for(int i=0;i<N;i++){
        double u=(double)rand()/RAND_MAX;
        now+=-log(u)/l;

        while(q.n&&q.a[q.h]<=now) q.h=(q.h+1)%Q,q.n--;

        if(q.n>max)max=q.n;
        if(q.n==Q){drop++;continue;}

        double s=now>ready?now:ready;
        qd+=s-now;
        e2e+=t1+D1+s-now+R+t2+D2;
        ready=s+R+t2;
        q.a[q.t]=ready;q.t=(q.t+1)%Q;q.n++;del++;
        if(q.n>max)max=q.n;
    }

    fprintf(f,"%s,%.2f,%.4f,%d,%d,%d,%.6f,%.6f,%.6f,%d\n",
        name,rho,l,N,del,drop,(double)drop/N,qd/del,e2e/del,max);
    free(q.a);
}

int main(){
    input();srand(seed);
    double t1=P*8.0/B1,t2=P*8.0/B2;
    FILE *f=fopen("simulation_results.csv","w");

    fprintf(f,"Experiment,Traffic Intensity (rho),Aggregate Lambda,Packets Generated,Packets Delivered,Packets Dropped,Drop Probability,Avg Queueing Delay,Avg End-to-End Delay,Max Queue Occupancy\n");

    double r[]={.2,.4,.6,.8,.9,1,1.2};
    for(int i=0;i<7;i++){
        char n[30]; double l=r[i]*B2/(P*8.0);
        sprintf(n,"Prob1_rho_%.2f",r[i]);
        sim(l,r[i],n,f,t1,t2);
    }

    double x[][4]={{100,100,100,700},{150,150,250,450},
                   {250,250,250,250},{400,300,200,100}};
    for(int i=0;i<4;i++){
        double l=x[i][0]+x[i][1]+x[i][2]+x[i][3];
        char n[30]; sprintf(n,"Prob2_Case_%d",i+1);
        sim(l,l*P*8/B2,n,f,t1,t2);
    }

    fclose(f);
    printf("Simulation complete! Results saved to 'simulation_results.csv'.\n");
}

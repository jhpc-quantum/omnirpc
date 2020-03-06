#include <stdio.h>
#include <stdlib.h>

#ifdef USE_OMRPC
#include "OmniRpc.h"
#endif

#define TRUE 1
#define FALSE 0

#define MAX_N 300
#define MAX_BREADTH 1000

int N;
int Cap;
int W[MAX_N];
int P[MAX_N];

int GLow = 0;

struct state {
    int i;
    int cp;
    int M;
} states[MAX_BREADTH];

void knap_dfs(int n, int W[],int P[], int i, int cp,int M,
	      int *g_lwb,int *ret);
int bound(int n,int W[],int P[],int i, int cp, int M,int *g_lwb);
void read_data_file(char *file);

double second();

int main(int argc,char *argv[])
{ 
    int r,rr,k; 
    double time,t;
    double t_max,t_min,t_total;
    int t_total_n;
    int breadth;

#ifdef USE_OMRPC
    OmniRpcInit(&argc, &argv);
#endif

    if (argc == 3){
      read_data_file(argv[1]);
      breadth = atoi(argv[2]);
    } else {
	printf("bad arg\n");
	exit(1);
    }

    printf("%s: breadth=%d, N=%d, Cap=%d\n",argv[1],breadth,N,Cap);

    time = second();

    /* breadth search */
    r = knap_bfs(Cap,breadth);
    printf("bfs end ...\n");

    t_max = 1.0E-10;
    t_min = 1.0E10;
    t_total = 0.0;
    t_total_n = 0;

    if(r < 0){
#pragma omp parallel for schedule(dynamic) private(k,rr,t)
	for(k = 0; k < breadth; k++){
	  t = second();
#ifdef USE_OMRPC
	    OmniRpcCall("knap_dfs",
			 N,W,P,states[k].i,states[k].cp,states[k].M,&GLow,&rr);
#else
	    knap_dfs(N,W,P,states[k].i,states[k].cp,states[k].M,&GLow,&rr);
#endif
	    t = second() - t;
#pragma omp critical 
	    {
	      /*
	      printf("k = %d, i=%d, cp=%d, M=%d, rr=%d, t=%g\n",
		     k,states[k].i,states[k].cp,states[k].M, rr, t);
	      */
		if(rr > r) r = rr;

		if(t > t_max) t_max = t;
		if(t < t_min) t_min = t;
		t_total += t;
		t_total_n++;
	    }
	}
    }

    time = second() - time;
    printf("Opt = %d\n", r);
    printf("exec. time = %g sec (max=%f, min=%f, avg=%f(%g/%d))\n", 
	   time,t_max,t_min,t_total/t_total_n,t_total,t_total_n);

#ifdef USE_OMRPC
    OmniRpcFinalize();
#endif
    exit(0);
}

void read_data_file(char *file)
{
    FILE *fp;
    int i;
    
    fp = fopen(file,"r");
    fscanf(fp,"%d",&N);
    fscanf(fp,"%d",&Cap);
    for(i = 0; i < N; i++)
      fscanf(fp,"%d",&W[i]);
    for(i = 0; i < N; i++)
      fscanf(fp,"%d",&P[i]);
    fclose(fp);
}

int bound(int n,int W[],int P[],int i, int cp, int M,int *g_lwb)
{
    int lwb, upb; 
    int ii,m;

    /* compute lower bound, simple estimation */
    lwb = cp;
    for(ii = i,m = M; (ii < n && m >= W[ii]); ii++){
	m -= W[ii];
	lwb += P[ii];
    }

    /* upper bound */
    if(ii < n) upb = lwb + (m*P[ii])/W[ii];
    else upb = lwb;
	
    if (*g_lwb < lwb){
	/* update GLow */
	*g_lwb = lwb;
    } else if(upb < *g_lwb) 
	return TRUE;
    return FALSE;
}

int knap_bfs(int Cap,int breadth)
{ 
    int first,last,i,M,cp;
    int Opt,total;

    states[0].i = 0;
    states[0].cp = 0;
    states[0].M = Cap;
    total = 1;
    first = last = 0;
    Opt = 0;
    while(total < breadth){
	i = states[first].i;
	M = states[first].M;
	cp = states[first].cp;
	if(cp > Opt) Opt = cp;
	if(i >= N || M <= 0 || bound(N,W,P,i,cp,M,&GLow)){
	    if(first == last) return Opt;
	    first = (first+1)%breadth;
	    total--;
	    continue;
	}
	if(M >= W[i]){
	    /* take case */
	    last = (last+1)%breadth;
	    states[last].i = i+1;
	    states[last].cp = cp + P[i];
	    states[last].M  = M - W[i];
	    last = (last+1)%breadth;
	    states[last].i = i+1;
	    states[last].cp = cp;
	    states[last].M  = M;
	    first = (first+1)%breadth;
	    total++;
	} else {
	    states[first].i = i+1;
	}
    }
    return -1;
}

/* 
 * knap_dfs: depth-first search
 * W,P : weight and price 
 *  i : search level 
 *  cp : current value
 *  M :  current capacity
 *  g_lwb : lower bound 
 *  r : return value
 */
void knap_dfs(int n, int W[],int P[],int i, int cp, int M,int *g_lwb,int *rp)
{ 
    int Opt; 
    int ii,m,l,r;
    
    Opt = cp;
    if (i < n && M > 0){
	if(bound(n,W,P,i,cp,M,g_lwb)) goto ret;

	if(M >= W[i]){
	    /* compute take case */
	    knap_dfs(n,W,P,i+1,cp+P[i],M-W[i],g_lwb,&l);
	    /* compute not-take case */
	    knap_dfs(n,W,P,i+1,cp,M,g_lwb,&r);
	    if(l > r) Opt = l;
	    else Opt = r;
	} else {
	    /* cannot take */
	    knap_dfs(n,W,P,i+1,cp,M,g_lwb,&r);
	    Opt = r;
	}
    }
ret:
    *rp = Opt;
}


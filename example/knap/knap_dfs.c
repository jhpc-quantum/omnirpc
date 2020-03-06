#define TRUE 1
#define FALSE 0

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

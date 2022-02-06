#ifndef _CRAND
#define _CRAND

int Seeded = 0;

int rand_int(int l, int u){
	if(!Seeded){
		time_t t;
		srand((unsigned) time(&t));
		Seeded = 1;
	}
	int out = (rand()% (u-l+1) +l);
	return(out);
}//rand_int






#endif



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mcmc/mcmc.h"
#include "jellyBean.h"


int main(int iargc, char *argv[]){

//d=8 -> delta_chisq=15.5
//d=5 -> delta_chisq=11

int i,j;
int doGuess=0;
double chiLimit=12.6;
int seed=99;
int nsamples=50000;
int burnin=3000;
int dim=10;
int nChains=4;
char nameroot[letters];

for(i=1;i<iargc;i++){
    if(argv[i][0]=='-'){
        switch(argv[i][1]){
            case 'h':
                printf("s = seed\nb = burnin\nn = nsamples\n");
                printf("d = dim\no = nameroot\nc = nChains\n");
                printf("g = doGuess (limit)\n");
                exit(1);
            case 's':
                i++;
                seed=atoi(argv[i]);
                break;
            case 'b':
                i++;
                burnin=atoi(argv[i]);
                break;
            case 'n':
                i++;
                nsamples=atoi(argv[i]);
                break;
            case 'd':
                i++;
                dim=atoi(argv[i]);
                break;
            case 'c':
                i++;
                nChains=atoi(argv[i]);
                break;
            case 'g':
                doGuess=1;
                i++;
                chiLimit=atof(argv[i]);
                break;
            case 'o':
                i++;
                for(j=0;j<letters-1 && argv[i][j]!=0;j++){
                    nameroot[j]=argv[i][j];
                }
                nameroot[j]=0;
                break;    
        }
    }
}


if(seed<0){
    seed=int(time(NULL));
    if(seed>10000)seed=seed%10000;
}


printf("seed %d\n",seed);

//declare the chisquared function APS will be searching
//ellipses_integrable chisq(dim,ncenters);

jellyBeanData chisq(dim,1,100,0.4,0.4,0.02,20.0);

//set the maximum and minimum values in parameter space
array_1d<double> max,min;
max.set_name("driver_max");
min.set_name("driver_min");

for(i=0;i<dim;i++){
    min.set(i,-30.0);
    max.set(i,30.0);
}


mcmc mcmc_test(nChains,seed,&chisq);
for(i=0;i<dim;i++){
    mcmc_test.set_min(i,min.get_data(i));
    mcmc_test.set_max(i,max.get_data(i));
}

if(doGuess==1){
    mcmc_test.guess_bases(chiLimit,1);
}
mcmc_test.set_burnin(burnin);
mcmc_test.set_name_root(nameroot);
mcmc_test.sample(nsamples);

}

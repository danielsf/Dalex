#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "carom.h"

double naiveDistance(array_1d<double> &p1, array_1d<double> &p2){
    if(p1.get_dim()!=p2.get_dim()){
        printf("in naive distance\n");
        printf("WARNING p1 d %d p2 d %d\n",p1.get_dim(),p2.get_dim());
        exit(1);
    }
    
    int i;
    double ans=0.0;
    for(i=0;i<p1.get_dim();i++){
        ans+=power(p1.get_data(i)-p2.get_data(i),2);
    }
    return sqrt(ans);
}

int main(int iargc, char *argv[]){

//d=8 -> delta_chisq=15.5
//d=5 -> delta_chisq=11

int i,j;
int seed=99;
int dim,ncenters;
int nsamples=10000;

dim=22;
ncenters=3;

if(iargc>1)seed=atoi(argv[1]);
if(iargc>2)ncenters=atoi(argv[2]);
if(iargc>3)dim=atoi(argv[3]);

if(iargc>4){
    nsamples=atoi(argv[4]);
}

if(seed<0){
    seed=int(time(NULL));
    if(seed>10000)seed=seed%10000;
}

char timingname[letters],outname[letters];

//what is the name of the file where APS will store its timing information
sprintf(timingname,"output/s_curve_d%d_c%d_s%d_timing.sav",dim,ncenters,seed);

//what is the name of the file where APS will output the points it sampled
sprintf(outname,"output/s_curve_d%d_c%d_s%d_output.sav",dim,ncenters,seed);

printf("seed %d\n",seed);

//declare the chisquared function APS will be searching
//ellipses_integrable chisq(dim,ncenters);

s_curve chisq(dim,ncenters);

array_2d<double> troughPoints,borderPoints;
troughPoints.set_name("troughPoints");
borderPoints.set_name("borderPoints");


chisq.get_trough_points(troughPoints);
printf("got trough points\n");
chisq.get_border_points(borderPoints);
printf("got border points\n");

double chi,chimax;
for(i=0;i<troughPoints.get_rows();i++){
   chi=chisq(*troughPoints(i));
   if(i==0 || chi>chimax)chimax=chi;
}
printf("after troughPoints chimax %e\n",chimax);
for(i=0;i<borderPoints.get_rows();i++){
    chi=fabs(33.93-chisq(*borderPoints(i)));
    if(i==0 || chi>chimax){
        chimax=chi;
    }
}
printf("after borderPoints dchimax %e\n",chimax);

printf("trough %d\n",troughPoints.get_rows());
printf("border %d\n",borderPoints.get_rows());


//declare APS
//the '20' below is the number of nearest neighbors to use when seeding the
//Gaussian process
//
//the '11.0' is the \Delta\chi^2 corresponding to a 95% confidence limit
//on a 5-dimensional parameter space

carom carom_test;
carom_test.set_deltachi(33.9);
carom_test.set_seed(seed);

//pass chisq to the aps object
carom_test.set_chisquared(&chisq);

//how often will APS stop and write its output
carom_test.set_write_every(3000);

//set the maximum and minimum values in parameter space
array_1d<double> max,min;
max.set_name("driver_max");
min.set_name("driver_min");
max.set_dim(dim);
min.set_dim(dim);

for(i=0;i<dim;i++){
    min.set(i,-200.0);
    max.set(i,200.0);
}


carom_test.set_timingname(timingname);
carom_test.set_outname(outname);
carom_test.set_min(min);
carom_test.set_max(max);

//initialize aps with 1000 random samples
printf("time to initialize\n");
chisq.reset_timer();
carom_test.initialize(1000);
carom_test.search();

/*
double chival,chivaltest,err;

i=-1;

//search parameter space until the
//chisquared function has been called
//10000 times
while(carom_test.get_called()<nsamples){
    carom_test.search();    
}
carom_test.write_pts();

array_1d<double> minpt;

//what is the point in parameter space corresponding to the
//minimum chisquared
carom_test.get_minpt(minpt);

printf("chimin %e\n",carom_test.get_chimin());

printf("ct_aps %d ct_simplex %d total %d\n",
carom_test.get_ct_aps(),carom_test.get_ct_simplex(),
carom_test.get_called());

array_2d<double> goodPoints;
goodPoints.set_name("global_goodPoints");
carom_test.get_good_points(goodPoints);

printf("got goodPoints %d\n",goodPoints.get_rows());

array_1d<int> troughFound,borderFound;
troughFound.set_name("global_troughFound");
borderFound.set_name("global_borderFound");

for(i=0;i<troughPoints.get_rows();i++){
    troughFound.set(i,0);
}
for(i=0;i<borderPoints.get_rows();i++){
    borderFound.set(i,0);
}

int iborder,itrough;
double dd,ddmin;
for(i=0;i<goodPoints.get_rows();i++){
    for(j=0;j<troughPoints.get_rows();j++){
        dd=naiveDistance(goodPoints(i)[0],troughPoints(j)[0]);
        if(j==0 || dd<ddmin){
            itrough=j;
            ddmin=dd;
        }
    }
    troughFound.set(itrough,1);
    
    for(j=0;j<borderPoints.get_rows();j++){
        dd=naiveDistance(goodPoints(i)[0],borderPoints(j)[0]);
        if(j==0 || dd<ddmin){
            iborder=j;
            ddmin=dd;
        }
    }
    borderFound.set(iborder,1);
}

iborder=0;
for(i=0;i<borderFound.get_dim();i++){
    if(borderFound.get_data(i)==0)iborder++;
}

itrough=0;
for(i=0;i<troughFound.get_dim();i++){
    if(troughFound.get_data(i)==0)itrough++;
}

printf("border did not find %d of %d\n",
iborder,borderPoints.get_rows());
printf("trough did not find %d of %d\n",
itrough,troughPoints.get_rows());
*/
}

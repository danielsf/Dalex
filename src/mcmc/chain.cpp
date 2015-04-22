#include "mcmc/chain.h"

chain::~chain(){}

void chain::initialize(){
    _dim=0;
    _n_written=0;
    _output_name[0]=0;
    _points.set_name("chain_points");
    _degeneracy.set_name("chain_degeneracy");
    _chisquared.set_name("chain_chisquared");
}

void chain::set_dim(int ii){
    _dim=ii;
    _points.reset();
    _degneracy.reset();
    _chisquared.reset();
    
    _points.set_cols(_dim);
}

void chain::set_output_name(char *nn){
    int i;
    for(i=0;i<letters-1 && nn[i]!=0;i++){
        _output_name[i]=nn[i];
    }
    _output_name[i]=0;
}

chain::chain(){
    intialize();    
}

chain::chain(int ii){
    initialize();
    set_dim(ii);
}

chain::chain(int ii, char *input_name){
    initialize();
    set_dim(ii);
    
    int i,ct;
    double chisq,mu;
    array_1d<double> vv;
    vv.set_name("chain_constructor_vv");
    
    FILE *input;
    input=fopen(input_name,"r");
    while(fscanf(input,"%d",&ct)){
        _degeneracy.add(ct);
        fscanf(input,"%le",&chisq);
        _chisquared.add(chisq);
        
        for(i=0;i<_dim;i++){
            fscanf(input,"%le",&mu);
            vv.set(i,mu);
        }
        _points.add_row(vv);
    }
    
    fclose(input);
}

void chain::verify_dim(int dex, char *routine){
    if(dex<0 || dex>=_dim){
        printf("WARNING chain::%s\n",routine);
        printf("%d but dim %d\n",dex,_dim);
        exit(1);
    }
}

void chain::verify_points(int dex, char *routine){
    if(dex<0 || dex>=_points.get_rows()){
        printf("WARNING chain::%s\n",routine);
        printf("%d but points %d\n",dex,_points.get_rows());
        exit(1);
    }
}

double chain::get_current_point(int dex){
    verify_dim(dex,"get_current_point");
    
    return _points.get_data(_points.get_rows()-1,dex);
}

double chain::get_current_chisquared(){
    return _chisquared.get_data(_chisquared.get_dim()-1);
}

int chain::get_current_degeneracy(){
    return _degeneracy.get_data(_degeneracy.get_dim()-1);
}

int chain::get_points(){
    return _points.get_rows();
}

int chain::get_dim(){
    return _dim;
}

double chain::get_point(int dex, int dim){
    verify_dim(dim,"get_point");
    verify_points(dex,"get_point");
    return _points.get_data(dex,dim);
}

double chain::get_chisquared(int dex){
    verify_points(dex,"get_chisquared");
    return _chisquared.get_data(dex);
}

int chain::get_degeneracy(int dex){
    verify_points(dex,"get_degeneracy");
    return _degeneracy.get_data(dex);
}

void chain::add_point(array_1d<double> &pt, double mu){
    _points.add_row(pt);
    _chisquared.add(mu);
    _degeneracy.add(1);
}

void chain::write_chain(){
    if(_output_name[0]==0){
        printf("WARNING asked to write chain but have no name\n");
        exit(1);
    }
    
    FILE *output;
    if(_n_written==0){
        output=fopen(_output_name,"w");
    }
    else{
        output=fopen(_output_name,"a");
    }
    
    int i,j;
    for(i=0;i<_points.get_rows();i++){
        fprintf(output,"%d %e ",_degeneracy.get_data(i),_chisquared.get_data(i));
        for(j=0;j<_dim;j++){
            fprintf(output,"%e ",_points.get_data(i,j));
        }
        fprintf(output,"\n");
    }
    fclose(output);
    
    _n_written+=_points.get_rows();
    
    _points.reset_preserving_room();
    _degeneracy.reset_preserving_room();
    _chisquared.reset_preserving_room();
}

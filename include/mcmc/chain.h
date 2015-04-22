#ifndef CHAIN_H
#define CHAIN_H

#include "containers.h"

class chain{

public:

    ~chain();

    chain();
    chain(int);
    chain(int, char*);
    
    void set_dim(int);
    void set_output_name(char*);
    
    double get_current_point(int);
    double get_current_chisquared();
    int get_current_degeneracy();
    
    int get_points();
    int get_dim();
    double get_point(int,int);
    double get_chisquared(int);
    int get_degeneracy(int);
    
    void add_point(array_1d<double>&, double);
    
    void write_chain();

private:

    int _dim,_n_written;
    array_2d<double> _points;
    array_1d<int> _degeneracy;
    array_1d<double> _chisquared;
    
    char _output_name[letters];

    void initialize();
    void verify_dim(int,char*);
    void verify_points(int,char*);

};

#endif

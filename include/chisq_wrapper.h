#ifndef CHISQ_WRAPPER_H
#define CHISQ_WRAPPER_H

#include <math.h>
#include <time.h>
#include <stdio.h>

#include "goto_tools.h"
#include "containers.h"
#include "kd.h"
#include "wrappers.h"
#include "chisq.h"

class chisq_wrapper : public function_wrapper{

public:
    chisq_wrapper();
    ~chisq_wrapper();
    void copy(chisq_wrapper&);
    
    void initialize(int);
    
    void set_chisquared(chisquared*);

    void set_target(double);
    void set_seed(int);
    void set_deltachi(double);
    void set_characteristic_length(int, double);
    void set_min(array_1d<double>&);
    void set_max(array_1d<double>&);
    void set_ddmin(double);
    
    int could_it_go_lower(double);
    
    double target();
    double chimin();
    double get_deltachi();
    int get_pts();
    int get_dim();
    virtual int get_called();
    double get_time_spent();
    
    double random_double();
    int random_int();

    void evaluate(array_1d<double>&, double*, int*);
    virtual double operator()(array_1d<double>&);
    double get_fn(int);
    double get_pt(int,int);
    array_1d<double>* get_pt(int);
    
    double distance(array_1d<double>&,int);
    double distance(array_1d<double>&,array_1d<double>&);
    double distance(int,int);
    
    void nn_srch(array_1d<double>&,int,array_1d<int>&,array_1d<double>&);
    
    Ran* get_dice();
    
    void get_min(array_1d<double>&);
    void get_max(array_1d<double>&);
    double get_min(int);
    double get_max(int);
    
    void find_gradient(array_1d<double>&,array_1d<double>&);
    void set_iWhere(int);
    int get_ct_where(int);
    int get_where_log(int);

    int in_bounds(array_1d<double>&);
    int in_bounds(int, double);

    void set_confidence_limit(double);
    void set_dof(int);

    kd_tree* get_tree();

private:
    double _chimin,_deltachi,_target,_ddmin;
    double _expected_min,_expected_delta,_confidence_limit;
    int _adaptive_target,_seed,_called,_mindex,_iWhere;
    int _dof;
    
    array_1d<double> _characteristic_length,_range_min,_range_max,_fn;
    
    chisquared *_chifn;
    kd_tree *_kptr;
    Ran *_dice;
    
    int is_valid(array_1d<double>&, int*);
    void is_it_safe(char*);
    
    array_1d<double> _valid_dd;
    array_1d<int> _valid_neigh,_ct_where,_where_log;

    chisquared_distribution _distribution;
};

#endif
#ifndef MCMC_H
#define MCMC_H

#include "mcmc/chain.h"
#include "chisq.h"
#include "simplex.h"
#include "eigen_wrapper.h"

class mcmc{

public:

    ~mcmc();
    mcmc(int, int, chisquared*);
    void set_check_every(int);
    void set_name_root(char*);
    void set_min(int,double);
    void set_max(int,double);
    void guess_bases(int);
    double acceptance_rate();
    void update_bases();
    void write_burnin();
    void write_chains();


private:

    arrayOfChains _chains;
    chisquared *_chisq;
    Ran *_dice;

    array_2d<double> _bases;
    array_1d<double> _sigma,_guess_max,_guess_min;

    char _name_root[letters];

    int _check_every,_last_set,_stable_bases;

    void initialize();
    void find_fisher_eigen(array_2d<double>&, array_1d<double>&, double*);
    void find_fisher_matrix(array_2d<double>&, array_1d<double>&, double*);
    void bisection(array_1d<double>&, double, array_1d<double>&, array_1d<double>&);

};

#endif

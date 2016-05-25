#ifndef EXPLORERS_H
#define EXPLORERS_H

#include "goto_tools.h"
#include "chisq_wrapper.h"
#include "dchi_simplex.h"


class explorers{

    public:
        ~explorers(){}

        explorers(){
            _chifn=NULL;
            _mindex=-1;
            _temp=1.0;
            _associates.set_name("explorers_associates");
            _particles.set_name("explorers_particles");
            _bases.set_name("explorers_bases");
            _norm.set_name("explorers_norm");
            _req_temp.set_name("explorers_req_temp");
            _accepted.set_name("explorers_accepted");
        }

        void set_n_particles(int ii){
            _n_particles=ii;
        }

        void set_chifn(chisq_wrapper *cc){
            _chifn=cc;
        }

        void set_associates(array_1d<int> aa){
            int i;
            _associates.reset_preserving_room();
            for(i=0;i<aa.get_dim();i++){
                _associates.set(i,aa.get_data(i));
            }
        }

        void get_min_pt(array_1d<double> &pp){
            int i;
            for(i=0;i<_chifn->get_dim();i++){
                pp.set(i,_particles.get_data(_mindex,i));
            }
        }

        void set_bases();
        void initialize_particles();
        void sample(int, array_2d<double>&);

    private:
        chisq_wrapper *_chifn;
        array_1d<int> _associates;
        array_2d<double> _bases;
        array_1d<double> _norm;
        array_1d<double> _req_temp;
        int _mindex;
        double _mu_min;
        int _n_particles;
        double _temp;
        array_1d<int> _accepted;
        int _attempted;
        array_2d<double> _particles;



};

#endif

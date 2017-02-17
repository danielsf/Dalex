#ifndef COST_FN_H
#define COST_FN_H

#include "chisq_wrapper.h"

class cost_fn : public function_wrapper{
    public:
        cost_fn(chisq_wrapper*, array_1d<int>&);
        ~cost_fn(){};
        virtual double operator()(const array_1d<double>&);
        virtual int get_called();
        double nn_distance(const array_1d<double>&);
        void set_envelope(double dd){
            _envelope=dd;
        }

        void copy_bases(array_2d<double> &out){
            int i;
            out.reset_preserving_room();
            for(i=0;i<_bases.get_rows();i++){
                out.add_row(_bases(i));
            }
        }

        double get_norm(int ii){
            return _norm.get_data(ii);
        }

        int get_cached_values(const int dex,
                              double *fn, double *chisq){

            int i;
            for(i=0;i<_pt_cache.get_dim();i++){
                if(_pt_cache.get_data(i)==dex){
                    chisq[0]=_chisq_cache.get_data(i);
                    fn[0]=_fn_cache.get_data(i);
                    return 1;
                }
            }

            return 0;

        }

    private:
        array_1d<int> _associates;
        array_1d<double> _median_associate;
        double _scalar_norm;
        chisq_wrapper *_chifn;
        int _called;
        double _envelope;

        array_2d<double> _bases;

        void _principal_set_bases();
        void _random_set_bases();

        void _set_bases(){
            if(_associates.get_dim()<_chifn->get_dim()){
                _random_set_bases();
            }
            else{
                _principal_set_bases();
            }
        }


        void _set_norm();
        array_1d<double> _norm;

        array_1d<int> _pt_cache;
        array_1d<double> _fn_cache,_chisq_cache;

};

#endif

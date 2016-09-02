#include "explorers.h"

void explorers::get_seed(array_2d<double> &seed){

    array_1d<int> mu_dex;
    mu_dex.set_name("exp_seed_mu_dex");
    array_1d<double> mu_sorted;
    mu_sorted.set_name("exp_mu_sorted");

    int i;
    for(i=0;i<_mu_arr.get_dim();i++){
        mu_dex.set(i,i);
    }
    sort_and_check(_mu_arr,mu_sorted,mu_dex);

    int j,k;
    double rr;

    for(i=0;i<_chifn->get_dim()+1;i++){
        seed.add_row(_particles(mu_dex.get_data(i))[0]);
        for(j=0;j<_chifn->get_dim();j++){
            _particles.set(mu_dex.get_data(i),j,_median_associate.get_data(j));
        }
        for(j=0;j<_chifn->get_dim();j++){
            rr=normal_deviate(_chifn->get_dice(),0.0,2.0);
            for(k=0;k<_chifn->get_dim();k++){
                _particles.add_val(mu_dex.get_data(i),k,rr*_norm.get_data(j)*_bases.get_data(j,k));
            }
        }
    }
}

void explorers::set_norm(){

    int ip,ia;
    double component;
    int i,j;

    _min.reset_preserving_room();
    _max.reset_preserving_room();
    for(ip=0;ip<_associates.get_dim();ip++){
        ia=_associates.get_data(ip);
        for(i=0;i<_chifn->get_dim();i++){
            component=0.0;
            for(j=0;j<_chifn->get_dim();j++){
                component+=_chifn->get_pt(ia,j)*_bases.get_data(i,j);
            }
            if(i>=_min.get_dim() || component<_min.get_data(i)){
                _min.set(i,component);
            }
            if(i>=_max.get_dim() || component>_max.get_data(i)){
                _max.set(i,component);
            }
        }
    }

    for(i=0;i<_chifn->get_dim();i++){
       _median_associate.set(i,0.0);
    }

    for(i=0;i<_chifn->get_dim();i++){
        _norm.set(i,0.2*(_max.get_data(i)-_min.get_data(i)));
        for(j=0;j<_chifn->get_dim();j++){
             _median_associate.add_val(j,_bases.get_data(i,j)*0.5*(_max.get_data(i)+_min.get_data(i)));
        }
    }

}

void explorers::initialize_particles(){

    _particles.reset();
    _attempted=0;
    _accepted.reset();
    _req_temp.reset();

    array_1d<double> min,max;
    min.set_name("exp_init_min");
    max.set_name("exp_init_max");

    int i,j;
    double xx;
    for(i=0;i<_associates.get_dim();i++){
        for(j=0;j<_chifn->get_dim();j++){
            xx=_chifn->get_pt(_associates.get_data(i),j);
            if(i==0 || xx<min.get_data(j)){
                min.set(j,xx);
            }
            if(i==0 || xx>max.get_data(j)){
                max.set(j,xx);
            }
        }
    }

    array_1d<double> trial;
    trial.set_name("exp_init_trial");
    int i_found;
    double span,mu;

    while(_particles.get_rows()<_n_particles){
        for(i=0;i<_chifn->get_dim();i++){
            span=max.get_data(i)-min.get_data(i);
            //sfd this should be 2.0*(double()-0.5)*span
            trial.set(i,(2.0*_chifn->random_double()-0.5)*span+min.get_data(i));
        }
        _chifn->evaluate(trial,&mu,&i_found);
        if(i_found>=0){
            _particles.add_row(trial);
            _accepted.add(0);
        }
    }

}


void explorers::bump_particles(){
    array_1d<double> dir, trial;
    dir.set_name("exp_bump_dir");
    trial.set_name("exp_bump_trial");
    _particles.reset_preserving_room();

    int i,j;
    while(_particles.get_rows()<_n_particles){
        for(i=0;i<_chifn->get_dim();i++){
            dir.set(i,normal_deviate(_chifn->get_dice(),0.0,1.0));
        }
        dir.normalize();
        for(i=0;i<_chifn->get_dim();i++){
            trial.set(i,_chifn->get_pt(_chifn->mindex(),i));
        }
        for(i=0;i<_chifn->get_dim();i++){
            for(j=0;j<_chifn->get_dim();j++){
                trial.add_val(j,1.0*(_max.get_data(i)-_min.get_data(i))
                               *dir.get_data(i)*_bases.get_data(i,j));
            }
        }
        _particles.add_row(trial);
    }
}


void explorers::sample(int n_steps){

    if(_particles.get_rows()!=_n_particles){
        initialize_particles();
    }

    dchi_interior_simplex dchifn(_chifn, _associates);
    dchifn.copy_bases(_bases);
    set_norm();

    _mindex=-1;

    double mu;
    int i_step;
    int i_dim,ip;
    int i;
    int accept_it;
    double rr,roll,ratio;
    array_1d<double> trial;
    trial.set_name("exp_sample_trial");

    for(ip=0;ip<_n_particles;ip++){
        mu=dchifn(_particles(ip)[0]);
        if(ip==0 || mu<_mu_min){
            _mindex=ip;
            _mu_min=mu;
        }
        _mu_arr.set(ip,mu);
    }


    double needed_temp;
    double old_temp;
    array_1d<int> req_temp_dex;
    array_1d<double> req_temp_sorted;
    req_temp_dex.set_name("exp_sample_req_temp_dex");
    req_temp_sorted.set_name("exp_sample_req_temp_sorted");

    int has_been_adjusted;

    array_1d<double> acceptance_rate,acceptance_rate_sorted;
    array_1d<int> acceptance_rate_dex;
    acceptance_rate.set_name("exp_sample_acceptance_rate");
    acceptance_rate_sorted.set_name("exp_sample_acceptance_rate_sorted");
    acceptance_rate_dex.set_name("exp_sample_acceptance_rate_dex");

    double med_acc;

    printf("    starting sampling with %e\n",_mu_min);

    int scalar_acceptance=0;

    for(i_step=0;i_step<n_steps;i_step++){

        for(ip=0;ip<_n_particles;ip++){
            i_dim=_chifn->random_int()%_chifn->get_dim();
            rr=normal_deviate(_chifn->get_dice(),0.0,1.0);
            for(i=0;i<_chifn->get_dim();i++){
                trial.set(i,_particles.get_data(ip,i)+
                          rr*_norm.get_data(i_dim)*_bases.get_data(i_dim,i));
            }
            accept_it=0;
            mu=dchifn(trial);
            if(mu<_mu_arr.get_data(ip)){
                accept_it=1;
                needed_temp=1.0;
            }
            else{
                ratio=exp(-0.5*(mu-_mu_arr.get_data(ip))/_temp);
                roll=_chifn->random_double();
                needed_temp=-0.5*(mu-_mu_arr.get_data(ip))/log(roll);
                if(needed_temp<1.0){
                    needed_temp=1.0;
                }
                if(roll<ratio){
                    accept_it=1;
                }
            }

            _req_temp.add(needed_temp);

            if(accept_it==1){
                scalar_acceptance++;
                _accepted.add_val(ip,1);
                _mu_arr.set(ip,mu);
                for(i=0;i<_chifn->get_dim();i++){
                    _particles.set(ip,i,trial.get_data(i));
                }
                if(mu<_mu_min){
                     _mindex=ip;
                     _mu_min=mu;
                }
            }

        }
        _attempted++;

        if(_attempted>0 && _attempted%(2*_chifn->get_dim())==0){
            acceptance_rate_dex.reset_preserving_room();
            acceptance_rate_sorted.reset_preserving_room();
            for(i=0;i<_accepted.get_dim();i++){
                acceptance_rate.set(i,double(_accepted.get_data(i))/double(_attempted));
                acceptance_rate_dex.set(i,i);
            }
            sort_and_check(acceptance_rate, acceptance_rate_sorted, acceptance_rate_dex);
            med_acc=acceptance_rate_sorted.get_data(acceptance_rate_dex.get_dim()/2);
            if(med_acc>0.75 || med_acc<0.3333){
                old_temp=_temp;
                req_temp_sorted.reset_preserving_room();
                req_temp_dex.reset_preserving_room();
                for(i=0;i<_req_temp.get_dim();i++){
                    req_temp_dex.set(i,i);
                }
                sort_and_check(_req_temp, req_temp_sorted, req_temp_dex);
                _temp=req_temp_sorted.get_data(req_temp_dex.get_dim()/2);
                if(fabs(1.0-old_temp/_temp)>0.01){
                    _req_temp.reset();
                    _accepted.zero();
                    _attempted=0;
                }
            }
        }

    }

    printf("    sampling min %e\n",_mu_min);
    printf("    temp %e\n",_temp);
    printf("    accepted %d steps %d\n",scalar_acceptance,
    _n_particles*n_steps);
}
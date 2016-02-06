#include "maps.h"

maps::maps(){
    _write_every=3000;
    _last_wrote_log=-1;
    _last_written=0;
    _ct_simplex=0;
    _ct_simplex_min=0;
    _ct_mcmc=0;
    _init_mcmc=0;
    _last_did_min=0;
    _calls_to_simplex=0;
    _log.set_name("carom_log");
    sprintf(_outname,"output/carom_output.sav");
    sprintf(_timingname,"output/carom_timing.sav");
    _time_started=double(time(NULL));
    _good_points.set_name("maps_good_points");
    _duds.set_name("maps_duds");
}

maps::~maps(){
    write_pts();

}

double maps::evaluate(array_1d<double> &pt, int *dex){
    double mu;
    _chifn.evaluate(pt,&mu,dex);

    if(mu<_chifn.target() && dex[0]>=0){
        if(_good_points.contains(dex[0])==0){
            _good_points.add(dex[0]);
        }
    }

    return mu;
}

void maps::assess_good_points(){
    assess_good_points(0,0);
}

void maps::assess_good_points(int i_min){
    assess_good_points(i_min,_chifn.get_pts());
}

void maps::assess_good_points(int i_min, int i_max){
    int i;

    for(i=0;i<_good_points.get_dim();i++){
        if(_chifn.get_fn(_good_points.get_data(i))>_chifn.target()){
            _good_points.remove(i);
            i--;
        }
    }

    for(i=i_min;i<i_max+1 && i<_chifn.get_pts();i++){
        if(_chifn.get_fn(i)<_chifn.target()){
            if(_good_points.contains(i)==0){
                _good_points.add(i);
            }
        }
    }
}

void maps::set_dof(int dd){
    _chifn.set_dof(dd);
}

void maps::set_confidence_limit(double cc){
    _chifn.set_confidence_limit(cc);
}

void maps::set_seed(int ii){
    _chifn.set_seed(ii);
}

void maps::set_min(array_1d<double> &vv){
    _chifn.set_min(vv);
}

void maps::set_max(array_1d<double> &vv){
    _chifn.set_max(vv);
}

void maps::set_characteristic_length(int dex, double vv){
    _chifn.set_characteristic_length(dex,vv);
}

void maps::set_chisquared(chisquared *xx){
    _chifn.set_chisquared(xx);
}

void maps::set_deltachi(double xx){
    _chifn.set_deltachi(xx);
}

void maps::set_target(double tt){
    _chifn.set_target(tt);
}

void maps::set_write_every(int ww){
    _write_every=ww;
}

int maps::get_called(){
    return _chifn.get_called();
}

void maps::set_outname(char *nn){
    int i;
    for(i=0;i<letters-1 && nn[i]!=0;i++){
        _outname[i]=nn[i];
    }
    _outname[i]=0;
}

void maps::set_timingname(char *nn){
    int i;
    for(i=0;i<letters-1 && nn[i]!=0;i++){
        _timingname[i]=nn[i];
    }
    _timingname[i]=0;
}

void maps::initialize(int npts){
    _chifn.initialize(npts);
    assess_good_points(0);
    _interpolator.set_kd_fn(_chifn.get_tree(), _chifn.get_fn_arr());
    write_pts();
}


void maps::write_log(){

    FILE *output;
    char log_name[2*letters],suffix[letters];
    array_1d<int> types;
    types.set_name("carom_write_log_types");
    types.add(_log_ricochet);
    types.add(_log_mcmc);
    types.add(_log_dchi_simplex);
    types.add(_log_simplex);
    types.add(_log_compass);
    types.add(_log_swarm);
    int i,j,ii;

    for(ii=0;ii<types.get_dim();ii++){
        if(types.get_data(ii)==_log_ricochet){
            sprintf(log_name,"%s_ricochet_log.txt",_outname);
        }
        else if(types.get_data(ii)==_log_mcmc){
            sprintf(log_name,"%s_mcmc_log.txt",_outname);
        }
        else if(types.get_data(ii)==_log_dchi_simplex){
            sprintf(log_name,"%s_dchi_simplex_log.txt",_outname);
        }
        else if(types.get_data(ii)==_log_simplex){
           sprintf(log_name,"%s_simplex_log.txt",_outname);
        }
        else if(types.get_data(ii)==_log_compass){
            sprintf(log_name,"%s_compass_log.txt",_outname);
        }
        else if(types.get_data(ii)==_log_swarm){
            sprintf(log_name,"%s_swarm_log.txt",_outname);
        }
        else{
            printf("WARNING asked for unknown log type %d\n",types.get_data(ii));
            exit(1);
        }

        if(_last_wrote_log<0){
            output=fopen(log_name,"w");
        }
        else{
            output=fopen(log_name,"a");
        }

        for(i=0;i<_log.get_cols(types.get_data(ii));i++){
            for(j=0;j<_chifn.get_dim();j++){
                fprintf(output,"%e ",_chifn.get_pt(_log.get_data(types.get_data(ii),i),j));
            }
            fprintf(output,"%e %d\n",
            _chifn.get_fn(_log.get_data(types.get_data(ii),i)),
            _log.get_data(types.get_data(ii),i));
        }

        fclose(output);
    }

    _last_wrote_log=_chifn.get_pts();
    _log.reset_preserving_room();

}


void maps::write_pts(){
    FILE *output;

    int i,j;
    output=fopen(_outname,"w");
    fprintf(output,"# ");
    for(i=0;i<_chifn.get_dim();i++){
        fprintf(output,"p%d ",i);
    }
    fprintf(output,"chisq mu sig ling\n");
    for(i=0;i<_chifn.get_pts();i++){
        for(j=0;j<_chifn.get_dim();j++){
            fprintf(output,"%.18e ",_chifn.get_pt(i,j));
        }
        fprintf(output,"%.18e 0 0 0\n",_chifn.get_fn(i));
    }
    fclose(output);

    if(_last_written==0){
        output=fopen(_timingname,"w");
        fprintf(output,"#seed %d\n",_chifn.get_seed());
    }
    else{
        output=fopen(_timingname,"a");
    }

    fprintf(output,"%d %d min %.4e target %.4e -- timing -- %.4e %.4e -- %.4e %.4e -- overhead %.4e -- %d -- ",
        _chifn.get_pts(),
        _chifn.get_called(),
        _chifn.chimin(),
        _chifn.target(),
        double(time(NULL))-_time_started,
        (double(time(NULL))-_time_started)/double(_chifn.get_pts()),
        _chifn.get_time_spent(),
        _chifn.get_time_spent()/double(_chifn.get_pts()),
        (double(time(NULL))-_time_started-_chifn.get_time_spent())/double(_chifn.get_pts()),
        _calls_to_simplex);

    fprintf(output,"\n");
    fclose(output);

    write_log();
    _last_written=_chifn.get_pts();

}

void maps::simplex_min_search(){
    printf("\ndoing maps.simplex_min_search -- min %e\n",_chifn.chimin());
    int pt_start=_chifn.get_pts();

    int previous_mindex;
    double mu_previous_min,mu;

    assess_good_points();

    array_1d<int> local_associates;
    int i,j;

    if(_chifn.get_pts()==_last_did_min){
        return;
    }

    if(_ct_simplex_min>0){
        for(i=0;i<_good_points.get_dim();i++){
            local_associates.add(_good_points.get_data(i));
        }
    }

    array_1d<double> norm;
    norm.set_name("simplex_min_search_norm");
    for(i=0;i<_chifn.get_dim();i++){
        norm.set(i,_chifn.get_characteristic_length(i));
    }

    previous_mindex=-1;
    if(local_associates.get_dim()<2){
        for(i=_last_did_min;i<_chifn.get_pts();i++){
            if(i!=_chifn.mindex()){
                if(previous_mindex<0 || _chifn.get_fn(i)<mu_previous_min){
                    mu_previous_min=_chifn.get_fn(i);
                    previous_mindex=i;
                }
            }
        }
    }
    else{
        for(i=0;i<local_associates.get_dim();i++){
            if(local_associates.get_data(i)!=_chifn.mindex()){
                mu=0.0;
                for(j=0;j<_chifn.get_dim();j++){
                    mu+=power((_chifn.get_pt(_chifn.mindex(),j)-_chifn.get_pt(local_associates.get_data(i),j))/norm.get_data(j),2);
                }
                if(previous_mindex<0 || mu>mu_previous_min){
                    previous_mindex=local_associates.get_data(i);
                    mu_previous_min=mu;
                }
            }
        }
    }

    if(previous_mindex<0){
        printf("WARNING previous mindex %d\n",previous_mindex);
        printf("n_local_associates %d\n",local_associates.get_dim());
        exit(1);
    }

    array_1d<int> empty;

    dchi_multimodal_simplex dchifn(&_chifn, empty);

    array_1d<int> seed_dex;
    seed_dex.set_name("carom_simplex_min_search_seed_dex");
    array_2d<double> seed;
    seed.set_name("carom_simplex_min_search_seed");
    seed.set_cols(_chifn.get_dim());

    array_2d<double> old_dir;
    old_dir.set_name("carom_simplex_min_searc_old_dir");
    old_dir.set_cols(_chifn.get_dim());

    int iFound,i_min,i_row;
    double mu_min;
    double target,tol,delta;

    array_1d<double> dir,arm,trial,trial_dir;
    dir.set_name("maps_min_dir");
    arm.set_name("maps_min_arm");
    trial.set_name("maps_min_trial");
    trial_dir.set_name("maps_min_trial_dir");

    for(i=0;i<_chifn.get_dim();i++){
        dir.set(i,(_chifn.get_pt(_chifn.mindex(),i)-_chifn.get_pt(previous_mindex,i))/norm.get_data(i));
    }

    for(i=0;i<_chifn.get_dim();i++){
       trial.set(i,_chifn.get_pt(_chifn.mindex(),i)+1.5*dir.get_data(i)*norm.get_data(i));
    }
    seed.add_row(trial);
    seed.add_row(_chifn.get_pt(previous_mindex)[0]);

    double rr;
    rr=dir.normalize();
    old_dir.add_row(dir);
    double component,length;
    while(seed.get_rows()<_chifn.get_dim()+1){
        for(i=0;i<_chifn.get_dim();i++){
            trial_dir.set(i,normal_deviate(_chifn.get_dice(),0.0,1.0));
        }
        trial_dir.normalize();
        for(i=0;i<old_dir.get_rows();i++){
            component=0.0;
            for(j=0;j<_chifn.get_dim();j++){
                component+=trial_dir.get_data(j)*old_dir.get_data(i,j);
            }
            for(j=0;j<_chifn.get_dim();j++){
                trial_dir.subtract_val(j,component*old_dir.get_data(i,j));
            }
        }
        length=trial_dir.normalize();
        if(length>1.0e-10){
            for(i=0;i<_chifn.get_dim();i++){
                trial.set(i,_chifn.get_pt(_chifn.mindex(),i)+norm.get_data(i)*rr*(dir.get_data(i)+0.1*trial_dir.get_data(i)));
            }
            seed.add_row(trial);
            old_dir.add_row(trial_dir);
        }

    }

    printf("got seed\n");

    simplex_minimizer ffmin;
    ffmin.set_chisquared(&dchifn);
    ffmin.set_dice(_chifn.get_dice());
    array_1d<double> min,max;
    min.set_name("carom_simplex_search_min");
    max.set_name("carom_simplex_search_min");
    _chifn.get_min(min);
    _chifn.get_max(max);
    ffmin.set_minmax(min,max);
    ffmin.use_gradient();

    array_1d<double> minpt;
    ffmin.find_minimum(seed,minpt);

    assess_good_points(pt_start);

    _last_did_min=_chifn.get_pts();

    _ct_simplex_min+=_chifn.get_pts()-pt_start;
    printf("    min %e\n",_chifn.chimin());

}


void maps::simplex_boundary_search(){
    printf("\ndoing maps.simplex_boundary_search() -- %e %d\n",_chifn.chimin(),_duds.get_dim());
    _calls_to_simplex++;
    int pt_start=_chifn.get_pts();

    array_1d<int> local_associates;
    local_associates.set_name("carom_simplex_local_associates");

    int i_node,i_pt;
    int i,j;
    double xmin,xmax,xx;

    assess_good_points();

    dchi_boundary_simplex_gp dchifn(&_chifn,&_interpolator,_good_points);

    simplex_minimizer ffmin;
    ffmin.set_chisquared(&dchifn);
    ffmin.set_dice(_chifn.get_dice());
    array_1d<double> min,max;
    min.set_name("carom_simplex_search_min");
    max.set_name("carom_simplex_search_min");
    _chifn.get_min(min);
    _chifn.get_max(max);
    ffmin.set_minmax(min,max);
    ffmin.use_gradient();

    array_2d<double> seed;
    seed.set_name("carom_simplex_search_seed");

    seed.set_cols(_chifn.get_dim());
    int iFound;
    array_1d<double> trial;
    array_1d<int> seed_dex;
    double ftrial;
    trial.set_name("carom_simplex_search_trial");
    seed_dex.set_name("carom_simplex_search_seed_dex");
    int i_min=-1;
    double mu_min;

    while(seed.get_rows()<_chifn.get_dim()){
        for(i=0;i<_chifn.get_dim();i++){
            trial.set(i,min.get_data(i)+_chifn.random_double()*(max.get_data(i)-min.get_data(i)));
        }
        ftrial=evaluate(trial,&iFound);

        if(ftrial<exception_value){
            if(seed_dex.contains(iFound)==0){
                seed_dex.add(iFound);
                seed.add_row(trial);
                if(i_min<0 || ftrial<mu_min){
                    i_min=iFound;
                    mu_min=ftrial;
                }
            }
        }
    }

    if(seed.get_rows()<_chifn.get_dim()+1){
        for(i=0;i<_chifn.get_dim();i++){
            trial.set(i,0.5*(_chifn.get_pt(i_min,i)+_chifn.get_pt(_chifn.mindex(),i)));
        }
        seed.add_row(trial);
    }

    double mu,start_min;
    for(i=0;i<seed.get_rows();i++){
        mu=dchifn(seed(i)[0]);
        if(i==0 || mu<start_min){
            start_min=mu;
        }
    }
    printf("    starting from %e\n",start_min);

    array_1d<double> minpt;
    minpt.set_name("carom_simplex_search_minpt");

    ffmin.find_minimum(seed,minpt);

    mu=evaluate(minpt, &i_min);

    assess_good_points(pt_start);

    _log.add(_log_simplex,i_min);

    double tol=0.01*(_chifn.target()-_chifn.chimin());
    if(_chifn.get_fn(i_min)-_chifn.target()>tol){
        bisection(_chifn.mindex(),i_min,_chifn.target(),tol);
    }

    printf("    actually found %e -- %e %e\n",
    _chifn.get_fn(i_min),_chifn.get_pt(i_min,0), _chifn.get_pt(i_min,1));

    printf("    adjusted %e\n",dchifn(_chifn.get_pt(i_min)[0]));

    printf("    min is %e target %e\n",_chifn.chimin(),_chifn.target());

    _ct_simplex+=_chifn.get_pts()-pt_start;
}

void maps::mcmc_search(){
    printf("\ndoing maps.mcmc_search %d %e %d %d\n",
    _ct_mcmc,_chifn.chimin(),_chifn.get_called(),_chifn.get_pts());
    int pt_start=_chifn.get_pts();

    int i;
    double delta=_chifn.target()-_chifn.chimin();

    if(_init_mcmc==0){
        _mcmc.initialize(4,99,&_chifn);
        _mcmc.set_name_root(_outname);
        for(i=0;i<_chifn.get_dim();i++){
            _mcmc.set_min(i,_chifn.get_min(i));
            _mcmc.set_max(i,_chifn.get_max(i));
        }
        _mcmc.set_burnin(1000);
        _mcmc.guess_bases(_chifn.get_pt(_chifn.mindex())[0], delta, 1);
        _init_mcmc=1;
        _mcmc_basis_min=_chifn.chimin();
    }

    if(_mcmc_basis_min-_chifn.chimin()>0.1*delta){
        _mcmc.guess_bases(_chifn.get_pt(_chifn.mindex())[0], delta, 1);
        _mcmc_basis_min=_chifn.chimin();
    }

    _mcmc.sample(100*_chifn.get_dim());

    assess_good_points(pt_start);

    _ct_mcmc+=_chifn.get_pts()-pt_start;
    printf("min %e target %e\n",_chifn.chimin(),_chifn.target());
}

void maps::search(int limit){
    while(_chifn.get_pts()<limit){
        //if(_ct_simplex+_ct_simplex_min<=_ct_mcmc){
            printf("\nchoosing simplex %d %d %d -- %d\n",
            _ct_simplex,_ct_simplex_min,_ct_simplex+_ct_simplex_min,_ct_mcmc);

            if(_ct_simplex<_ct_simplex_min){
                simplex_boundary_search();
            }
            else{
                simplex_min_search();
            }
        //}
        //else{
        //    mcmc_search();
        //}
        if(_chifn.get_pts()-_last_written>_write_every){
            write_pts();
        }
    }
}


int maps::bisection(int ilow, int ihigh, double target, double tol){
    return bisection(_chifn.get_pt(ilow)[0], _chifn.get_pt(ihigh)[0], target, tol);
}


int maps::bisection(int iOrigin, array_1d<double> &dir, double target, double tol){

    array_1d<double> highball;
    highball.set_name("maps_bisection_dir_highball");
    double norm=1.0;
    double fhigh=-2.0*exception_value;
    int i;
    for(i=0;i<_chifn.get_dim();i++){
        highball.set(i,_chifn.get_pt(iOrigin,i));
    }

    while(fhigh<target+tol){
        for(i=0;i<_chifn.get_dim();i++){
            highball.add_val(i,norm*dir.get_data(i));
        }
        fhigh=evaluate(highball,&i);
        norm*=2.0;
    }

    return bisection(_chifn.get_pt(iOrigin)[0], highball, target, tol);
}

int maps::bisection(array_1d<double> &low_in, array_1d<double> &high_in,
                    double target, double tol){


    int i;
    array_1d<double> trial,lowball,highball;
    trial.set_name("maps_bisection_trial");
    lowball.set_name("maps_bisection_trial");
    highball.set_name("maps_bisection_trial");

    for(i=0;i<_chifn.get_dim();i++){
        lowball.set(i,low_in.get_data(i));
        highball.set(i,high_in.get_data(i));
    }

    int iFound,iTrial;
    double mu,dmu,muFound,dmuFound;

    mu=evaluate(highball,&iFound);
    muFound=evaluate(lowball, &iFound);

    dmuFound=fabs(muFound-target);

    int ct;

    for(ct=0;(ct<5 || dmuFound>tol) && ct<40;ct++){
        for(i=0;i<_chifn.get_dim();i++){
            trial.set(i,0.5*(lowball.get_data(i)+highball.get_data(i)));
        }
        mu=evaluate(trial,&iTrial);
        if(mu<target){
            for(i=0;i<_chifn.get_dim();i++){
                lowball.set(i,trial.get_data(i));
            }
        }
        else{
            for(i=0;i<_chifn.get_dim();i++){
                highball.set(i,trial.get_data(i));
            }
        }

        dmu=fabs(mu-target);
        if(dmu<dmuFound){
            iFound=iTrial;
            muFound=mu;
            dmuFound=dmu;
        }
    }

    return iFound;

}

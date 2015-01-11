#include "node.h"

node::node(){
    initialize();
}

node::~node(){}

node::node(const node &in){
    initialize();
    copy(in);
}

node& node::operator=(const node&in){
    if(this==&in) return *this;
    initialize();
    copy(in);
    return *this;
}

void node::initialize(){
    _chisquared=NULL;
    _chimin=2.0*exception_value;
    _centerdex=-1;
    _bisection_tolerance=0.01;
    _compass_points.set_name("node_compass_points");
    _basis_associates.set_name("node_basis_associates");
    _basis_mm.set_name("node_basis_mm");
    _basis_bb.set_name("node_basis_bb");
    _basis_model.set_name("node_basis_model");
    _basis_vectors.set_name("node_basis_vectors");
    _basis_ddsq.set_name("node_basis_ddsq");
    _basis_vv.set_name("node_basis_vv");
}

void node::copy(const node &in){
    _centerdex=in._centerdex;
    _chimin=in._chimin;
    
    int i,j;
    
    _chisquared=in._chisquared;
    
    _compass_points.reset();
    for(i=0;i<in._compass_points.get_dim();i++){
        _compass_points.set(i,in._compass_points.get_data(i));
    }
    
    _basis_associates.reset();
    for(i=0;i<in._basis_associates.get_dim();i++){
        _basis_associates.set(i,in._basis_associates.get_data(i));
    }
    
    _basis_mm.reset();
    for(i=0;i<in._basis_mm.get_dim();i++){
        _basis_mm.set(i,in._basis_mm.get_data(i));
    }
    
    _basis_bb.reset();
    for(i=0;i<in._basis_bb.get_dim();i++){
        _basis_bb.set(i,in._basis_bb.get_data(i));
    }
    
    _basis_model.reset();
    for(i=0;i<in._basis_model.get_dim();i++){
        _basis_model.set(i,in._basis_model.get_data(i));
    }
    
    _basis_vectors.reset();
    _basis_vectors.set_cols(in._basis_vectors.get_cols());
    for(i=0;i<in._basis_vectors.get_rows();i++){
        for(j=0;j<in._basis_vectors.get_cols();j++){
            _basis_vectors.get_data(i,j,in._basis_vectors.get_data(i,j));
        }
    }
}

void node::set_center(int ix){
    _centerdex=ix;
    if(_chisquared!=NULL){
        _chimin=_chisquared->get_fn(ix);
    }
}

void node::set_chisquared(chisquared *cc){
    _chisquared=cc;
    if(_centerdex>=0){
        _chimin=_chisquared->get_fn(_centerdex);
    }
    
    int i,j;
    _basis_vectors.reset();
    _basis_vectors.set_cols(_chisquared->get_dim());
    for(i=0;i<_chisquared->get_dim();i++){
        for(j=0;j<_chisquared->get_dim();j++){
            if(i==j){
                _basis_vectors.set(i,j,1.0);
            }
            else{
                _basis_vectors.set(i,j,0.0);
            }
        }
    }
}

int node::get_center(){
    return _centerdex;
}

void node::set_basis(int ii, int jj, double vv){
    if(_chisquared==NULL){
        printf("WARNING cannot set node basis before assigning _chisquared\n");
        exit(1);
    }
    _basis_vectors.set(ii,jj,vv);
}

void node::is_it_safe(char *word){
    if(_chisquared==NULL){
        printf("WARNING in node::%s\n",word);
        printf("_chisquared is null\n");
        exit(1);
    }
    
    if(_basis_vectors.get_cols()!=_chisquared.get_dim() || 
       _basis_vectors.get_rows()!=_chisquared.get_dim()){
    
        printf("WARNING in node::%s\n",word);
        
        printf("_basis_vectors %d by %d\n",_basis_vectors.get_rows(),
        _basis_vectors.get_cols());
        
        printf("but dim should be %d\n",_chisquared.get_dim());
        
        exit(1);
    
    }
}

void node::evaluate(array_1d<double> &pt, double *value, int *dex){
    is_it_safe();
    
    _chisquared->evaluate(pt,value,dex);
    
    if(dex>=0){
        if(value<_chimin){
            _chimin=value;
            _centerdex=dex;
        }
    }
}

void node::bisection(array_1d<double> &lowball, double flow, array_1d<double> &highball, double fhigh){

    is_it_safe();
    
    if(flow>fhigh){
        printf("WARNING in node bisection flow %e fhigh %e\n",flow,fhigh);
        exit(1);
    }
    
    double ftrial,threshold;
    array_1d<double> trial;
    trial.set_name("node_bisection_trial");
    
    if(_chisquared->get_deltachi()<0.0){
        threshold=_bisection_tolerance;
    }
    else{
        threshold=_bisection_tolerance*_chisquared->get_deltachi();
    }
    
    int took_a_step=0,ct,i,iout;
    
    iout=-1;
    while(ct<100 && (took_a_step==0 || _chisquared->target()-flow>threshold)){
        for(i=0;i<_chisquared.get_dim();i++){
            trial.set(i,0.5*(lowball.get_data(i)+highball.get_data(i)));
        }
        
        evaluate(trial,&ftrial,&i);
        
        if(ftrial<_chisquared->target()){
            flow=ftrial;
            if(iout>=0)iout=i;
            for(i=0;i<_chisquared->get_dim();i++){
                lowball.set(i,trial.get_data(i));
            }
            took_a_step=1;
        }
        else{
            fhigh=ftrial;
            for(i=0;i<_chisquared->get_dim();i++){
                highball.set(i,trial.get_data(i));
            }
        }
        
        
        ct++;
    }
    
    return iout;

}

void node::perturb_bases(int idim, array_1d<double> &dx, array_2d<double> bases_out){
    
    is_it_safe();
    
    int i,j;
    for(i=0;i<_chisquared.get_dim();i++){
        for(j=0;j<_chisquared.get_dim();j++){
            bases_out.set(i,j,_basis_vectors.get_data(i,j));
        }
    }
    
    for(i=0;i<_chisquared.get_dim();i++){
        bases_out.add_val(idim,i,dx.get_data(i));
    }
    bases_out(idim).normalize();
    
    int ix,jx;
    double mu;
    
    for(ix=idim+1;ix!=idim;){
        if(ix>=_chisquared.get_dim()){
            ix=0;
        }
        
        for(jx=idim;jx!=ix;){
            if(ix>=_chisquared.get_dim()){
                jx=0;
            }
            
            mu=0.0;
            for(i=0;i<_chisquared.get_dim();i++){
                mu+=bases_out.get_data(ix,i)*bases_out.get_data(jx,i);
            }
            for(i=0;i<_chisquared.get_dim();i++){
                bases_out.subtract_val(ix,i,mu*bases_out.get_data(jx,i));
            }
            
            if(jx<_chisquared->get_dim()-1)jx++;
            else jx=0;
        }
        
        bases_out(ix).normalize();
        
        if(ix<_chisquared->get_dim()-1)ix++;
        else ix=0;
        
    }
    
    /////////////////testing
    for(ix=0;ix<_chisquared.get_dim();ix++){
        mu=0.0;
        for(i=0;i<_chisquared.get_dim();i++){
            mu+=bases_out.get_data(ix,i)*bases_out.get_data(ix,i);
        }
        if(fabs(mu-1.0)>1.0e-6){
            printf("WARNING in node perturb bases, square norm %e\n",mu);
            exit(1);
        }
        
        for(jx=ix+1;jx<_chisquared.get_dim();jx++){
            mu=0.0;
            for(i=0;i<_chisquared.get_dim();i++){
                mu+=bases_out.get_data(ix,i)*bases_out.get_data(jx,i);
            }
            
            if(fabs(mu)>1.0e-6){
                printf("WARNING in node perturb bases, dot product %e\n",mu);
                exit(1);
            }
        }
    }

}

double node::basis_error(array_2d<double> &trial_bases, array_1d<double> &trial_model){

    if(_basis_associates.get_dim()<=0){
        printf("WARNING cannot calculate basis error there are only %d associates\n",
        _basis_associates.get_dim());
        
        exit(1);
    }
    
    is_it_safe();
    
    trial_model.zero();
    if(_basis_ddsq.get_rows()>_basis_associates.get_dim()){
        _basis_ddsq.reset();
    }
    
    if(_ddsq.get_cols()!=_chisquared->get_dim()){
        _ddsq.set_cols(_chisquared->get_dim());
    }
    
    if(_basis_mm.get_dim()!=_chisquared->get_dim()*_chisquared->get_dim()){
        _basis_mm.set_dim(_chisquared->get_dim()&_chisquared->get_dim());
    }
    
    if(_basis_bb.get_dim()!=_chisquared->get_dim()){
        _basis_bb.set_dim(_chisquared->get_dim());
    }
    
    if(_basis_vv.get_dim()!=_chisquared->get_dim()){
        _basis_vv.set_dim(_chisquared->get_dim());
    }
    
    _basis_mm.zero();
    _basis_bb.zero();
    _basis_vv.zero();
    _basis_ddsq.zero();
    
    int i,j,ix;
    double mu;
    for(ix=0;ix<_basis_associates.get_dim();ix++){
        for(i=0;i<_chisquared.get_dim();i++){
            mu=0.0;
            for(j=0;j<_chisquared.get_dim();j++){
                mu+=_chisquared->get_dim(_basis_associates.get_data(ix),j)*trial_bases.get_data(i,j);
            }
            _basis_ddsq.set(ix,i,mu*mu);
        }
    }
    
    for(i=0;i<_chisquared->get_dim();i++){
        for(j=0;j<_basis_associates.get_dim();j++){
            _basis_bb.add_val(i,_ddsq.get_data(j,i)*(_chisquared->get_fn(_basis_associates.get_data(j))-_chimin));
        }
    }
    
    int k;
    for(i=0;i<_chisquared->get_dim();i++){
        for(j=i;j<_chisquared->get_dim();j++){
            ix=i*_chisquared.get_dim()+j;
            for(k=0;k<_basis_associates.get_dim();k++){
                _basis_mm.add_val(ix,_basis_ddsq.get_data(k,i)*_basis_ddsq.get_data(k,j));
            }
            if(j!=i){
                _basis_mm.set(j*_chisquared->get_dim()+i,_basis_mm.get_data(ix));
            }
        }
    }
    
    try{
        naive_gaussian_solver(_basis_mm,_basis_bb,trial_model,_chisquared->get_dim());
    }
    catch(int iex){
        printf("WARNING basis_error was no good\n");
        return 2.0*exception_value;
    }
    
    double error=0.0,chi_model;
    for(i=0;i<_basis_associates.get_dim();i++){
        chi_model=_chimin;
        for(j=0;j<_chisquared->get_dim();j++){
            chi_model+=trial_model.get_data(j)*_basis_ddsq.get_data(i,j);
        }
        error+=power(_chisquared->get_fn(_basis_associates.get_data(i))-chi_model,2);
    }
    
    return error;
    
}

void node::compass_search(){
    
    is_it_safe();
    
    int ix,i,j,iFound;
    double sgn,flow,fhigh,dx,ftrial,step;
    array_1d<double> lowball,highball,trial;
    
    lowball.set_name("node_compass_search_lowball");
    highball.set_name("node_compass_search_highball");
    
    dx=0.0;
    for(ix=0;ix<_chisquared->get_dim();ix++){
        for(sgn=-1.0;sgn<1.5;sgn+=2.0){
            flow=2.0*exception_value;
            fhigh=-2.0*exception_value;
            
            if(sgn<0.0){
                flow=_chimin;
                for(i=0;i<_chisquared->get_dim();i++){
                    lowball.set(i,_chisquared->get_pt(_centerdex,i));
                }
            }
            else{
                for(i=0;i<_chisquared->get_dim();i++){
                    trial.set(i,_chisquared->get_pt(_centerdex,i)+sgn*dx*_basis_vectors.get_data(ix,i));
                }
                evaluate(trial,&ftrial,&iFound);
                
                if(ftrial<_chisquared->target()){
                    flow=ftrial;
                    for(i=0;i<_chisquared->get_dim();i++){
                        lowball.set(i,trial.get_data(i));
                    }
                }
                else{
                    fhigh=ftrial;
                    for(i=0;i<_chisquared->get_dim();i++){
                        highball.set(i,trial.get_data(i));
                    }
                }
            }
            
            if(flow>_chisquared->target()){
                flow=_chimin;
                for(i=0;i<_chisquared->get_dim();i++){
                    lowball.set(i,_chisquared->get_pt(_centerdex,i));
                }
            }
            
            step=1.0;
            while(fhigh<=_chisquared->target()){
                for(i=0;i<_chisquared->get_dim();i++){
                    highball.set(i,lowball.get_data(i)+sgn*step*_basis_vectors.get_data(ix,i));
                }
                evaluate(highball,&fhigh,&iFound);
                step*=2.0;
            }
            
            iFound=bisection(lowball,flow,highball,fhigh);
            
            dx=0.0;
            for(i=0;i<_chisquared->get_dim();i++){
                dx+=_basis_vectors.get_data(ix,i)*(_chisquared->get_pt(center_dex,i)+_chisquared->get_pt(iFound,i));
            }
            
            for(i=0;i<_chisquared->get_dim();i++){
                trial.set(i,0.5*(_chisquared->get_pt(center_dex,i)+_chisquared->get_pt(iFound,i)));
            }
            evaluate(trial,&ftrial,&iFound);
            if(iFound>=0){
                _basis_associates.add(iFound);
            }
        }
    }
}

///////////////arrayOfNodes code below//////////

arrayOfNodes::arrayOfNodes(){
    data=NULL;
    ct=0;
    room=0;
}

arrayOfNodes::~arrayOfNodes(){
    if(data!=NULL){
        delete [] data;
    }
}

void arrayOfNodes::add(int cc, chisq_wrapper *gg){

    node *buffer;
    int i,j;
    
    if(_ct==_room){
        if(_ct>0){
            buffer=new node[ct];
            for(i=0;i<ct;i++){
                buffer[i].copy(data[i]);
            }
            
            delete [] data;
        }
        
        _room+=5;
        data=new node[_room];
        
        if(_ct>0){
            for(i=0;i<_ct;i++){
                data[i].copy(buffer[i]);
            }
            delete [] buffer;
        }
    }
    
    data[ct].set_chisquared(gg);
    data[ct].set_center(cc);

    _ct++;

}

void arrayOfNodes::add(gpWrapper *g, int i){
    add(i,g);
}

int arrayOfNodes::get_dim(){
    return _ct;
}

void arrayOfNodes::remove(int ii){
    int i;
    if(ii>=_ct) return;
    
    for(i=ii+1;i<_ct;i++){
        data[i-1].copy(data[i]);
    }
    _ct--;
}

node* arrayOfNodes::operator()(int dex){
    if(dex<0 || dex>=_ct){
        printf("WARNING asked arrayOfNodes for dex %d but only have %d (operator)\n",
        dex,_ct);
        
        exit(1);
    }
    return &data[dex];
}

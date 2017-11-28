#include "hyperbox.h"
#include "exampleLikelihoods.h"

void pixellate(const array_1d<double> &pt,
               const array_1d<double> &dx,
               array_1d<int> &px){

    int i,j;
    for(i=0;i<pt.get_dim();i++){
        j=int(pt.get_data(i)/dx.get_data(i));
        if(pt.get_data(i)-j*dx.get_data(i)>0.5*dx.get_data(i))j++;
        if(j*dx.get_data(i)-pt.get_data(i)>0.5*dx.get_data(i))j--;

        if(fabs(j*dx.get_data(i)-pt.get_data(i))>0.5*dx.get_data(i)){
            printf("pixellate failed\n");
            printf("input %e fit %e\n",pt.get_data(i),j*dx.get_data(i));
            printf("dx %e delta %e\n",dx.get_data(i), fabs(j*dx.get_data(i)-pt.get_data(i)));
            exit(1);
        }
        px.set(i,j);
    }

}

class hyperbox_integrator{

    public:

        hyperbox_list hb_list;

        hyperbox_integrator(){
            _dalex_pts.set_name("integrator_dalex_pts");
            _dalex_chisq.set_name("integrator_dalex_chisq");
            _dx.set_name("integrator_dx");
            _pixel_list.set_name("integrator_pixel_list");
            _pixel_mapping.set_name("integrator_pixel_mapping");
            _local_pixel.set_name("integrator_local_pixel");
        }

        ~hyperbox_integrator(){}

        int get_n_pts(){
            int sum=0;
            int i;
            for(i=0;i<_pixel_mapping.get_rows();i++){
                sum+=_pixel_mapping.get_cols(i);
            }
            if(sum!=_dalex_pts.get_rows()){
                printf("WARNING _dalex_pts %d; _pixel_mapping %d\n",
                _dalex_pts.get_rows(),sum);
                exit(1);
            }
            return _dalex_pts.get_rows();
        }

        double chisq_min(){
            return _chisq_min;
        }

        void _add_pixel(int pt_dex){
            pixellate(_dalex_pts(pt_dex),_dx,_local_pixel);
            int i,j;
            int pixel_dex=-1;
            for(i=0;i<_pixel_list.get_rows();i++){
                pixel_dex=i;
                for(j=0;j<_dalex_pts.get_cols();j++){
                    if(_pixel_list.get_data(i,j)!=_local_pixel.get_data(j)){
                        pixel_dex=-1;
                        break;
                    }
                }
                if(pixel_dex>=0){
                    break;
                }
            }

            int already_contained=0;
            if(pixel_dex<0){
                _pixel_list.add_row(_local_pixel);
                _pixel_mapping.set(_pixel_list.get_rows()-1,0,pt_dex);
                pixel_dex=_pixel_list.get_rows()-1;
            }
            else{
                already_contained=0;
                for(i=0;i<_pixel_mapping.get_cols(i);i++){
                    if(_pixel_mapping.get_data(pixel_dex,i)==pt_dex){
                        already_contained=1;
                        break;
                    }
                }
                if(already_contained==0){
                    _pixel_mapping.add(pixel_dex,pt_dex);
                }
            }

            double xmin,xmax;
            for(i=0;i<_dalex_pts.get_cols();i++){
                xmin=_pixel_list.get_data(pixel_dex,i)*_dx.get_data(i)-0.5*_dx.get_data(i);
                xmax=_pixel_list.get_data(pixel_dex,i)*_dx.get_data(i)+0.5*_dx.get_data(i);
                if(_dalex_pts.get_data(pt_dex,i)<xmin){
                    printf("pixelization failed on min: %e !> %e\n",
                    _dalex_pts.get_data(pt_dex,i),xmin);
                    exit(1);
                }
                if(_dalex_pts.get_data(pt_dex,i)>xmax){
                    printf("pixelization failed on max: %e !< %e\n",
                    _dalex_pts.get_data(pt_dex,i),xmax);
                    exit(1);
                }
            }

        }

        void set_pts(array_2d<double> &pts, array_1d<double> &xx, double delta_chisq){
             _dalex_pts.reset_preserving_room();
             _dalex_chisq.reset_preserving_room();
             _dx.reset_preserving_room();
             _pixel_list.reset_preserving_room();
             _pixel_mapping.reset_preserving_room();
             int i;
             _chisq_min=2.0*exception_value;
             for(i=0;i<pts.get_rows();i++){
                 _dalex_pts.add_row(pts(i));
                 _dalex_chisq.add(xx.get_data(i));
                 if(i==0 || xx.get_data(i)<_chisq_min){
                     _chisq_min=xx.get_data(i);
                 }
             }

             int j;
             array_1d<double> good_xmin, good_xmax;
             good_xmin.set_name("integrator_set_pts_good_xmin");
             good_xmax.set_name("integrator_set_pts_good_xmax");
             for(i=0;i<_dalex_pts.get_rows();i++){
                 if(_dalex_chisq.get_data(i)<=_chisq_min+delta_chisq){
                     for(j=0;j<_dalex_pts.get_cols();j++){
                         if(j>=good_xmin.get_dim() || _dalex_pts.get_data(i,j)<good_xmin.get_data(j)){
                             good_xmin.set(j,_dalex_pts.get_data(i,j));
                         }
                         if(j>=good_xmax.get_dim() || _dalex_pts.get_data(i,j)>good_xmax.get_data(j)){
                             good_xmax.set(j,_dalex_pts.get_data(i,j));
                         }
                     }
                 }
             }

             for(i=0;i<_dalex_pts.get_cols();i++){
                 _dx.set(i,0.1*(good_xmax.get_data(i)-good_xmin.get_data(i)));
             }

             for(i=0;i<_dalex_pts.get_rows();i++){
                 _add_pixel(i);
             }
             get_n_pts();
             printf("done with initialization\n");
        }

        int add_pt(array_1d<double> &pt, double xx){
            int box_dex=-1;
            int i,j,valid;
            for(i=0;i<hb_list.ct();i++){
                valid=1;
                for(j=0;j<hb_list(i)->dim();j++){
                    if(pt.get_data(j)<hb_list(i)->min(j)){
                        valid=0;
                        break;
                    }
                    if(pt.get_data(j)>hb_list(i)->max(j)){
                        valid=0;
                        break;
                    }
                }
                if(valid==1){
                    box_dex=i;
                    break;
                }
            }
            if(box_dex<0){
                return 0;
            }
            add_pt(pt,xx,box_dex);
            //printf("found a box %d\n",box_dex);
            //exit(1);
            return 1;
        }

        void add_pt(array_1d<double> &pt, double xx, int box_dex){
            _dalex_pts.add_row(pt);
            _dalex_chisq.add(xx);
            if(_dalex_pts.get_rows()!=_dalex_chisq.get_dim()){
                printf("WARNING array points %d chisq values %d\n",
                _dalex_pts.get_rows(),_dalex_chisq.get_dim());
                exit(1);
            }
            _add_pixel(_dalex_pts.get_rows()-1);
            array_1d<double> dummy_pt;
            dummy_pt.set_name("dummy_pt");
            int i;
            for(i=0;i<pt.get_dim();i++){
                dummy_pt.set(i,pt.get_data(i));
            }
            dummy_pt.set(pt.get_dim(),xx-_chisq_min);
            hb_list(box_dex)->add_point(dummy_pt);
        }

        void create_hyperboxes(){
            hb_list.reset();
            hb_list.set_room(_dalex_pts.get_rows());
            int n_box_pts=0;
            int pt_dex;
            double t_pre_init=double(time(NULL));
            array_1d<double> pt;
            pt.set_name("integrator_create_hyperboxes_pt");
            array_2d<double> box_pts;
            box_pts.set_name("integrator_create_hyperboxes_box_pts");
            array_1d<double> box_min,box_max;
            box_min.set_name("integrator_create_hyperboxes_box_min");
            box_max.set_name("integrator_create_hyperboxes_box_max");
            pt.reset_preserving_room();
            hyperbox hb;
            int i,j,k;
            int dim;
            dim=_dalex_pts.get_cols();
            for(i=0;i<_pixel_list.get_rows();i++){
                box_pts.reset_preserving_room();
                for(j=0;j<dim;j++){
                    box_min.set(j,_pixel_list.get_data(i,j)*_dx.get_data(j)-0.5*_dx.get_data(j));
                    box_max.set(j,_pixel_list.get_data(i,j)*_dx.get_data(j)+0.5*_dx.get_data(j));
                    if(box_min.get_data(j)>box_max.get_data(j)){
                       printf("setting min/max backwards\n");
                       printf("%e  %e\n",box_min.get_data(j),box_max.get_data(j));
                       printf("%d %e\n",_pixel_list.get_data(i,j),_dx.get_data(j));
                       exit(1);
                    }
                }
                for(j=0;j<_pixel_mapping.get_cols(i);j++){
                    pt_dex = _pixel_mapping.get_data(i,j);
                    for(k=0;k<dim;k++){
                        pt.set(k,_dalex_pts.get_data(pt_dex,k));
                        if(pt.get_data(k)>box_max.get_data(k) || pt.get_data(k)<box_min.get_data(k)){
                            printf("assigning point that is outside of box\n");
                            printf("failed: %e < %e < %e\n",
                            box_min.get_data(k),pt.get_data(k),box_max.get_data(k));
                            exit(1);
                        }
                    }
                    pt.set(dim,_dalex_chisq.get_data(pt_dex)-_chisq_min);
                    box_pts.add_row(pt);
                }
                if(box_pts.get_rows()==0){
                    printf("trying to initialize empty box\n");
                    exit(1);
                }
                for(j=0;j<box_pts.get_rows();j++){
                    for(k=0;k<dim;k++){
                        if(box_pts.get_data(j,k)<box_min.get_data(k) ||
                           box_pts.get_data(j,k)>box_max.get_data(k)){

                            printf("hyper box assignment failed\n");
                            printf("%e %e %e\n",box_min.get_data(k),
                            box_pts.get_data(j,k),box_max.get_data(k));
                            exit(1);
                        }
                    }
                }
                n_box_pts+=box_pts.get_rows();
                hb.build(box_pts,box_min,box_max);
                hb_list.add(hb);
            }

            printf("hyperboxes %d; points %d; array_pts %d\n",hb_list.ct(),n_box_pts,get_n_pts());

            for(i=0;i<hb_list.ct();i++){
                if(hb_list(i)->dim()!=dim){
                    printf("WARNING initial box has dim %d\n",
                    hb_list(i)->dim());
                    exit(1);
                 }
            }
        }

        void split_hyperboxes(){
            int i,j;
            hyperbox hb;
            int n_pts=0;
            for(i=0;i<hb_list.ct();i++){
                n_pts+=hb_list(i)->n_pts();
            }
            int room0=hb_list.room();
            if(n_pts>hb_list.room()){
                hb_list.set_room(n_pts+100000);
            }
            int all_clear=0;
            array_1d<double> min1,min2,max1,max2;
            array_2d<double> pts1,pts2;
            min1.set_name("min1");
            min2.set_name("min2");
            max1.set_name("max1");
            max2.set_name("max2");
            pts1.set_name("pts1");
            pts2.set_name("pts2");
            double t_pre_split=double(time(NULL));
            while(all_clear==0){
                all_clear=1;
                for(i=0;i<hb_list.ct();i++){
                    if(hb_list(i)->n_pts()>1){
                        all_clear=0;
                        hb_list(i)->split(pts1,min1,max1,pts2,min2,max2);
                        hb_list(i)->build(pts1,min1,max1);
                        hb.build(pts2,min2,max2);
                        hb_list.add(hb);
                    }
                }
            }

            for(i=0;i<hb_list.ct();i++){
                if(hb_list(i)->n_pts()!=1){
                    printf("have box with %d pts\n",hb_list(i)->n_pts());
                    exit(1);
                }
            }

            n_pts=0;
            for(i=0;i<hb_list.ct();i++){
                n_pts+=hb_list(i)->n_pts();
            }
            if(n_pts!=_dalex_pts.get_rows()){
                printf("WARNING; pt mismatch\n");
                exit(1);
            }
        }


    void sample(int n_samples, double delta_chisq, jellyBeanData *chifn, char *out_name){

        Ran dice(81234123);
        array_1d<double> ln_tot_vol;
        double tot_vol;
        array_1d<double> ln_vol;
        array_1d<int> ln_vol_dex;
        array_1d<double> ln_vol_sorted;
        int i;
        for(i=0;i<hb_list.ct();i++){
            if(_dalex_chisq.get_data(i)<_chisq_min+delta_chisq){
                ln_vol_dex.add(i);
                ln_vol.add(hb_list(i)->ln_vol());
            }
        }
        sort(ln_vol,ln_vol_sorted,ln_vol_dex);
        ln_tot_vol.set_dim(ln_vol.get_dim());
        ln_tot_vol.set(0,ln_vol_sorted.get_data(0));
        tot_vol=exp(ln_tot_vol.get_data(0));
        for(i=1;i<ln_vol.get_dim();i++){
            tot_vol += exp(ln_vol_sorted.get_data(i));
            ln_tot_vol.set(i,log(tot_vol));
            printf("ln_vol_sorted %d %e %e\n",ln_vol_dex.get_data(i),ln_vol_sorted.get_data(i),tot_vol);
        }
        for(i=0;i<ln_tot_vol.get_dim();i++){
            ln_tot_vol.subtract_val(i,ln_tot_vol.get_data(ln_tot_vol.get_dim()-1));
        }

        int samples_taken;
        int i_box,i_box_dex;
        int accept_it;
        int degen=-1;
        array_1d<double> pt;
        array_1d<double> next_pt;
        double chisq=-1.0;
        double next_chisq;
        double roll;
        FILE *output;
        output = fopen(out_name, "w");
        for(samples_taken=0;samples_taken<n_samples;samples_taken++){
            roll = log(dice.doub());
            for(i_box_dex=0;
                i_box_dex<ln_tot_vol.get_dim() && roll>ln_tot_vol.get_data(i_box_dex);
                i_box_dex++);

            i_box = ln_vol_dex.get_data(i_box_dex);
            if(_dalex_chisq.get_data(i_box)>_chisq_min+delta_chisq){
                printf("WARNING box has %e, but %e is limit\n",
                _dalex_chisq.get_data(i_box),_chisq_min+delta_chisq);
            }

            for(i=0;i<_dalex_pts.get_cols();i++){
                next_pt.set(i,hb_list(i_box)->min(i)+
                              dice.doub()*(hb_list(i_box)->max(i)-hb_list(i_box)->min(i)));
            }
            next_chisq = chifn[0](next_pt);
            accept_it=0;
            if(chisq<0.0 || next_chisq<chisq){
                accept_it=1;
            }
            else{
                roll=dice.doub();
                if(roll<exp(-0.5*(next_chisq-chisq))){
                    accept_it=1;
                }
            }
            /*i=ln_tot_vol.get_dim()-1;
            printf("i_box %d chisq %e next_chisq %e - %e %e\n",
            i_box_dex,chisq,next_chisq,ln_tot_vol.get_data(i),ln_tot_vol.get_data(i-1));*/

            if(accept_it==1){
                if(degen>0){
                    fprintf(output,"%d %e ",degen, chisq);
                    for(i=0;i<_dalex_pts.get_cols();i++){
                        fprintf(output,"%e ",pt.get_data(i));
                    }
                    fprintf(output,"\n");
                }
                degen=1;
                for(i=0;i<_dalex_pts.get_cols();i++){
                     pt.set(i,next_pt.get_data(i));
                }
                chisq=next_chisq;
            }
            else{
                degen++;
            }

        }
        fprintf(output,"%d %e ",degen, chisq);
        for(i=0;i<_dalex_pts.get_cols();i++){
            fprintf(output,"%e ",pt.get_data(i));
        }
        fprintf(output,"\n");

        fclose(output);
    }


    private:
        array_2d<double> _dalex_pts;
        array_1d<double> _dalex_chisq,_dx;
        array_2d<int> _pixel_list;
        asymm_array_2d<int> _pixel_mapping;
        double _chisq_min;
        array_1d<int> _local_pixel;

};


int main(int iargc, char *argv[]){

    int i,j,k,dim;
    char in_name[letters];
    char out_name[letters];
    double delta_chisq=-1.0;
    in_name[0]=0;
    out_name[0]=0;
    dim=-1;
    int n_samples=0;
    for(i=1;i<iargc;i++){
        if(argv[i][0]=='-'){
            switch(argv[i][1]){
                case 'i':
                    i++;
                    for(j=0;j<letters-1 && argv[i][j]!=0;j++){
                        in_name[j]=argv[i][j];
                    }
                    in_name[j]=0;
                    break;
                case 'o':
                    i++;
                    for(j=0;j<letters-1 && argv[i][j]!=0;j++){
                        out_name[j]=argv[i][j];
                    }
                    out_name[j]=0;
                    break;
                case 'd':
                    i++;
                    dim=atoi(argv[i]);
                    break;
                case 'c':
                    i++;
                    delta_chisq=atof(argv[i]);
                    break;
                case 'n':
                    i++;
                    n_samples=atoi(argv[i]);
                    break;
            }
        }
    }

    if(dim<0){
        printf("need to specify dim\n");
        exit(1);
    }
    if(in_name[0]==0){
        printf("need to specify in_name\n");
        exit(1);
    }
    if(out_name[0]==0){
        printf("need to specify out_name\n");
        exit(1);
    }
    if(delta_chisq<0.0){
        printf("need to specify delta_chisq\n");
        exit(1);
    }

    int n_cols=0;
    char word[letters];
    word[0]=0;
    FILE *in_file;
    in_file = fopen(in_name, "r");
    while(compare_char("log", word)==0){
        fscanf(in_file,"%s",word);
        if(compare_char("#",word)==0){
            n_cols++;
        }
    }

    array_1d<double> pt;
    pt.set_name("pt");
    array_1d<double> dalex_chisq,xmin,xmax;
    array_2d<double> dalex_pts;
    double xx;
    dalex_chisq.set_name("dalex_chisq");
    dalex_pts.set_name("dalex_pts");
    xmin.set_name("xmin");
    xmax.set_name("xmax");

    double chisq_min=2.0*exception_value;
    printf("n_cols %d\n",n_cols);
    while(fscanf(in_file,"%le",&xx)>0){
        pt.set(0,xx);
        if(xmin.get_dim()==0 || xx<xmin.get_data(0)){
            xmin.set(0,xx);
        }
        if(xmax.get_dim()==0 || xx>xmax.get_data(0)){
            xmax.set(0,xx);
        }
        for(i=1;i<dim;i++){
            fscanf(in_file,"%le",&xx);
            pt.set(i,xx);
            if(i>=xmin.get_dim() || xx<xmin.get_data(i)){
                xmin.set(i,xx);
            }
            if(i>=xmax.get_dim() || xx>xmax.get_data(i)){
                xmax.set(i,xx);
            }
        }
        dalex_pts.add_row(pt);
        fscanf(in_file,"%le",&xx);
        dalex_chisq.add(xx);
        if(xx<chisq_min){
            chisq_min=xx;
        }
        for(i=dim+1;i<n_cols;i++){
            fscanf(in_file,"%le",&xx);
        }
    }

    fclose(in_file);
    printf("read in data\n");

    array_1d<double> good_xmin,good_xmax;
    good_xmin.set_name("good_xmin");
    good_xmax.set_name("good_xmax");
    for(i=0;i<dalex_pts.get_rows();i++){
        if(dalex_chisq.get_data(i)<=chisq_min+21.03){
            for(j=0;j<dim;j++){
                xx=dalex_pts.get_data(i,j);
                if(j>=good_xmin.get_dim() || xx<good_xmin.get_data(j)){
                    good_xmin.set(j,xx);
                }
                if(j>=good_xmax.get_dim() || xx>good_xmax.get_data(j)){
                    good_xmax.set(j,xx);
                }
            }
        }
    }


    jellyBeanData *chifn;

    if(dim==12){
        chifn = new gaussianJellyBean12();
    }
    else if(dim==4){
        chifn = new integrableJellyBean();
    }
    else{
        printf("cannot do dim %d\n",dim);
        exit(1);
    }
    Ran dice(81242);
    array_1d<double> ran_pt;
    ran_pt.set_name("ran_pt");
    for(i=0;i<100000;i++){
        for(j=0;j<dim;j++){
            xx=good_xmax.get_data(j)-good_xmin.get_data(j);
            ran_pt.set(j,good_xmin.get_data(j)+dice.doub()*xx);
        }
        xx=chifn[0](ran_pt);
        dalex_pts.add_row(ran_pt);
        dalex_chisq.add(xx);
    }


    hyperbox_integrator hb_integrator;
    hb_integrator.set_pts(dalex_pts, dalex_chisq, delta_chisq);
    dalex_pts.reset();
    dalex_chisq.reset();

    hb_integrator.create_hyperboxes();
    hb_integrator.split_hyperboxes();
    hb_integrator.sample(n_samples, delta_chisq, chifn, out_name);

}

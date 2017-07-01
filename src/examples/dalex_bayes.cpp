#include "containers.h"
#include "goto_tools.h"
#include "kd.h"

void pixellate(const array_1d<double> &pt,
               const array_1d<double> &dx,
               const array_1d<double> &min,
               array_1d<int> &px){

    int i,j;
    for(i=0;i<pt.get_dim();i++){
        for(j=0;min.get_data(i)+j*dx.get_data(i)<pt.get_data(i);j++);
        if(min.get_data(i)+j*dx.get_data(i)-pt.get_data(i)>0.5*dx.get_data(i)){
            j--;
        }
        px.set(i,j);
    }

}

int main(int iargc, char *argv[]){

    double pixel_factor=0.1;
    int i,j,k,dim;
    char in_name[letters];
    char out_name[letters];
    double delta_chisq=-1.0;
    in_name[0]=0;
    out_name[0]=0;
    dim=-1;
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

    array_1d<double> dalex_chisq;
    double xx;
    dalex_chisq.set_name("dalex_chisq");

    printf("n_cols %d\n",n_cols);
    while(fscanf(in_file,"%le",&xx)>0){
        for(i=1;i<dim;i++){
            fscanf(in_file,"%le",&xx);
        }
        fscanf(in_file,"%le",&xx);
        dalex_chisq.add(xx);
        for(i=dim+1;i<n_cols;i++){
            fscanf(in_file,"%le",&xx);
        }
    }

    fclose(in_file);

    double chisq_min=exception_value;
    for(i=0;i<dalex_chisq.get_dim();i++){
        if(dalex_chisq.get_data(i)<chisq_min){
            chisq_min=dalex_chisq.get_data(i);
        }
    }

    array_1d<double> pt;
    pt.set_name("pt");
    array_2d<double> good_pts;
    good_pts.set_name("good_pts");

    word[0]=0;
    in_file = fopen(in_name, "r");
    while(compare_char("log", word)==0){
        fscanf(in_file,"%s",word);
        if(compare_char("#",word)==0){
            n_cols++;
        }
    }

    while(fscanf(in_file,"%le",&xx)>0){
        pt.set(0,xx);
        for(i=1;i<dim;i++){
            fscanf(in_file,"%le",&xx);
            pt.set(i,xx);
        }
        fscanf(in_file,"%le",&xx);
        if(xx<=chisq_min+delta_chisq){
            good_pts.add_row(pt);
        }
        for(i=dim+1;i<n_cols;i++){
            fscanf(in_file,"%le",&xx);
        }
    }
    fclose(in_file);

    array_1d<double> dx;
    dx.set_name("dx");
    array_1d<double> xmin,xmax;
    xmin.set_name("xmin");
    xmax.set_name("xmax");
    for(i=0;i<good_pts.get_rows();i++){
        for(j=0;j<dim;j++){
            if(j>=xmin.get_dim() || good_pts.get_data(i,j)<xmin.get_data(j)){
                xmin.set(j,good_pts.get_data(i,j));
            }
            if(j>=xmax.get_dim() || good_pts.get_data(i,j)>xmax.get_data(j)){
                xmax.set(j,good_pts.get_data(i,j));
            }
        }
    }

    printf("set min max %d %d %e\n",dim,xmax.get_dim(),chisq_min);
    printf("good pts %d %e\n",good_pts.get_rows(),delta_chisq);

    for(i=0;i<dim;i++){
        dx.set(i,pixel_factor*(xmax.get_data(i)-xmin.get_data(i)));
    }

    int ix, iy;

    array_2d<int> pixel_list;
    pixel_list.set_name("pixel_list");
    array_1d<int> pixel;
    pixel.set_name("pixel");
    int is_valid,is_same;
    for(i=0;i<good_pts.get_rows();i++){
        pixellate(good_pts(i),dx,xmin,pixel);
        is_valid=1;
        for(j=0;j<pixel_list.get_rows();j++){
            is_same=1;
            for(k=0;k<dim;k++){
                if(pixel_list.get_data(j,k)!=pixel.get_data(k)){
                    is_same=0;
                    break;
                }
            }
            if(is_same==1){
                is_valid=0;
                break;
            }
        }
        if(is_valid==1){
            pixel_list.add_row(pixel);
        }
    }

    printf("n pixels %d\n",pixel_list.get_rows());
    FILE *out_file;
    out_file=fopen("pixel_test.txt","w");
    for(i=0;i<pixel_list.get_rows();i++){
        for(j=0;j<dim;j++){
            fprintf(out_file,"%le ",xmin.get_data(j)+pixel_list.get_data(i,j)*dx.get_data(j));
        }
        fprintf(out_file,"\n");
    }
    fclose(out_file);
}

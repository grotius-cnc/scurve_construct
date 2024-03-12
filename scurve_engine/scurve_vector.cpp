#include "scurve_vector.h"

scurve_vector::scurve_vector()
{

}

// Assuming you have allocated memory for scurve_vector
//   scurve_vector* myVector = (scurve_vector*)malloc(sizeof(scurve_vector));

// Now you can use myVector as a pointer to scurve_vector

// Don't forget to free the allocated memory when done
// free(myVector);

extern "C" {
    scurve_vector* vector_init_ptr_c() {
        return new scurve_vector();
    }
}

extern "C" void vector_add_segment_c(scurve_vector *ptr, struct sc_segment b){
    ptr->pvec.push_back(b);
}

extern "C" void vector_at_id_set_tarpositions_c(scurve_vector *ptr){

    int n=ptr->pvec.size()-1;
    if(ptr->pvec.size()==1){
        ptr->pvec.at(n).lenbeg=0;
        ptr->pvec.at(n).lenend=ptr->pvec.at(n).path_lenght;
    }
    if(ptr->pvec.size()>1){
        ptr->pvec.at(n).lenbeg=ptr->pvec.at(n-1).lenend;
        ptr->pvec.at(n).lenend=ptr->pvec.at(n-1).lenend+ptr->pvec.at(n).path_lenght;
    }

    // std::cout<<"vector nr:"<<n<<std::endl;
    // std::cout<<"lenbeg:"<<ptr->pvec.at(n).lenbeg<<std::endl;
    // std::cout<<"lenend:"<<ptr->pvec.at(n).lenend<<std::endl;
}

extern "C" int vector_size_c(scurve_vector *ptr){
    return ptr->pvec.size();
}

extern "C" void vector_clear_c(scurve_vector *ptr){
    return ptr->pvec.clear();
}

extern "C" int vector_at_id_c(scurve_vector *ptr, int n){
    return ptr->pvec.at(n).primitive_id;
}

extern "C" struct sc_segment vector_at_c(scurve_vector *ptr, size_t index){
    if(index>ptr->pvec.size()-1){
        index=ptr->pvec.size()-1;
    }
    if(index<0){
        index=0;
    }
    return ptr->pvec.at(index);
}

extern "C" void vector_nr_increment(scurve_vector *ptr, int *n){
    *n=*n+1;
    *n=fmin(*n,vector_size_c(ptr));
    *n=fmax(*n,0);
}

extern "C" void vector_nr_decrement(scurve_vector *ptr, int *n){
    *n=*n-1;
    *n=fmin(*n,vector_size_c(ptr));
    *n=fmax(*n,0);
}

extern "C" void vector_traject_lenght(scurve_vector *ptr, double *l){
    for(size_t i=0; i<ptr->pvec.size(); i++){
        *l+=ptr->pvec.at(i).path_lenght;
    }
}















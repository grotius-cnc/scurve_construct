#ifndef SCURVE_VECTOR_H
#define SCURVE_VECTOR_H

#ifdef __cplusplus

#include <vector>
#include <iostream>
#include <cmath>
#include "scurve_segment.h"

class scurve_vector
{
public:

    //! Constructor.
    scurve_vector();
    std::vector<sc_segment> pvec;

private:
};

//! Here it tells if this code is used in c, convert the class to a struct. This is handy!
#else
typedef struct scurve_vector scurve_vector;
#endif //! cplusplus

#endif // SCURVE_VECTOR_H

#ifndef SCURVE_SEGMENT_H
#define SCURVE_SEGMENT_H

#include "stdbool.h"
#include "math.h"

struct sc_pnt {
    double x, y, z;
};

struct sc_dir {
    double a, b, c;
};

struct sc_ext {
    double u, v, w;
};

enum sc_primitive_id {
    sc_line,
    sc_arc,
};

//! Inherent numbers to canon motion type.
enum sc_motion_type {
    sc_rapid=1,
    sc_linear=2,
    sc_circle=3,
};

//! One gcode line, cq segment.
//! Can be expanded to hold digital io values.
struct sc_segment {

    enum sc_primitive_id primitive_id;
    enum sc_motion_type type;

    int gcode_line_nr;

    //! Point s=start, w=waypoint, c=center.
    struct sc_pnt pnt_s, pnt_e, pnt_w, pnt_c;
    struct sc_dir dir_s, dir_e;
    struct sc_ext ext_s, ext_e;

    //! The angle to previous & next primitive.
    double angle_begin;
    double angle_end;

    //! If arc's velmax is reduced by gforce impact value, this is maxvel.
    //! Otherwise the velmax is set to program velmax.
    double vo;
    double vm;
    double ve;

    //! Store the lenght for the scurve planner.
    double path_lenght;

    //! Lenght begin, lenght end.
    double lenbeg, lenend;

    //! Store the radius, we can use this for look ahead tiny arc's.
    double radius;
};

#endif // SCURVE_SEGMENT_H

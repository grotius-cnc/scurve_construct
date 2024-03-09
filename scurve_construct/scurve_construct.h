#ifndef SCURVE_CONSTRUCT_H
#define SCURVE_CONSTRUCT_H

#include "math.h"
#include "scurve_struct.h"

#ifdef __cplusplus

double ttot_period(scurve_period p);
double stot_period(scurve_period p);

double delvelocity(scurve_data d);
double ttot(scurve_data d);
double stot(scurve_data d);
double vel(scurve_data d);
double acc(scurve_data d);
double pos(scurve_data d);

//! Class to construct a forward or stop scurve using different functions.
class scurve_construct
{
public:
    scurve_construct();

    scurve_data  set_init_values(double jerk_max,
                         double acceleration_max,
                         double maximum_velocity,
                         double cycletime,
                         scurve_data s);

    void zero_period(scurve_period &p);

    void jog_velocity(scurve_data &s, int status, double tarpos);

    void jog_position_master(scurve_data &s, int status, double tarpos);
    void jog_position_fwd(scurve_data &s, int status, double tarpos);
    void jog_position_rev(scurve_data &s, int status, double tarpos);

    void forward_curve_build(scurve_data &s);
    void stop_curve_build(scurve_data &s);

    void stop_lenght(scurve_data &s, double &lenght, double &time);

    void curve_play(scurve_data &s);

    void t1_t2_t3_build(double jermax,
                        double accinf,
                        double curvel,
                        double endvel,
                        scurve_period &c1,
                        scurve_period &c2,
                        scurve_period &c3);

    void t1_build(double jermax,
                  double curvel,
                  double curacc,
                  double endacc,
                  scurve_period &c1);

    void t2_build(double curvel,
                  double endvel,
                  double accinf,
                  scurve_period &c2);

    void t3_build(double jermax,
                  double accinf,
                  double curvel,
                  double curacc,
                  double endacc,
                  scurve_period &c3);

    void t4_build(double curvel,
                  scurve_period &c4);

    void t1_play(double at_time,
                 double accinf,
                 double jermax,
                 double timbeg,
                 double timend,
                 double velini,
                 double disbeg,
                 double &velend,
                 double &disend,
                 double &accend);

    void t2_play(double attime,
                 double accinf,
                 double timsta,
                 double timend,
                 double velbeg,
                 double &velend,
                 double &disend,
                 double &accend);

    void t3_play(double attime,
                 double accinf,
                 double jermax,
                 double timbeg,
                 double timend,
                 double velini,
                 double disbeg,
                 double &velend,
                 double &disend,
                 double &accend);

    void t4_play(double attime,
                 double timsta,
                 double velbeg,
                 double &velend,
                 double &disend,
                 double &accend);

    // "dv"  Delta velocity, dv=vo-ve, derived from: t1=2*(ve-vo)/as;
    // A t1,t3 curve using as is excact dv. Then 0.5*dv is a t1 or a t3.
    double delta_vel(double jm, double as);

    // When vo to ve < dv, use custom as.
    // This results in t1 with ace, t3 with acs.
    // Valid for curvel with zero acceleration, endvel with zero acceleration.
    void t1_t3_custom_as(double jm, double curvel, double endvel, double &as);

    // The netto difference between two values.
    double diff(double a, double b);
};

//! Here it tells if this code is used in c, convert the class to a struct. This is handy!
#else
typedef struct scurve_construct scurve_construct;
#endif //! cplusplus

#endif // SCURVE_CONSTRUCT_H













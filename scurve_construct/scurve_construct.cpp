#include "scurve_construct.h"
#include "iostream"

scurve_construct::scurve_construct()
{

}

double ttot_period(scurve_period p){    // Time total.
    double ttot=p.timend-p.timbeg;
    return ttot;
}
double stot_period(scurve_period p){    // Displacement total.
    double stot=p.disend-p.disbeg;
    return stot;
}
double delvelocity(scurve_data d){
    double jm=d.jermax;                 // "jm"  Jerk max. Steepness value of a scurve.
    double as=d.maxacc*2;               // "as"  Max acceleration at inflection point. as=2*A.
    double dvt=2*as/jm;                 // "dvt" Delta velocity time, derived from: jm=2*as/t1.
    double dv=fabs((dvt*as)/2);         // "dv"  Delta velocity, dv=vo-ve, derived from: t1=2*(ve-vo)/as;
    return dv;
}
double ttot(scurve_data d){             // Total time of a forward or stop curve.
    return ttot_period(d.c0)+ttot_period(d.c1)+ttot_period(d.c2)+
            ttot_period(d.c3)+ttot_period(d.c4);
}
double stot(scurve_data d){             // Total displacement of a forward or stop curve.
    return stot_period(d.c0)+stot_period(d.c1)+stot_period(d.c2)+
            stot_period(d.c3)+stot_period(d.c4);
}
double vel(scurve_data d){              // Current velocity.
    if(d.revers){
        return -fabs(d.vr);
    }
    return d.vr;
}
double acc(scurve_data d){              // Current acceleration.
    if(d.revers){
        return -fabs(d.ar);
    }
    return d.ar;
}
double pos(scurve_data d){              // Currernt position.
    return d.incpos;
}

extern "C" struct scurve_data set_init_values_c(double jermax,
                                                double accmax,
                                                double maxvel,
                                                double cyctim,
                                                struct scurve_data data){
    return scurve_construct().set_init_values(jermax,accmax,maxvel,cyctim,data);
}
extern "C" struct scurve_data jog_update_c(struct scurve_data data){
    scurve_construct().jog_update(data);
    return data;
}
extern "C" struct scurve_data jog_velocity_c(struct scurve_data data, int enable, double endvel, double endacc, double tarpos){
    scurve_construct().jog_velocity(data,enable,endvel,endacc,tarpos);
    return data;
}
extern "C" struct scurve_data jog_position_c(struct scurve_data data, int enable, double endvel, double endacc, double tarpos, int jog_fwd, int jog_rev){
    scurve_construct().jog_position_master(data,enable,endvel,endacc,tarpos,jog_fwd,jog_rev);
    return data;
}

extern "C" void jog_results_c(struct scurve_data data, double *velocity, double *acceleration, double *position, int *finished){
    double v,a,p;
    int f;
    scurve_construct().jog_results(data,v,a,p,f);
    *velocity=v;
    *acceleration=a;
    *position=p;
    *finished=f;
}

scurve_data scurve_construct::set_init_values(double jerk_max,
                                              double acceleration_max,
                                              double maximum_velocity,
                                              double cycletime,
                                              scurve_data data){

    data.jermax=jerk_max;
    data.maxacc=acceleration_max;
    data.maxvel=maximum_velocity;
    data.intval=cycletime;
    return data;
}

void scurve_construct::zero_period(scurve_period &p){
    p.accbeg=0;
    p.accend=0;
    p.velini=0;
    p.velbeg=0;
    p.velend=0;
    p.disbeg=0;
    p.disend=0;
    p.timbeg=0;
    p.timend=0;
    p.jermax=0;
    p.accinf=0;
}

void scurve_construct::jog_velocity(scurve_data &s, int enable, double endvel, double endacc, double tarpos){

    s.tarpos=tarpos;
    s.modpos=0;         // Keep zero, otherwise it will set tarpos at finish.
    s.endvel=endvel;
    s.endacc=endacc;

    // Jog stop first.
    if(s.guivel<0 && s.guipos<s.tarpos){
        s.endvel=0;
        s.endacc=0;
        scurve_construct().stop_curve_build(s);
        // std::cout<<"stopcurve change direction."<<std::endl;
        return;
    }
    if(s.guivel>0 && s.guipos>s.tarpos){
        s.endvel=0;
        s.endacc=0;
        scurve_construct().stop_curve_build(s);
        // std::cout<<"stopcurve change direction."<<std::endl;
        return;
    }

    // Jog program dir.
    if(s.guivel==0 && s.guipos<s.tarpos ){
        s.revers=0;
    }
    if(s.guivel==0 && s.guipos>s.tarpos ){
        s.revers=1;
    }

    if(enable){
        scurve_construct().forward_curve_build(s);
        // std::cout<<"forward curve."<<std::endl;
    }
    if(!enable){
        if(s.vr<s.endvel){ // If velocity is below the endvel, endvel is set to zero.
            s.endvel=0;
        }
        scurve_construct().stop_curve_build(s);
        // std::cout<<"stopcurve to ve:"<<s.endvel<<std::endl;
    }

    s.finish=0;
    if(s.vr==s.endvel && s.guiacc==s.endacc){ // s.vr is the abs positive of s.guivel.
        s.finish=1;
        // std::cout<<"jogging velocity mode finished."<<std::endl;
    }
}

void scurve_construct::jog_position_master(scurve_data &s, int enable, double endvel, double endacc, double tarpos, int jog_fwd, int jog_rev){

    s.finish=0;
    s.tarpos=tarpos;
    s.modpos=1;             // Enable position limit to tarpos.
    s.endvel=endvel;
    s.endacc=endacc;
    s.pd.btn_fwd=jog_fwd;   // Button press, release forward.
    s.pd.btn_rev=jog_rev;   // Button press, release reverse.

    if(!enable){
         s.pd.stopinit=0;
    }

    if(s.pd.btn_fwd && s.guivel<0){ // Jog stop first before changing direction.
        s.endvel=0;
        s.endacc=0;
        scurve_construct().stop_curve_build(s);
        // std::cout<<"direction change to ve=0."<<std::endl;
        return;
    }
    if(s.pd.btn_rev && s.guivel>0){ // Jog stop first before changing direction.
        s.endvel=0;
        s.endacc=0;
        scurve_construct().stop_curve_build(s);
        // std::cout<<"direction change to ve=0."<<std::endl;
        return;
    }

    if(s.pd.btn_fwd && s.guipos>=s.tarpos){
        scurve_construct().stop_curve_build(s);
        s.finish=1;
        // std::cout<<"finished fwd."<<std::endl;
        return;
    }
    if(s.pd.btn_rev && s.guipos<=s.tarpos){
        scurve_construct().stop_curve_build(s);
        s.finish=1;
        // std::cout<<"finished rev."<<std::endl;
        return;
    }

    // Jog direction.
    if(s.guipos<s.tarpos ){
        jog_position_fwd(s,enable,tarpos);
        // std::cout<<"jogging fwd to ve:"<<s.endvel<<std::endl;
        return;
    }
    if(s.guipos>s.tarpos ){
        jog_position_rev(s,enable,tarpos);
        // std::cout<<"jogging rev to ve:"<<s.endvel<<std::endl;
        return;
    }
}

void scurve_construct::jog_position_rev(scurve_data &s, int enable, double tarpos){

    s.tarpos=tarpos;
    s.revers=1;

    if(!enable){
        s.pd.stopinit=0;
    }

    stop_lenght(s,s.pd.stopdist,s.pd.stoptime);
    s.pd.stopdist=-abs(s.pd.stopdist);

    s.pd.overshoot=(s.guipos+s.pd.stopdist)-s.tarpos;
    // std::cout<<"overshoot:"<<s.pd.overshoot<<std::endl;

    if(s.guivel<=0 && s.pd.overshoot<0){
        if(!s.pd.stopinit){
            int cycles=s.pd.stoptime/s.intval;
            s.pd.dist_remove_a_cycle=s.pd.overshoot/cycles;
            s.pd.stopinit=1;
        }
        scurve_construct().stop_curve_build(s);
        s.guipos+=s.pd.dist_remove_a_cycle;
        return;
    }

    if(enable){
        scurve_construct().forward_curve_build(s);
    }
    if(!enable){
        //if(s.vr<s.endvel){ // If velocity is below the endvel, endvel is set to zero.
            s.endvel=0;
        //}
        scurve_construct().stop_curve_build(s);
    }
}

void scurve_construct::jog_position_fwd(scurve_data &s, int enable, double tarpos){

    s.tarpos=tarpos;
    s.revers=0;

    if(!enable){
        s.pd.stopinit=0;
    }

    stop_lenght(s,s.pd.stopdist,s.pd.stoptime);
    s.pd.overshoot=(s.guipos+s.pd.stopdist)-tarpos;
    // std::cout<<"overshoot:"<<s.pd.overshoot<<std::endl;

    if(s.guivel>=0 && s.pd.overshoot>0){
        if(!s.pd.stopinit){
            int cycles=s.pd.stoptime/s.intval;
            s.pd.dist_remove_a_cycle=s.pd.overshoot/cycles;
            s.pd.stopinit=1;
        }
        scurve_construct().stop_curve_build(s);
        s.guipos-=s.pd.dist_remove_a_cycle;
        return;
    }

    if(enable){
        scurve_construct().forward_curve_build(s);
    }
    if(!enable){
        //if(s.vr<s.endvel){ // If velocity is below the endvel, endvel is set to zero.
            s.endvel=0;
        //}
        scurve_construct().stop_curve_build(s);
    }
}

void scurve_construct::stop_lenght(scurve_data &s, double &lenght, double &time){
    scurve_data data=s;
    stop_curve_build(data);
    lenght=stot_period(data.c0)+stot_period(data.c1)+stot_period(data.c2)+stot_period(data.c3);
    time=ttot_period(data.c0)+ttot_period(data.c1)+ttot_period(data.c2)+ttot_period(data.c3);
}

//! Stop scurve algoritme.
//! Can handle positive or negative begin acceleration values.
void scurve_construct::stop_curve_build(scurve_data &s){

    double jermax=s.jermax;
    double accinf=s.maxacc*2;
    double curvel=s.vr;
    double curacc=s.ar;
    double delvel=delvelocity(s);
    double endacc=s.endacc;
    double endvel=s.endvel;

    s.oldpos+=s.sr;
    s.sr=0;
    s.curtim=0;

    zero_period(s.c0);
    zero_period(s.c1);
    zero_period(s.c2);
    zero_period(s.c3);
    zero_period(s.c4);

    if(curacc==0){ // Curve from curvel to endvel.
        jermax=-fabs(jermax);
        accinf=-fabs(accinf);
        t1_t2_t3_build(jermax,
                       accinf,
                       curvel,
                       endvel,
                       s.c1,
                       s.c2,
                       s.c3);
    }

    if(curacc>0){ // Add acceleration to zero period.
        jermax=fabs(jermax);
        accinf=fabs(accinf);
        t3_build(jermax,accinf,curvel,curacc,endacc,s.c0);

        jermax=-fabs(jermax);
        accinf=-fabs(accinf);
        t1_t2_t3_build(jermax,
                       accinf,
                       s.c0.velend,
                       endvel,
                       s.c1,
                       s.c2,
                       s.c3);
    }

    if(curacc<0){
        jermax=-fabs(jermax);
        accinf=-fabs(accinf);
        t3_build(jermax,accinf,curvel,curacc,endacc,s.c3); // Check if t3 is valid to endvel.
        if(s.c3.velend==endvel){
            // std::cout<<"use t3 only."<<std::endl;
        } else if(curacc==accinf){ // If at maxacc, use period t2+t3.
            t2_build(curvel,0.5*delvel,accinf,s.c2);
            t3_build(jermax,accinf,s.c2.velend,accinf,endacc,s.c3);
            if(s.c3.velend==endvel){
                // std::cout<<"use t2,t3."<<std::endl;
            }
        } else { // Use periods t1,t2,t3, check for custom accinf.
            t1_build(jermax,curvel,curacc,accinf,s.c1); // Get vo of t1.
            double vo=s.c1.velini;
            // Get as of t1,t3.
            t1_t2_t3_build(jermax,accinf,vo,endvel,s.c1,s.c2,s.c3); // Get eventual custom as.
            double as=s.c1.accend;
            t1_build(jermax,curvel,curacc,as,s.c1); // Apply custom as.
            // std::cout<<"use t1,t2,t3, accinf:"<<as<<std::endl;
        }
    }
    t4_build(s.endvel,s.c4);
    s.c4.timbeg=0;
    s.c4.timend=INFINITY;
    s.c4.velbeg=s.endvel;
    s.c4.velend=s.endvel;
    s.c4.disbeg=0;
    s.c4.disend=INFINITY;
    s.c4.accbeg=0;
    s.c4.accend=0;
}

//! Forward scurve algoritme.
//! Can handle positive or negative begin acceleration values.
void scurve_construct::forward_curve_build(scurve_data &s){

    double jermax=s.jermax;
    double accinf=s.maxacc*2;
    double maxvel=s.maxvel;
    double curvel=s.vr;
    double curacc=s.ar;
    double delvel=delvelocity(s);
    double endacc=0;

    s.oldpos+=s.sr;
    s.sr=0;
    s.curtim=0;

    zero_period(s.c0);
    zero_period(s.c1);
    zero_period(s.c2);
    zero_period(s.c3);
    zero_period(s.c4);

    if(curacc==0){
        // Add t1,t2,t3 period to velmax. Positve curve.
        jermax=fabs(jermax);
        accinf=fabs(accinf);
        t1_t2_t3_build(jermax,
                       accinf,
                       curvel,
                       maxvel,
                       s.c1,
                       s.c2,
                       s.c3);
    }
    if(curacc<0){
        // First to acceleration zero using t3 curve.
        jermax=-fabs(jermax);
        accinf=-fabs(accinf);
        t3_build(jermax,accinf,curvel,curacc,endacc,s.c0);

        // Add t1,t2,t3 period to velmax. Positve curve.
        jermax=fabs(jermax);
        accinf=fabs(accinf);
        t1_t2_t3_build(jermax,
                       accinf,
                       s.c0.velend,
                       maxvel,
                       s.c1,
                       s.c2,
                       s.c3);
    }
    if(curacc>0){
        jermax=fabs(jermax);
        accinf=fabs(accinf);
        t3_build(jermax,accinf,curvel,curacc,endacc,s.c3); // Check if t3 is valid to endvel.

        if(s.c3.velend==maxvel){
            // std::cout<<"use t3 only."<<std::endl;
        } else if(curacc==accinf){ // If at maxacc, use period t2+t3.
            t2_build(curvel,maxvel-(0.5*delvel),accinf,s.c2);
            t3_build(jermax,accinf,s.c2.velend,accinf,endacc,s.c3);
            if(s.c3.velend==maxvel){
                // std::cout<<"use t2,t3."<<std::endl;
            }
        } else { // Use periods t1,t2,t3, check for custom accinf.
            t1_build(jermax,curvel,curacc,accinf,s.c1); // Get vo of t1.
            double vo=s.c1.velini;
            // Get as of t1,t3.
            t1_t2_t3_build(jermax,accinf,vo,maxvel,s.c1,s.c2,s.c3); // Get eventual custom as.
            double as=s.c1.accend;
            t1_build(jermax,curvel,curacc,as,s.c1); // Apply custom as.
            // std::cout<<"use t1,t2,t3, accinf:"<<as<<std::endl;
        }
    }

    t4_build(s.maxvel,s.c4);
    s.c4.timbeg=0;
    s.c4.timend=INFINITY;
    s.c4.velbeg=s.maxvel;
    s.c4.disbeg=0;
    s.c4.disend=INFINITY;
    s.c4.accbeg=0;
    s.c4.accend=0;
}

//! Calculates netto difference between to values.
//! For example difference -100, 100 would be 200.
double scurve_construct::diff(double a, double b){

    if(a<=0 && b>=0){
        return fabs(a)+b;
    }
    if(a>=0 && b<=0){
        return a+fabs(b);
    }
    if(a<=0 && b<=0){
        return fabs(a)-fabs(b);
    }
    // (a>=0 && b>=0){
    return a-b;
}

//! Run a scurve algoritme given the build values.
//! Calculate when the stop curve is valid.
//! Try to end at given endpoint.
//! Returns 1, when finished.
void scurve_construct::jog_update(scurve_data &s){

    s.curtim+=s.intval;
    s.oldpos=s.sr;

    if(s.curtim<ttot_period(s.c0)){
        t3_play(s.curtim,
                s.c0.accinf,
                s.c0.jermax,
                s.c0.timbeg,
                s.c0.timend,
                s.c0.velini,
                s.c0.disbeg,
                s.vr,s.sr,s.ar);
    }
    if(s.curtim>=ttot_period(s.c0) && s.curtim<=ttot_period(s.c0)+ttot_period(s.c1)){
        t1_play(s.curtim-ttot_period(s.c0),
                s.c1.accinf,
                s.c1.jermax,
                s.c1.timbeg,
                s.c1.timend,
                s.c1.velini,
                s.c1.disbeg,
                s.vr,s.sr,s.ar);
        s.sr+=stot_period(s.c0);
    }
    if(s.curtim>ttot_period(s.c0)+ttot_period(s.c1) && s.curtim<ttot_period(s.c0)+ttot_period(s.c1)+ttot_period(s.c2)){
        t2_play(s.curtim-(ttot_period(s.c0)+ttot_period(s.c1)),
                s.c2.accinf,
                s.c2.timbeg,
                s.c2.timend,
                s.c2.velbeg,
                s.vr,s.sr,s.ar);
        s.sr+=stot_period(s.c0)+stot_period(s.c1);
    }
    if(s.curtim>=ttot_period(s.c0)+ttot_period(s.c1)+ttot_period(s.c2) && s.curtim<=ttot_period(s.c0)+ttot_period(s.c1)+ttot_period(s.c2)+ttot_period(s.c3)){
        t3_play(s.curtim-(ttot_period(s.c0)+ttot_period(s.c1)+ttot_period(s.c2)),
                s.c3.accinf,
                s.c3.jermax,
                s.c3.timbeg,
                s.c3.timend,
                s.c3.velini,
                s.c3.disbeg,
                s.vr,s.sr,s.ar);
        s.sr+=stot_period(s.c0)+stot_period(s.c1)+stot_period(s.c2);
    }
    if(s.curtim>ttot_period(s.c0)+ttot_period(s.c1)+ttot_period(s.c2)+ttot_period(s.c3)){
        t4_play(s.curtim-(ttot_period(s.c0)+ttot_period(s.c1)+ttot_period(s.c2)+ttot_period(s.c3)),
                s.c4.timbeg,
                s.c4.velbeg,
                s.vr,s.sr,s.ar);
        s.sr+=stot_period(s.c0)+stot_period(s.c1)+stot_period(s.c2)+stot_period(s.c3);
    }

    s.incpos=s.sr-s.oldpos;

    if(s.revers){
        s.guipos-=s.incpos;
        s.guivel=-abs(s.vr);
        s.guiacc=-abs(s.ar);
    }
    if(!s.revers){
        s.guipos+=s.incpos;
        s.guivel=s.vr;
        s.guiacc=s.ar;
    }

    if(s.modpos && s.finish){
        s.guipos=s.tarpos;
    }
}

void scurve_construct::jog_results(scurve_data s, double &velocity, double &acceleration, double &position, int &finished){
    velocity=s.guivel;
    acceleration=s.guiacc;
    position=s.guipos;
    finished=s.finish;
}

//! Play convex period t3.
void scurve_construct::t3_play(double attime,
                               double accinf,
                               double jermax,
                               double timbeg,
                               double timend,
                               double velini,
                               double disbeg,
                               double &velend,
                               double &disend,
                               double &accend){
    double jm=jermax;
    double as=accinf;
    double ts=timbeg;
    double te=timend;
    double vo=velini;

    ts+=attime;
    ts=fmin(ts,te);

    double vr=vo + as*ts - jm*(ts*ts)/2;                  // v=vo + as*t - jm*(t*t)/2;
    double sr=vo*ts + as*(ts*ts)/2 - jm*(ts*ts*ts)/6;     // s=vo*t + as*(t*t)/2 - jm*(t*t*t)/6;
    double ar=as-jm*ts;                                   // a=as-jm*t;

    velend=vr;
    disend=sr;
    accend=ar;

    disend-=disbeg;
}

//! Buld convex period t3.
void scurve_construct::t3_build(double jermax,
                                double accinf,
                                double curvel,
                                double curacc,
                                double endacc,
                                scurve_period &c3){
    double vrs,srs,ars;
    double vre,sre,are;

    double jm=jermax;
    double as=accinf;

    double ts=(as-curacc)/jm;   // Time start.
    double te=(as-endacc)/jm;   // Time end.

    double vo=curvel-as*ts+0.5*jm*(ts*ts);                 // Derived from v=vh+as*t-jm*(t*t)/2;

    vrs=vo + as*ts - jm*(ts*ts)/2;                  // v=vo + as*t - jm*(t*t)/2;
    srs=vo*ts + as*(ts*ts)/2 - jm*(ts*ts*ts)/6;     // s=vo*t + as*(t*t)/2 - jm*(t*t*t)/6;
    ars=as-jm*ts;                                   // a=as-jm*t;

    vre=vo + as*te - jm*(te*te)/2;                  // v=vo + as*t - jm*(t*t)/2;
    sre=vo*te + as*(te*te)/2 - jm*(te*te*te)/6;     // s=vo*t + as*(t*t)/2 - jm*(t*t*t)/6;
    are=as-jm*te;                                   // a=as-jm*t;

    c3.disbeg=srs; // Displacement at time start.
    c3.disend=sre; // Displacement at time end.
    c3.velend=vre;
    c3.velini=vo;
    c3.accbeg=curacc;
    c3.accend=endacc;
    c3.timbeg=ts;  // Time begin.
    c3.timend=te;  // Time end.
    c3.accinf=as;
    c3.jermax=jm;
}

//! Play concave period t1.
void scurve_construct::t1_play(double at_time,
                               double accinf,
                               double jermax,
                               double timbeg,
                               double timend,
                               double velini,
                               double disbeg,
                               double &velend, double &disend, double &accend){
    double as=accinf;
    double jm=jermax;
    double ts=timbeg;
    double te=timend;
    double vo=velini;

    ts+=at_time;
    ts=fmin(ts,te);

    double vr=vo + jm*(ts*ts)/2;          // vo+jm*(t*t)/2;
    double sr=vo*ts + jm*(ts*ts*ts)/6;    // vo*t+jm*(t*t*t)/6;
    double ar=jm*ts;                      // jm*t;

    velend=vr;
    disend=sr;
    accend=ar;

    disend-=disbeg;
}

//! Build concave period t1.
void scurve_construct::t1_build(double jermax,
                                double curvel,
                                double curacc,
                                double endacc,
                                scurve_period &c1){
    double vrs,srs,ars;
    double vre,sre,are;

    double jm=jermax;

    double ts=curacc/jm; // Time start.
    double te=endacc/jm; // Time end.

    double vo=curvel-(jm*ts*ts)/2.0;       // Calculate the vo given the start time ts.

    vrs=vo + jm*(ts*ts)/2;          // vo+jm*(t*t)/2;
    srs=vo*ts + jm*(ts*ts*ts)/6;    // vo*t+jm*(t*t*t)/6;
    ars=jm*ts;                      // jm*t;

    vre=vo + jm*(te*te)/2;          // vo+jm*(t*t)/2;
    sre=vo*te + jm*(te*te*te)/6;    // vo*t+jm*(t*t*t)/6;
    are=jm*te;                      // jm*t;

    c1.velini=vo;
    c1.velend=vre;
    c1.disbeg=srs;
    c1.disend=sre;
    c1.accbeg=curacc;
    c1.accend=endacc;
    c1.timbeg=ts;
    c1.timend=te;
    c1.jermax=jm;
}

//! Play linear acceleration period t2.
void scurve_construct::t2_play(double attime,
                               double accinf,
                               double timsta,
                               double timend,
                               double velbeg,
                               double &velend,
                               double &disend,
                               double &accend){
    double ts=timsta;
    double te=timend;
    double vo=velbeg;
    double as=accinf;

    ts+=attime;
    ts=fmin(ts,te);

    double vr=vo+as*ts; //! Linear acc.
    double sr=(vr*vr-vo*vo)/(2*as);    // (vr*vr-v1*v1)/(2*as);
    double ar=as;

    velend=vr;
    disend=sr;
    accend=ar;
}

//! Build linear acceleration period t2.
void scurve_construct::t2_build(double curvel,
                                double endvel,
                                double accinf,
                                scurve_period &c2){
    double vo=curvel;
    double ve=endvel;
    double as=accinf;

    double tr=(ve-vo)/as;              // Derived from vo-vr=as*t, vr=vo+as*t
    double sr=(ve*ve-vo*vo)/(2*as);    // (vr*vr-v1*v1)/(2*as);

    if(vo==ve){
        tr=0;
        sr=0;
    }

    c2.velini=curvel;
    c2.velbeg=curvel;
    c2.velend=endvel;
    c2.disbeg=0;
    c2.disend=sr;
    c2.accbeg=accinf;
    c2.accend=accinf;
    c2.timbeg=0;
    c2.timend=tr;
    c2.accinf=accinf;
}

//! Play steady period t4.
void scurve_construct::t4_play(double attime,
                               double timsta,
                               double velbeg,
                               double &velend,
                               double &disend,
                               double &accend){
    double ts=timsta;
    double vo=velbeg;

    ts+=attime;

    double vr=vo;
    double sr=vo*ts;
    double ar=0;

    velend=vr;
    disend=sr;
    accend=ar;
}

//! Build steady period t4.
void scurve_construct::t4_build(double curvel, scurve_period &c4){

    c4.velini=curvel;
    c4.velbeg=curvel;
    c4.velend=curvel;
    c4.disbeg=0;
    c4.disend=0;
    c4.accbeg=0;
    c4.accend=0;
    c4.timbeg=0;
    c4.timend=0;
}

//! Build a t1,t2,t3 curve using zero acceleration begin & end.
void scurve_construct::t1_t2_t3_build(double jermax,
                                      double accinf,
                                      double curvel,
                                      double endvel,
                                      scurve_period &c1,
                                      scurve_period &c2,
                                      scurve_period &c3){

    double dv=delta_vel(jermax,accinf);

    double velshif=endvel-curvel;

    double zeroac=0; // Zero acceleration.

    if(velshif<dv){ // Using t1 ace, t3 acs.

        double custas=0;                    // Custom as at inflection point.
        double velhal=(curvel+endvel)*0.5;  // Velocity half.

        t1_t3_custom_as(jermax,curvel,endvel,custas);

        t1_build(jermax,
                 curvel,
                 zeroac,
                 custas,
                 c1);

        t2_build(velhal,
                 velhal,
                 custas,
                 c2);

        t3_build(jermax,
                 accinf,
                 velhal,
                 custas,
                 zeroac,
                 c3);
    }
    if(velshif>=dv){ // Using full t1,t3, add t2.

        t1_build(jermax,
                 curvel,
                 zeroac,
                 accinf,
                 c1);

        double v=velshif-dv;

        t2_build(c1.velend,
                 c1.velend+v,
                 accinf,
                 c2);

        t3_build(jermax,
                 accinf,
                 c2.velend,
                 accinf,
                 zeroac,
                 c3);
    }
}

// "dv"  Delta velocity, dv=vo-ve, derived from: t1=2*(ve-vo)/as;
// A t1,t3 curve using as is excact dv. Then 0.5*dv is a t1 or a t3.
double scurve_construct::delta_vel(double jm, double as){
    double dvt=2*as/jm;
    double dv=fabs((dvt*as)/2);
    return dv;
}

// When vo to ve < dv, use custom as.
// This results in t1 with ace, t3 with acs.
// Valid for curvel with zero acceleration, endvel with zero acceleration.
void scurve_construct::t1_t3_custom_as(double jm, double curvel, double endvel, double &as){
    double vo=curvel;
    double ve=endvel;
    double vh=(vo+ve)/2;
    // t1, from vo to vh. From acc 0 to ace ?..
    double t1 = sqrt(2 * (vh - vo) / jm); // Derived from: vh=vo+jm*(t*t)/2
    double t2=0;
    double t3=t1;

    // Set new as value, if curve ve-vo>=dv the as=2*a.
    // If curve ve-vo<dv, the curve acceleration at the inflection point is calculated by:
    as=jm*t1;
}







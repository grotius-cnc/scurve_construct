#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "iostream"

#include "../scurve_engine/scurve_engine.h"
#include "../scurve_engine/scurve_vector.h"

scurve_data s;
double maxvel=0;
double maxacc=0;
double maxjer=0;
double trjlen=0; // Traject lenght.
double intval=0.01;
double curvel=0,curacc=0,curpos=0;
int pause=0;
int finished=0;
int run=0;
int nr=0;
double maxvel_fo=0; // Max velocity including feed overide.
double maxvel_fo_af=0; // Max velocity including feed overide & adaptive feed.
double qtime=0;
extern "C" struct scurve_data set_init_values_c(double jermax,
                                                double accmax,
                                                double maxvel,
                                                double cyctim,
                                                struct scurve_data data);
extern "C" struct scurve_data jog_update_c(struct scurve_data data);
extern "C" struct scurve_data jog_velocity_c(struct scurve_data data, int enable, double endvel, double endacc, double tarpos);
extern "C" struct scurve_data jog_position_c(struct scurve_data data, int enable, double endvel, double endacc, double tarpos, int jog_fwd, int jog_rev);
extern "C" void jog_results_c(struct scurve_data data, double *velocity, double *acceleration, double *position, int *finished);

scurve_vector* ptr;
extern "C" { scurve_vector* vector_init_ptr_c(); }
extern "C" void vector_add_segment_c(scurve_vector *ptr, struct sc_segment b);
extern "C" struct sc_segment vector_at_c(scurve_vector *ptr, size_t index);
extern "C" void vector_at_id_set_tarpositions_c(scurve_vector *ptr);
extern "C" int vector_size_c(scurve_vector *ptr);
extern "C" void vector_nr_increment(scurve_vector *ptr, int *n);
extern "C" void vector_nr_decrement(scurve_vector *ptr, int *n);
extern "C" void vector_traject_lenght(scurve_vector *ptr, double *l);

double lenght(sc_pnt p0, sc_pnt p1){
    return sqrt(pow(p1.x-p0.x,2)+pow(p1.y-p0.y,2)+pow(p1.z-p0.z,2));
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_CustomPlot( 0 ),
    timeInterval( 10 )
{
    ui->setupUi(this);
    setMinimumWidth( 1000 );
    setMinimumHeight( 700);
    // Allocate memory for the plot widget object
    m_CustomPlot = new QCustomPlot( this );
    // The next line is responsible for adding
    // the plot widget to our application's main window
    //setCentralWidget( m_CustomPlot ); // there is no need to add and promote a widget

    ui->gridLayout->addWidget(m_CustomPlot);

    // Add a graph to the plot widget ( the main graph )
    m_CustomPlot->addGraph();
    // Add a graph to the plot widget ( this will be a horizontal line )
    // that folows the current value of the main graph
    m_CustomPlot->addGraph();
    m_CustomPlot->addGraph();

    // tell the plot widget to display the x axis values as a time
    // instead of numbers
    m_CustomPlot->xAxis->setTickLabelType( QCPAxis::ltDateTime );
    // Set the x axis time string format
    m_CustomPlot->xAxis->setDateTimeFormat( "" );
    m_CustomPlot->xAxis->setAutoTickStep( true );
    m_CustomPlot->yAxis->setAutoTickStep( true );

    // Allocating memory for the text item that will
    // display the current value as a text.
    m_ValueIndex = new QCPItemText( m_CustomPlot );
    // Set the font of the text item
    m_ValueIndex->setFont(QFont(font().family(), 8)); // make font a bit larger
    // Add the text item to the plt widget
    m_CustomPlot->addItem( m_ValueIndex );

    // Change the color of the graphs
    QColor brushClr = Qt::green;
    brushClr.setAlphaF( .5 );
    m_CustomPlot->graph( 0 )->setBrush( brushClr );
    m_CustomPlot->graph( 0 )->setPen( QColor() );

    QColor brushClr1 = Qt::yellow;
    brushClr1.setAlphaF( .5 );
    m_CustomPlot->graph( 1 )->setBrush( brushClr1 );
    m_CustomPlot->graph( 1 )->setPen( QColor() );

    QColor brushClr2 = Qt::blue;
    brushClr2.setAlphaF( .5 );
    m_CustomPlot->graph( 2 )->setBrush( brushClr2 );
    m_CustomPlot->graph( 2 )->setPen( QColor() );

    // setup a timer and start it.
    timer = new QTimer(this);
    connect(timer, SIGNAL( timeout() ), this, SLOT( updatePlot() ) );
    timer->start( timeInterval );

    maxacc=ui->lineEdit_maxacc->text().toDouble();
    maxvel=ui->lineEdit_maxvel->text().toDouble();
    maxjer=ui->lineEdit_maxjer->text().toDouble();

    ptr = vector_init_ptr_c();

    sc_segment seg;
    seg.pnt_s={-50,0,0};
    seg.pnt_e={10,0,0};
    seg.vo=0;
    seg.ve=2;
    seg.primitive_id=sc_primitive_id::sc_line;
    seg.type=sc_motion_type::sc_linear;
    seg.gcode_line_nr=0;
    seg.path_lenght=lenght(seg.pnt_s,seg.pnt_e);
    vector_add_segment_c(ptr,seg);
    vector_at_id_set_tarpositions_c(ptr);

    seg.pnt_s={10,0,0};
    seg.pnt_e={20,0,0};
    seg.vo=2;
    seg.ve=0;
    seg.primitive_id=sc_primitive_id::sc_line;
    seg.type=sc_motion_type::sc_linear;
    seg.gcode_line_nr=0;
    seg.path_lenght=lenght(seg.pnt_s,seg.pnt_e);
    seg.lenbeg=vector_at_c(ptr,0).lenend;
    seg.lenend=vector_at_c(ptr,0).lenend+seg.path_lenght;
    vector_add_segment_c(ptr,seg);
    vector_at_id_set_tarpositions_c(ptr);

    seg.pnt_s={20,0,0};
    seg.pnt_e={50,0,0};
    seg.vo=2;
    seg.ve=maxvel;
    seg.primitive_id=sc_primitive_id::sc_line;
    seg.type=sc_motion_type::sc_linear;
    seg.gcode_line_nr=0;
    seg.path_lenght=lenght(seg.pnt_s,seg.pnt_e);
    seg.lenbeg=vector_at_c(ptr,0).lenend;
    seg.lenend=vector_at_c(ptr,0).lenend+seg.path_lenght;
    vector_add_segment_c(ptr,seg);
    vector_at_id_set_tarpositions_c(ptr);

    seg.pnt_s={50,0,0};
    seg.pnt_e={75,0,0};
    seg.vo=maxvel;
    seg.ve=0;
    seg.primitive_id=sc_primitive_id::sc_line;
    seg.type=sc_motion_type::sc_linear;
    seg.gcode_line_nr=0;
    seg.path_lenght=lenght(seg.pnt_s,seg.pnt_e);
    seg.lenbeg=vector_at_c(ptr,0).lenend;
    seg.lenend=vector_at_c(ptr,0).lenend+seg.path_lenght;
    vector_add_segment_c(ptr,seg);
    vector_at_id_set_tarpositions_c(ptr);

    seg.pnt_s={75,0,0};
    seg.pnt_e={100,0,0};
    seg.vo=0;
    seg.ve=0;
    seg.primitive_id=sc_primitive_id::sc_line;
    seg.type=sc_motion_type::sc_linear;
    seg.gcode_line_nr=0;
    seg.path_lenght=lenght(seg.pnt_s,seg.pnt_e);
    seg.lenbeg=vector_at_c(ptr,0).lenend;
    seg.lenend=vector_at_c(ptr,0).lenend+seg.path_lenght;
    vector_add_segment_c(ptr,seg);
    vector_at_id_set_tarpositions_c(ptr);

    vector_traject_lenght(ptr,&trjlen);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updatePlot()
{
    // Get the current time
    //QTime tm = QTime::currentTime();
    // Convert the curretn time to millseconds
    //qreal seconds = 60 * 60 * tm.hour() + 60 * tm.minute() + tm.second();
    //qreal timeValue = 1000 * seconds + tm.msec();
    // Add the time the x data buffer
    qtime+=intval;
    m_XData.append( qtime );


    if(run){

    double adafee=ui->doubleSpinBox_adafee->value();
    if(adafee<0){
        ui->checkBox_revmot_status->setChecked(1);
    } else {
        ui->checkBox_revmot_status->setChecked(0);
    }

    double feeove=ui->doubleSpinBox_feed_override->value();
    maxvel_fo=(feeove/100)*maxvel;
    maxvel_fo_af=(abs(adafee)/100)*maxvel_fo;

    if(!finished){ // Don't set a new velmax value on the edge of a segment to prevent a standstill.
        s=set_init_values_c(maxjer,maxacc,maxvel_fo_af,intval,s);
    }

    double endvel=0;
    double endacc=0;
    int jog_fwd=0;
    int jog_rev=0;
    double tarpos=0;
    double jog_tarpos=0;
    if(ui->checkBox_revmot->isChecked() || adafee<0){ // Reverse motion.
        jog_fwd=0;
        jog_rev=1;
        tarpos=vector_at_c(ptr,nr).lenbeg;
        endvel=vector_at_c(ptr,nr).vo;
        jog_tarpos=-INFINITY;
    } else { // Forward motion.
        jog_fwd=1;
        jog_rev=0;
        tarpos=vector_at_c(ptr,nr).lenend;
        endvel=vector_at_c(ptr,nr).ve;
        jog_tarpos=INFINITY;
    }

    if(!pause){
        int pos_enable=1;
        s=jog_position_c(s,pos_enable,endvel,endacc,tarpos,jog_fwd,jog_rev);
    } else {
        double jog_enable=0; // Stop jogging to zero velocity.
        double jog_endvel=0;
        double jog_endacc=0;
        s=jog_velocity_c(s,jog_enable,jog_endvel,jog_endacc,jog_tarpos); // Stop asap.
    }
    s=jog_update_c(s);


    jog_results_c(s,&curvel,&curacc,&curpos,&finished);

    if(finished && jog_fwd){
        vector_nr_increment(ptr,&nr);
    }
    if(finished && jog_rev){
        vector_nr_decrement(ptr,&nr);
    }

    }

    // Write graph values.
    qreal value=curvel;
    m_YData.append( value );

    value=curacc;
    m_YData1.append( value );

    value=curpos*0.1;
    m_YData2.append( value );

    ui->label_curvel->setText(QString::number(curvel,'f',10));
    ui->label_curacc->setText(QString::number(curacc,'f',10));
    ui->label_curpos->setText(QString::number(curpos,'f',10));
    ui->label_vectnr->setText(QString::number(nr,'f',0));
    ui->label_segbeg->setText(QString::number(vector_at_c(ptr,nr).lenbeg,'f',3));
    ui->label_segend->setText(QString::number(vector_at_c(ptr,nr).lenend,'f',3));
    ui->checkBox_finished->setChecked(finished);
    ui->checkBox_pause->setChecked(pause);
    ui->label_trjlen->setText(QString::number(trjlen,'f',3));
    ui->label_trjpro->setText(QString::number((curpos/trjlen),'f',2));
    ui->label_segpro->setText(QString::number(( (curpos - vector_at_c(ptr,nr).lenbeg) / vector_at_c(ptr,nr).path_lenght ),'f',2));
    ui->label_seg_vo->setText(QString::number(vector_at_c(ptr,nr).vo,'f',3));
    ui->label_seg_ve->setText(QString::number(vector_at_c(ptr,nr).ve,'f',3));

    // Keep the data buffers size under 100 value each,
    // so our moemoty won't explode with random numbers
    if( m_XData.size() > 500 ){
        m_XData.remove( 0 );
        m_YData.remove( 0 );
        m_YData1.remove( 0 );
        m_YData2.remove( 0 );
    }

    m_CustomPlot->graph( 0 )->setData( m_XData , m_YData );
    m_CustomPlot->graph( 1 )->setData( m_XData , m_YData2 );
    m_CustomPlot->graph( 2 )->setData( m_XData , m_YData1 );

    // Set the range of the vertical and horizontal axis of the plot ( not the graph )
    // so all the data will be centered. first we get the min and max of the x and y data
    QVector<double>::iterator xMaxIt = std::max_element( m_XData.begin() , m_XData.end() );
    QVector<double>::iterator xMinIt = std::min_element( m_XData.begin() , m_XData.end() );
    QVector<double>::iterator yMinIt = std::min_element( m_YData.begin() , m_YData.end() );
    QVector<double>::iterator yMaxIt = std::max_element( m_YData.begin() , m_YData.end() );

    //yMinIt = std::min_element( m_YData1.begin() , m_YData1.end() );
    //  yMaxIt = std::max_element( m_YData1.begin() , m_YData1.end() );

    qreal yPlotMin = *yMinIt;;
    // qreal yPlotMax = *yMaxIt;

    qreal xPlotMin = *xMinIt;
    qreal xPlotMax = *xMaxIt;

    m_CustomPlot->xAxis->setRange( xPlotMin , xPlotMax );
    m_CustomPlot->yAxis->setRange( yPlotMin - fmax(maxvel_fo,maxvel) -1 , fmax(maxvel_fo,maxvel) +1 );

    // m_CustomPlot->yAxis->setRange( yPlotMin - maxvel -10 , maxvel+10 );

    // Update the plot widget
    m_CustomPlot->replot();
}



void MainWindow::on_pushButton_setall_pressed()
{
    maxacc=ui->lineEdit_maxacc->text().toDouble();
    maxvel=ui->lineEdit_maxvel->text().toDouble();
    maxjer=ui->lineEdit_maxjer->text().toDouble();
}

void MainWindow::on_pushButton_pause_pressed()
{
    pause=1;
}

void MainWindow::on_pushButton_resume_pressed()
{
    pause=0;
}

void MainWindow::on_pushButton_run_pressed()
{
    run=1;
}

void MainWindow::on_pushButton_pause_timer_pressed()
{
    if(timer->isActive()){
        timer->stop();
        ui->checkBox_pause_timer->setChecked(1);
    } else {
        timer->start();
        ui->checkBox_pause_timer->setChecked(0);
    }
}

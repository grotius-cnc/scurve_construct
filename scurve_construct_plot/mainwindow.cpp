#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "iostream"

#include "../scurve_construct/scurve_construct.h"
scurve_data s;
double jermax=10;
double maxacc=10;
double maxvel=10;
double intval=0.01;
int enable=0;
double tarpos=0;
extern "C" struct scurve_data set_init_values_c(double jermax,
                                  double accmax,
                                  double maxvel,
                                  double cyctim,
                                  struct scurve_data data);
extern "C" struct scurve_data scurve_play_c(struct scurve_data data);
extern "C" struct scurve_data jog_velocity_c(struct scurve_data data, int enable, double tarpos);
extern "C" struct scurve_data jog_position_c(struct scurve_data data, int enable, double tarpos);

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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updatePlot()
{
    // Get the current time
    QTime tm = QTime::currentTime();
    // Convert the curretn time to millseconds
    qreal seconds = 60 * 60 * tm.hour() + 60 * tm.minute() + tm.second();
    qreal timeValue = 1000 * seconds + tm.msec();
    // Add the time the x data buffer
    m_XData.append( timeValue );

    s=set_init_values_c(jermax,maxacc,maxvel,intval,s);
    s=jog_position_c(s,enable,tarpos);
    s=scurve_play_c(s);

    // Write graph values.
    qreal value=s.guivel;
    m_YData.append( value );

    value=s.guiacc;
    m_YData1.append( value );

    value=s.guipos*0.1;
    m_YData2.append( value );

    ui->label_curvel->setText(QString::number(s.guivel,'f',15));
    ui->label_curacc->setText(QString::number(s.guiacc,'f',15));
    ui->label_curpos->setText(QString::number(s.guipos,'f',15));

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
    m_CustomPlot->yAxis->setRange( yPlotMin - s.maxvel -1 , s.maxvel+1 );

    // m_CustomPlot->yAxis->setRange( yPlotMin - maxvel -10 , maxvel+10 );

    // Update the plot widget
    m_CustomPlot->replot();
}

void MainWindow::on_pushButton_pause_pressed()
{
    if(timer->isActive()){
        timer->stop();
    } else {
        timer->start();
    }
}

void MainWindow::on_pushButton_jog_reverse_pressed()
{
    enable=1;
    tarpos=-10000;
}

void MainWindow::on_pushButton_jog_forward_pressed()
{
    enable=1;
    tarpos=10000;
}

void MainWindow::on_pushButton_jog_reverse_released()
{
    enable=0;
}

void MainWindow::on_pushButton_jog_forward_released()
{
    enable=0;
}


void MainWindow::on_pushButton_jog_pos_rev_pressed()
{
    enable=1;
    tarpos=ui->lineEdit_rev_tarpos->text().toDouble();
}

void MainWindow::on_pushButton_jog_pos_rev_released()
{
    enable=0;
}

void MainWindow::on_pushButton_jog_pos_fwd_pressed()
{
    enable=1;
    tarpos=ui->lineEdit_fwd_tarpos->text().toDouble();
}

void MainWindow::on_pushButton_jog_pos_fwd_released()
{
    enable=0;
}

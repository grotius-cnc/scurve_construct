#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    // This function is responsible for ploting
    // and updating the graphs , with each timer tick
    void updatePlot();



    void on_pushButton_setall_pressed();

    void on_pushButton_pause_pressed();

    void on_pushButton_resume_pressed();

    void on_pushButton_run_pressed();

    void on_pushButton_pause_timer_pressed();

private:
    Ui::MainWindow *ui;
    // Our custom plot widget
    QCustomPlot * m_CustomPlot;
    // This object will hold the current value as a text
    // that will appear at the extreme right of the plot,
    QCPItemText *m_ValueIndex;

    // The time between each update, this
    // will be  used by the timer to call "updatePlot" slot
    qreal timeInterval;

    // Data buffers
    QVector<qreal> m_YData;
    QVector<qreal> m_YData1;
    QVector<qreal> m_YData2;
    QVector<qreal> m_XData;

    QTimer *timer;
};

#endif // MAINWINDOW_H

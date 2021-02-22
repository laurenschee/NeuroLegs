#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QInputDialog>
#include <QStatusBar>
#include <QLabel>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QtNetwork>
#include <QAbstractSocket>
#include <QtCharts>
#include <QApplication>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QLegend>
#include <QSlider>
#include <QProcess>
#include <QIcon>
#include <QDateTime>
#include <QTimer>
#include <QQueue>
#include <QElapsedTimer>
#include <QDebug>
#include <QChartView>
#include <cmath>

#define TIMER 1
#define TIMERSCROLL 2
#define TIMERTIMEOUT 3

#define PTSHOWN ptShown

uint16_t  MapLinPwm(unsigned short valSens, int sens);

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow;}
QT_END_NAMESPACE

struct ConfigStimWhole {
    struct ConfigStim {
                int maxForce = 0 , minForce = 0, maxPw = 0, minPw = 0, sensor = 0;
                float current = 0;
    };

    ConfigStim red;  // for red, sensor is LATO_GAMBA
    ConfigStim blue;
    ConfigStim black;
    ConfigStim white;
};

#define PERIOD 1
#define SCROLLRATE 20
#define TIMEOUTUDP 10000


extern ConfigStimWhole *CONF, CONF_SVR;
extern bool lettaConf;
extern double cnt;
extern float kneeYaw;
extern float shankYaw;
extern float thighYaw;
extern QMutex mutex, mutexTimeout;
extern bool receivedUdp;
extern QQueue<QPointF> dataListRaw[7];
extern unsigned short pwShort[4];
extern QQueue<QPointF> dataStimRaw[4];
extern bool svrConnected;

extern bool activeSensor[7];

extern int RED_RAW_S;
extern int BLUE_RAW_S;
extern int BLACK_RAW_S;
extern int WHITE_RAW_S;

class UDPClass : public QObject
{
    Q_OBJECT

public:
    UDPClass();

    QUdpSocket *udpsocket;
    QUdpSocket *udpsocket_client;
    QString serverName;
    int serverPort;

    QElapsedTimer timerBench;
    QTimer *keepaliveTimer;

    #define MESSAGE_LENGTH 7
    double message_[MESSAGE_LENGTH];
    unsigned short messageShort[MESSAGE_LENGTH];

    char buffer[sizeof(struct ConfigStimWhole)];
    char buffer3[sizeof(short)*MESSAGE_LENGTH+sizeof(cnt)+sizeof(kneeYaw)+sizeof(shankYaw)+sizeof(thighYaw)];

public slots:
    void DisconnectUDP();
    void IniateUDP(QString serverName, int port, bool local);
    void SendKA();
    void LeggiDato();
    void udpSocketsClose();
    void SendConf();
    void send88();
    void emergency();
    void calibration();
    void closeLoop();
    void stopLoop();

signals:
    void startTimer(int tmr, int msec);
    void appendTextEdit(const QString &);
    void ResetSettingsConnection(void);
    void LoadConfSensors();    
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void IniateUDP(QString serverName, int port, bool local);
    void udpSocketsClose();
    void DisconnectUDP();
    void SendConf();
    void send88();
    void emergency();
    void calibration();
    void closeLoop();
    void stopLoop();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString serverName;
    int serverPort;

    QTimer *timerScroll;
    QTimer *timerTimeout;
    QTimer *timer;

    bool resetSC = 0;
    bool svrRun = 0;
    float sec = 1;
    bool firstDato = 1;
    int count_cnt = 1;
    int ptShown = 40;

    int oldposition = 1;

    bool right_leg = true;
    bool left_leg = false;

    UDPClass *udpEntity;
    QThread udpThread;
    QProcess *extSvr;

    QLineSeries *series0 = new QLineSeries();
    QLineSeries *series1 = new QLineSeries();
    QLineSeries *series2 = new QLineSeries();
    QLineSeries *series3 = new QLineSeries();
    QLineSeries *series4 = new QLineSeries();
    QLineSeries *series5 = new QLineSeries();
    QLineSeries *series6 = new QLineSeries();

    QLineSeries *sstim0 = new QLineSeries();
    QLineSeries *sstim1 = new QLineSeries();
    QLineSeries *sstim2 = new QLineSeries();
    QLineSeries *sstim3 = new QLineSeries();

    QLineSeries *sangle0 = new QLineSeries();
    QLineSeries *sangle1 = new QLineSeries();
    QLineSeries *sangle2 = new QLineSeries();

    QLineSeries *serieSep[7] = {new QLineSeries(), new QLineSeries(), new QLineSeries() ,new QLineSeries() ,new QLineSeries() ,new QLineSeries() ,new QLineSeries()};

    QChart *chart = new QChart();
    QChart *chartAngle = new QChart();
    QChart *chartstim = new QChart();
    QChart *chartSep[7] = {new QChart(), new QChart(),new QChart(),new QChart(),new QChart(),new QChart(),new QChart()};

    QValueAxis *axisX = new QValueAxis();
    QValueAxis *axisY = new QValueAxis();

    void appendTextEdit(QString text);
    void CleanPoints(QLineSeries *series, double min);
    void LoadConfSensors();

private slots:
    void ResetSettingsConnection();
    void startTimer(int tmr, int msec);
    void scrollUpdate();
    void updateTime();
    void CheckTimeout();
    void CleanShow();
    //void LockSettings();

    //void on_actionStart_triggered();
    //void on_actionStop_triggered();
    void on_actionConnect_triggered();
    void on_actionDisconnect_triggered();
    void on_actionQuit_triggered();
    void on_spinBox_valueChanged(int arg1);
    //void on_pushButton_clicked();
    //void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    //void on_horizontalScrollBar_sliderMoved(int position);

    //void on_checkBox_0_clicked();
    //void on_checkBox_1_clicked();
    //void on_checkBox_2_clicked();
    //void on_checkBox_3_clicked();
    //void on_checkBox_4_clicked();
    //void on_checkBox_5_clicked();
    //void on_checkBox_6_clicked();

    //void on_checkBox_clicked();
    //void on_checkBox_2_clicked();

    void on_checkBox_stateChanged(int arg1);
    void on_checkBox_2_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
};



#endif // MAINWINDOW_H


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "calibrationwindow.h"
#include "ui_calibrationwindow.h"

#define MAX_Y_NEWTON 300

ConfigStimWhole *CONF, CONF_SVR;
bool lettaConf = 0;
double cnt = 0;
float kneeYaw = 0;
float shankYaw = 0;
float thighYaw = 0;
bool receivedUdp = 0;
bool svrConnected = 0;
QQueue<QPointF> dataListRaw[7];
QQueue<QPointF> dataStimRaw[4];
unsigned short pwShort[4];
QMutex mutex, mutexTimeout;
bool acqB = 1;

bool activeSensor[7] = {1, 1, 1, 1, 1, 1, 1};

// PER STIMOLATION IN CHARTS
unsigned int SHORT_S_MAX_RED = 300;
unsigned int SHORT_S_MAX_BLUE = 300;
unsigned int SHORT_S_MAX_BLACK= 300;
unsigned int SHORT_S_MAX_WHITE= 300;

unsigned int SHORT_THRS_STIM_RED= 40;
unsigned int SHORT_THRS_STIM_BLUE= 40;
unsigned int SHORT_THRS_STIM_BLACK= 40;
unsigned int SHORT_THRS_STIM_WHITE= 40;

unsigned int PULSEWDITH_MAX_RED = 170;
unsigned int PULSEWDITH_MIN_RED= 100;

unsigned int PULSEWDITH_MAX_BLUE= 170;
unsigned int PULSEWDITH_MIN_BLUE= 100;

unsigned int PULSEWDITH_MAX_BLACK= 170;
unsigned int PULSEWDITH_MIN_BLACK= 100;

unsigned int PULSEWDITH_MAX_WHITE= 170;
unsigned int PULSEWDITH_MIN_WHITE= 100;

int RED_RAW_S = 1;
int BLUE_RAW_S = 2;
int BLACK_RAW_S = 3;
int WHITE_RAW_S = 4;

void MainWindow::startTimer(int tmr, int msec)
{
    switch (tmr) {
        case TIMER:
            timer->start(msec);
            break;
        case TIMERSCROLL:
            timerScroll->start(msec);
            break;
        case TIMERTIMEOUT:
            timerTimeout->start(msec);
            break;
    }
}

void MainWindow::appendTextEdit(QString text)
{
    ui->textEdit->appendPlainText(text);
}

void MainWindow::ResetSettingsConnection()
{
    emit udpSocketsClose();

    timerScroll->stop();
    timerTimeout->stop();
    timer->stop();

    resetSC = 1;

    svrConnected = 0;

    lettaConf = 0;
    svrRun = 0;

    sec = 0;
}

void MainWindow::CleanShow()
{
    int i;
    if (resetSC) {
        series0->clear();
        series1->clear();
        series2->clear();
        series3->clear();
        series4->clear();
        series5->clear();
        series6->clear();

        for (i = 0 ; i < 7 ; i++) {
            dataListRaw[i].clear();
        }
    }
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{

    udpEntity = new UDPClass();
    udpEntity->moveToThread(&udpThread);

    connect(udpEntity, &UDPClass::appendTextEdit, this, &MainWindow::appendTextEdit);
    connect(udpEntity, &UDPClass::ResetSettingsConnection, this, &MainWindow::ResetSettingsConnection);
    connect(udpEntity, &UDPClass::startTimer, this, &MainWindow::startTimer);
    connect(&udpThread, &QThread::finished, udpEntity, &QObject::deleteLater);

    connect(this, &MainWindow::DisconnectUDP, udpEntity, &UDPClass::DisconnectUDP);
    connect(this, &MainWindow::IniateUDP, udpEntity, &UDPClass::IniateUDP);
    connect(this, &MainWindow::udpSocketsClose, udpEntity, &UDPClass::udpSocketsClose);
    connect(this, &MainWindow::SendConf, udpEntity, &UDPClass::SendConf);
    connect(this, &MainWindow::send88, udpEntity, &UDPClass::send88);
    connect(this, &MainWindow::emergency, udpEntity, &UDPClass::emergency);
    connect(this, &MainWindow::calibration, udpEntity, &UDPClass::calibration);
    connect(this, &MainWindow::closeLoop, udpEntity, &UDPClass::closeLoop);
    connect(this, &MainWindow::stopLoop, udpEntity, &UDPClass::stopLoop);

    timerScroll = new QTimer(this);
    timerTimeout = new QTimer(this);
    timer = new QTimer(this);

    setWindowIcon(QIcon("qrc:/file/foot.ico"));

    udpThread.start();
    udpThread.setPriority(QThread::HighestPriority);

    ui->setupUi(this);
    //ui->horizontalScrollBar->setDisabled(1);



    series0->setName("S0");
    series1->setName("S1");
    series2->setName("S2");
    series3->setName("S3");
    series4->setName("S4");
    series5->setName("S5");
    series6->setName("S6");

    QPen defp = series0->pen();

    defp.setWidth(2);

    defp.setColor(QColor(0, 114, 189));
    series0->setPen(defp);

    defp.setColor(QColor(217, 83, 25));
    series1->setPen(defp);

    defp.setColor(QColor(237, 177, 32));
    series2->setPen(defp);

    defp.setColor(QColor(126, 47, 142));
    series3->setPen(defp);

    defp.setColor(QColor(119, 172, 48));
    series4->setPen(defp);

    defp.setColor(QColor(75, 219, 239));
    series5->setPen(defp);

    defp.setColor(QColor(174, 49, 73));
    series6->setPen(defp);

    chart->addSeries(series0);
    chart->addSeries(series1);
    chart->addSeries(series2);
    chart->addSeries(series3);
    chart->addSeries(series4);
    chart->addSeries(series5);
    chart->addSeries(series6);

    QFont ft= chart->legend()->font();
    ft.setPointSize(12);
    chart->legend()->setFont(ft);

    chart->setTitle("Sensors reading");
    chart->createDefaultAxes();

    chart->axisY()->setRange(0, 500);
    chart->axisY()->setTitleText("Newton");
    chart->axisX()->setTitleText("Sec");
    chart->axisX()->setRange(0*PERIOD, PTSHOWN*PERIOD);

    QFont ft2;
    ft2.setPointSize(12);
    chart->axisX()->setLabelsFont(ft2);
    chart->axisY()->setLabelsFont(ft2);

    // per ottimizzare:

    ui->graphicsView->setRenderHint(QPainter::Antialiasing, 0);
    ui->graphicsView->setRenderHint(QPainter::SmoothPixmapTransform, 0);
    ui->graphicsView->setRenderHint(QPainter::HighQualityAntialiasing, 0);
    ui->graphicsView->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing);
    ui->graphicsView->setChart(chart);

    ui->graphicsView->setInteractive(false);
    QSurfaceFormat::defaultFormat().setSwapInterval(0);

    sstim0->setName("Ch0");
    sstim1->setName("Ch1");
    sstim2->setName("Ch2");
    sstim3->setName("Ch3");

    defp.setWidth(2);
    defp.setColor(QColor(0, 114, 189));
    sstim0->setPen(defp);

    defp.setColor(QColor(217, 83, 25));
    sstim1->setPen(defp);

    defp.setColor(QColor(237, 177, 32));
    sstim2->setPen(defp);

    defp.setColor(QColor(126, 47, 142));
    sstim3->setPen(defp);

    QFont fts= chart->legend()->font();
    fts.setPointSize(12);
    chartstim->legend()->setFont(fts);

    chartstim->addSeries(sstim0);
    chartstim->addSeries(sstim1);
    chartstim->addSeries(sstim2);
    chartstim->addSeries(sstim3);

    chartstim->setTitle("Pulsewidth Modulation");
    chartstim->createDefaultAxes();

    chartstim->axisY()->setRange(0, 700);
    chartstim->axisY()->setTitleText("uS");
    chartstim->axisX()->setTitleText("Sec");
    chartstim->axisX()->setRange(0*PERIOD, PTSHOWN*PERIOD);

    chartstim->axisX()->setLabelsFont(ft2);
    chartstim->axisY()->setLabelsFont(ft2);

    // Assegno ai graphics view le chart, alle quali sono associate serie
    ui->graphicsView_2->setChart(chartstim);

    timer->setTimerType(Qt::PreciseTimer);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));

    timerScroll->setTimerType(Qt::CoarseTimer);
    connect(timerScroll, SIGNAL(timeout()), this, SLOT(scrollUpdate()));

    timerTimeout->setTimerType(Qt::VeryCoarseTimer);
    connect(timerTimeout, SIGNAL(timeout()), this, SLOT(CheckTimeout()));

    ui->textEdit->appendPlainText("Trying to connect..\n");

    /*ui->checkBox_0->setChecked(activeSensor[0]);
    ui->checkBox_1->setChecked(activeSensor[1]);
    ui->checkBox_2->setChecked(activeSensor[2]);
    ui->checkBox_3->setChecked(activeSensor[3]);
    ui->checkBox_4->setChecked(activeSensor[4]);
    ui->checkBox_5->setChecked(activeSensor[5]);
    ui->checkBox_6->setChecked(activeSensor[6]);

    serieSep[0]->setName("S0");
    serieSep[1]->setName("S1");
    serieSep[2]->setName("S2");
    serieSep[3]->setName("S3");
    serieSep[4]->setName("S4");
    serieSep[5]->setName("S5");
    serieSep[6]->setName("S6");

    for (int x = 0 ; x < 7 ; x++) {
        chartSep[x]->addSeries(serieSep[x]);

        chartSep[x]->createDefaultAxes();
        chartSep[x]->axisX()->setTitleText("Sec");
        chartSep[x]->axisX()->setRange(0*PERIOD, PTSHOWN*PERIOD);
        chartSep[x]->axisY()->setRange(0, MAX_Y_NEWTON);
        chartSep[x]->axisY()->setTitleText("Val");
    }

    ui->single_S0_chart->setRenderHint(QPainter::Antialiasing, 0);
    ui->single_S0_chart->setRenderHint(QPainter::SmoothPixmapTransform, 0);
    ui->single_S0_chart->setRenderHint(QPainter::HighQualityAntialiasing, 0);
    ui->single_S0_chart->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing);
    ui->single_S0_chart->setChart(chartSep[0]);
    ui->single_S0_chart->setInteractive(false);

    ui->single_S1_chart->setRenderHint(QPainter::Antialiasing, 0);
    ui->single_S1_chart->setRenderHint(QPainter::SmoothPixmapTransform, 0);
    ui->single_S1_chart->setRenderHint(QPainter::HighQualityAntialiasing, 0);
    ui->single_S1_chart->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing);
    ui->single_S1_chart->setChart(chartSep[1]);
    ui->single_S1_chart->setInteractive(false);

    ui->single_S2_chart->setRenderHint(QPainter::Antialiasing, 0);
    ui->single_S2_chart->setRenderHint(QPainter::SmoothPixmapTransform, 0);
    ui->single_S2_chart->setRenderHint(QPainter::HighQualityAntialiasing, 0);
    ui->single_S2_chart->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing);
    ui->single_S2_chart->setChart(chartSep[2]);
    ui->single_S2_chart->setInteractive(false);

    ui->single_S3_chart->setRenderHint(QPainter::Antialiasing, 0);
    ui->single_S3_chart->setRenderHint(QPainter::SmoothPixmapTransform, 0);
    ui->single_S3_chart->setRenderHint(QPainter::HighQualityAntialiasing, 0);
    ui->single_S3_chart->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing);
    ui->single_S3_chart->setChart(chartSep[3]);
    ui->single_S3_chart->setInteractive(false);

    ui->single_S4_chart->setRenderHint(QPainter::Antialiasing, 0);
    ui->single_S4_chart->setRenderHint(QPainter::SmoothPixmapTransform, 0);
    ui->single_S4_chart->setRenderHint(QPainter::HighQualityAntialiasing, 0);
    ui->single_S4_chart->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing);
    ui->single_S4_chart->setChart(chartSep[4]);
    ui->single_S4_chart->setInteractive(false);

    ui->single_S5_chart->setRenderHint(QPainter::Antialiasing, 0);
    ui->single_S5_chart->setRenderHint(QPainter::SmoothPixmapTransform, 0);
    ui->single_S5_chart->setRenderHint(QPainter::HighQualityAntialiasing, 0);
    ui->single_S5_chart->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing);
    ui->single_S5_chart->setChart(chartSep[5]);
    ui->single_S5_chart->setInteractive(false);

    ui->single_S6_chart->setRenderHint(QPainter::Antialiasing, 0);
    ui->single_S6_chart->setRenderHint(QPainter::SmoothPixmapTransform, 0);
    ui->single_S6_chart->setRenderHint(QPainter::HighQualityAntialiasing, 0);
    ui->single_S6_chart->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing);
    ui->single_S6_chart->setChart(chartSep[6]);
    ui->single_S6_chart->setInteractive(false);*/

    sangle0->setName("Knee");
    sangle1->setName("Shank");
    sangle2->setName("Thigh");

    defp.setWidth(2);

    defp.setColor(QColor(0, 114, 189));
    sangle0->setPen(defp);

    defp.setColor(QColor(217, 83, 25));
    sangle1->setPen(defp);

    defp.setColor(QColor(237, 177, 32));
    sangle2->setPen(defp);

    chartAngle->addSeries(sangle0);
    chartAngle->addSeries(sangle1);
    chartAngle->addSeries(sangle2);

    ft.setPointSize(12);
    chartAngle->legend()->setFont(ft);
    chartAngle->setTitle("Angle");
    chartAngle->createDefaultAxes();

    chartAngle->axisY()->setRange(0, 100);
    chartAngle->axisY()->setTitleText("degree");
    chartAngle->axisX()->setTitleText("Sec");
    chartAngle->axisX()->setRange(0*PERIOD, PTSHOWN*PERIOD);

    chartAngle->axisX()->setLabelsFont(ft2);
    chartAngle->axisY()->setLabelsFont(ft2);

    ui->graphicsView_3->setChart(chartAngle);

    ui->pushButton_4->setStyleSheet("background-color : red");

}

void MainWindow::scrollUpdate()
{
    //qDebug() << "Entered in scrollUpdate";
    emit LoadConfSensors();

    int count;

    if (dataListRaw->isEmpty())
        return;

    if (!mutex.tryLock())
        return;

    count = dataListRaw[0].count()-1;

    if (count == 0) {
        mutex.unlock();
        return;
    }

    if (series0->count() == 0) {
        firstDato = 1;
    }

    /*if (cnt > PTSHOWN){
        CleanPoints(series0, cnt-PTSHOWN);
        CleanPoints(series1, cnt-PTSHOWN);
        CleanPoints(series2, cnt-PTSHOWN);
        CleanPoints(series3, cnt-PTSHOWN);
        CleanPoints(series4, cnt-PTSHOWN);
        CleanPoints(series5, cnt-PTSHOWN);
        CleanPoints(series6, cnt-PTSHOWN);

        CleanPoints(sstim0, cnt-PTSHOWN);
        CleanPoints(sstim1, cnt-PTSHOWN);
        CleanPoints(sstim2, cnt-PTSHOWN);
        CleanPoints(sstim3, cnt-PTSHOWN);

        for (int x = 0 ; x < 7 ; x++)
            CleanPoints(serieSep[x], cnt-PTSHOWN);
    }*/

    if (cnt > (count_cnt * PTSHOWN)) {
        chart->axisX()->setRange((cnt)*PERIOD, (cnt+PTSHOWN)*PERIOD);
        chartstim->axisX()->setRange((cnt)*PERIOD, (cnt+PTSHOWN)*PERIOD);
        //for (int x = 0; x < 7 ; x++)
        //    chartSep[x]->axisX()->setRange((cnt)*PERIOD, (cnt+PTSHOWN)*PERIOD);
        chartAngle->axisX()->setRange((cnt)*PERIOD, (cnt+PTSHOWN)*PERIOD);
        count_cnt ++;
    }

    series0->append(dataListRaw[0]);
    series1->append(dataListRaw[1]);
    series2->append(dataListRaw[2]);
    series3->append(dataListRaw[3]);
    series4->append(dataListRaw[4]);
    series5->append(dataListRaw[5]);
    series6->append(dataListRaw[6]);

    //for (int x = 0 ; x < 7 ; x++)
    //    serieSep[x]->append(dataListRaw[x]);

    dataListRaw[0].clear();
    dataListRaw[1].clear();
    dataListRaw[2].clear();
    dataListRaw[3].clear();
    dataListRaw[4].clear();
    dataListRaw[5].clear();
    dataListRaw[6].clear();

    mutex.unlock();

    if (firstDato && (series0->count() > 0)) {
        firstDato = 0;

        auto minX = series0->at(0).x();

        chart->axisX()->setRange(minX, PTSHOWN);
        chartstim->axisX()->setRange(minX, PTSHOWN);
        chartAngle->axisX()->setRange(minX, PTSHOWN);
        //for (int x = 0; x < 7 ; x++)
        //    chartSep[x]->axisX()->setRange(minX, ptShown);

    }

    sstim0->append(cnt, pwShort[0]);
    sstim1->append(cnt, pwShort[1]);
    sstim2->append(cnt, pwShort[2]);
    sstim3->append(cnt, pwShort[3]);

    if(right_leg){
        sangle0->append(cnt, kneeYaw*180/3.14159265);
        sangle1->append(cnt, shankYaw*180/3.14159265);
        sangle2->append(cnt, thighYaw*180/3.14159265);
    }
    else{
        sangle0->append(cnt, -kneeYaw*180/3.14159265);
        sangle1->append(cnt, -shankYaw*180/3.14159265);
        sangle2->append(cnt, -thighYaw*180/3.14159265);
    }

}

void MainWindow::updateTime()
{
    sec = sec + 1;
}

MainWindow::~MainWindow()
{
    /*if(svrRun)
        extSvr->kill();

    emit DisconnectUDP();

    udpThread.quit();
    udpThread.wait();*/

    delete ui;
}

uint16_t MapLinPwm(unsigned short valSens, int sens)
{
    switch(sens) {
        case 0:
            if (valSens > SHORT_S_MAX_RED)
                return PULSEWDITH_MAX_RED;
            else if (valSens > SHORT_THRS_STIM_RED)
                return (PULSEWDITH_MIN_RED + ((float)valSens)/(SHORT_S_MAX_RED) * (PULSEWDITH_MAX_RED - PULSEWDITH_MIN_RED));
            else
                return 0;
            break;
        case 1:
            if (valSens > SHORT_S_MAX_BLUE)
                return PULSEWDITH_MAX_BLUE;
            else if (valSens > SHORT_THRS_STIM_BLUE)
                return (PULSEWDITH_MIN_BLUE + ((float)valSens)/(SHORT_S_MAX_BLUE) * (PULSEWDITH_MAX_BLUE - PULSEWDITH_MIN_BLUE));
            else
                return 0;
            break;
        case 2:
            if (valSens > SHORT_S_MAX_BLACK)
                return PULSEWDITH_MAX_BLACK;
            else if (valSens > SHORT_THRS_STIM_BLACK)
                return (PULSEWDITH_MIN_BLACK + ((float)valSens)/(SHORT_S_MAX_BLACK) * (PULSEWDITH_MAX_BLACK - PULSEWDITH_MIN_BLACK));
            else
                return 0;
            break;
        case 3:
            if (valSens > SHORT_S_MAX_WHITE)
                return PULSEWDITH_MAX_WHITE;
            else if (valSens > SHORT_THRS_STIM_WHITE)
                return (PULSEWDITH_MIN_WHITE + ((float)valSens)/(SHORT_S_MAX_WHITE) * (PULSEWDITH_MAX_WHITE - PULSEWDITH_MIN_WHITE));
            else
                return 0;
            break;
    }
    return 0;
}

void MainWindow::LoadConfSensors()
{
    SHORT_S_MAX_RED = ui->lineEdit_10->text().toInt();
    SHORT_S_MAX_BLUE = ui->lineEdit_12->text().toInt();
    SHORT_S_MAX_BLACK = ui->lineEdit_14->text().toInt();
    SHORT_S_MAX_WHITE = ui->lineEdit_16->text().toInt();

    SHORT_THRS_STIM_RED = ui->lineEdit_9->text().toInt();
    SHORT_THRS_STIM_BLUE = ui->lineEdit_11->text().toInt();
    SHORT_THRS_STIM_BLACK = ui->lineEdit_13->text().toInt();
    SHORT_THRS_STIM_WHITE = ui->lineEdit_15->text().toInt();

    PULSEWDITH_MIN_RED = ui->lineEdit->text().toInt();
    PULSEWDITH_MAX_RED = ui->lineEdit_2->text().toInt();

    PULSEWDITH_MIN_BLUE = ui->lineEdit_3->text().toInt();
    PULSEWDITH_MAX_BLUE = ui->lineEdit_4->text().toInt();

    PULSEWDITH_MIN_BLACK = ui->lineEdit_5->text().toInt();
    PULSEWDITH_MAX_BLACK = ui->lineEdit_6->text().toInt();

    PULSEWDITH_MIN_WHITE = ui->lineEdit_7->text().toInt();
    PULSEWDITH_MAX_WHITE = ui->lineEdit_8->text().toInt();

    RED_RAW_S = ui->lineEdit_17->text().toInt();
    BLUE_RAW_S = ui->lineEdit_18->text().toInt();
    BLACK_RAW_S = ui->lineEdit_19->text().toInt();
    WHITE_RAW_S = ui->lineEdit_20->text().toInt();
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_checkBox_stateChanged(int arg1)
{
    if(ui->checkBox->isChecked()){
        right_leg = true;
        left_leg = false;
        ui->checkBox_2->setCheckState(Qt::CheckState::Unchecked);
    }
}

void MainWindow::on_checkBox_2_stateChanged(int arg1)
{
    if(ui->checkBox_2->isChecked()){
        right_leg = false;
        left_leg = true;
        ui->checkBox->setCheckState(Qt::CheckState::Unchecked);
    }
}

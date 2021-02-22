#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "calibrationwindow.h"
#include "ui_calibrationwindow.h"

void MainWindow::on_actionConnect_triggered()
{
    bool ok;

    if (svrConnected == 1) {
        QMessageBox aboutBox;
        aboutBox.setText("Connection");
        aboutBox.setInformativeText("Already connected!");
        aboutBox.setIcon(QMessageBox::Warning);
        aboutBox.exec();

        return;
    }

    cnt = 0;
    chart->axisX()->setRange(0*PERIOD, PTSHOWN*PERIOD);

    CleanShow();

    serverName = QInputDialog::getText(this, tr("Insert Server"),
                                            tr("Server:"), QLineEdit::Normal,
                                            "10.42.0.1", &ok);
    if (!ok || serverName.isEmpty())
        return;

    serverPort = QInputDialog::getInt(this, tr("Insert Port"),
                                         tr("Port:"), 9666, 1, 100000, 1, &ok);
    if (!ok || serverPort == 0)
        return;

    emit IniateUDP(serverName, serverPort, 0);

    ui->textEdit->appendPlainText("Trying to connect...");
    //ui->horizontalScrollBar->setDisabled(1);
}


/*void MainWindow::on_actionStart_triggered()
{
    svrRun=1;
    ui->textEdit->appendPlainText("Trying to run locally the server..");
    extSvr = new QProcess(this);

    serverName = "127.0.0.1";
    serverPort = 9666;

    emit IniateUDP(serverName, serverPort, 1);
}*/

void MainWindow::on_actionDisconnect_triggered()
{
    //ui->horizontalScrollBar->setEnabled(1);
    emit DisconnectUDP();
}

/*void MainWindow::on_actionStop_triggered()
{
    if (svrRun) {
        extSvr->kill();
        ui->textEdit->appendPlainText("Local server stopped.");
        ResetSettingsConnection();
    }
}*/

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    ptShown = arg1;

    chart->axisX()->setRange(1*PERIOD, PTSHOWN*PERIOD);
    chartstim->axisX()->setRange(1*PERIOD, PTSHOWN*PERIOD);
}

void MainWindow::CleanPoints(QLineSeries *series, double min)
{
    long int i;
    for (i = series->count()-1 ; i > 0 ; i--) {
        if (series->at(i).x() < min) {
            series->removePoints(0, i+1);
            break;
        }
    }
}

/*void MainWindow::on_checkBox_0_clicked()
{
    activeSensor[0] = !activeSensor[0];
    if(activeSensor[0]) {
        series0->show();
    }
    else {
        series0->hide();

    }
}

void MainWindow::on_checkBox_1_clicked()
{
    activeSensor[1] = !activeSensor[1];
    if(activeSensor[1]) {
        series1->show();
    }
    else {
        series1->hide();

    }
}

void MainWindow::on_checkBox_2_clicked()
{
    activeSensor[2] = !activeSensor[2];
    if(activeSensor[2]) {
        series2->show();
    }
    else {
        series2->hide();

    }
}

void MainWindow::on_checkBox_3_clicked()
{
    activeSensor[3] = !activeSensor[3];
    if(activeSensor[3]) {
        series3->show();
    }
    else {
        series3->hide();

    }
}

void MainWindow::on_checkBox_4_clicked()
{
    activeSensor[4] = !activeSensor[4];
    if(activeSensor[4]) {
        series4->show();
    }
    else {
        series4->hide();

    }
}

void MainWindow::on_checkBox_5_clicked()
{
    activeSensor[5] = !activeSensor[5];
    if(activeSensor[5]) {
        series5->show();
    }
    else {
        series5->hide();

    }
}

void MainWindow::on_checkBox_6_clicked()
{
    activeSensor[6] = !activeSensor[6];
    if(activeSensor[6]) {
        series6->show();
    }
    else {
        series6->hide();

    }
}

void MainWindow::on_pushButton_clicked()
{
    QPixmap img;
    img = ui->graphicsView->grab();

    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save chart image"), "",
            tr("Data File (*.png);;All Files (*)"));
    if (fileName.isEmpty())
        return;
    // aggiungere controlli sul salvataggio.
    else {
        img.save(fileName);
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    qDebug() << "Entered in pushButton_2()";
    QPixmap img;
    img = ui->graphicsView_2->grab();

    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save chart image"), "",
            tr("Data File (*.png);;All Files (*)"));
    if (fileName.isEmpty())
        return;
    // aggiungere controlli sul salvataggio.
    else {
        img.save(fileName);
    }
}

void MainWindow::on_horizontalScrollBar_sliderMoved(int position)
{
    // Andrebbe messo il tipo di move che c'e' in leggidato,... pero' questo funziona..
    chart->scroll(-chart->plotArea().width() * (position - oldposition) / PTSHOWN,0);
    oldposition = position;
}*/

void MainWindow::on_pushButton_3_clicked()
{
    qDebug() << "Entered in pushButton_3()";
    emit send88();
}

void MainWindow::on_pushButton_4_clicked()
{
    qDebug() << "Entered in pushButton_4()";
    emit emergency();
}

void MainWindow::on_pushButton_5_clicked()
{
    qDebug() << "Entered in pushButton_5()";
    if(!svrConnected)
        emit appendTextEdit("Remember to connect!");
    else{
        emit calibration();
        calibrationwindow calibWindow;
        calibWindow.setModal(true);
        calibWindow.exec();
        calibWindow.hide();
    }

}

void MainWindow::on_pushButton_6_clicked()
{
    qDebug() << "Entered in pushButton_6()";
    if(!svrConnected)
        emit appendTextEdit("Remember to connect!");
    else{
        emit closeLoop();
    }
}

void MainWindow::on_pushButton_7_clicked()
{
    qDebug() << "Entered in pushButton_7()";
    if(!svrConnected)
        emit appendTextEdit("Remember to connect!");
    else{
        emit stopLoop();
    }
}


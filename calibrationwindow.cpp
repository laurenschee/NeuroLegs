#include "calibrationwindow.h"
#include "ui_calibrationwindow.h"

calibrationwindow::calibrationwindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::calibrationwindow)
{
    ui->setupUi(this);
}

calibrationwindow::~calibrationwindow()
{
    delete ui;
}

void calibrationwindow::on_pushButton_clicked()
{
    emit accept();
}

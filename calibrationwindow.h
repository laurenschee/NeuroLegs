#ifndef CALIBRATIONWINDOW_H
#define CALIBRATIONWINDOW_H

#include <QDialog>

namespace Ui {
class calibrationwindow;
}

class calibrationwindow : public QDialog
{
    Q_OBJECT

public:
    explicit calibrationwindow(QWidget *parent = nullptr);
    ~calibrationwindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::calibrationwindow *ui;
};

#endif // CALIBRATIONWINDOW_H

#ifndef MYDIALOG_H
#define MYDIALOG_H

#include <QDialog>

namespace Ui {
class MyDialog;
}

class MyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MyDialog(QWidget *parent = nullptr);
    ~MyDialog();

    QDateTime getStartTime();
    QDateTime getEndTime();
    void setStartTime();
    void setEndTime();


private slots:

private:  //私有成员 提供公共访问方法或属性来获取值
    Ui::MyDialog *ui;
};

#endif // MYDIALOG_H

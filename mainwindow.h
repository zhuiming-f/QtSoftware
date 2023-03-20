#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "tasklist.h"
#include<QtSql/QSqlDatabase>
#include <QMainWindow>
#include<QStandardItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //void focusOutEvent(QFocusEvent *event); //重载QWidget的聚焦函数
    //void pushData();
    void readData();

private slots:
    void on_processInfo_pushButton_clicked();

    void on_addTaskGroup_pushButton_clicked();

    //void on_addTask_pushButton_clicked();

    //void on_del_pushButton_clicked();

    void on_update_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    tasklist *task;

    QSqlDatabase db;
};
#endif // MAINWINDOW_H

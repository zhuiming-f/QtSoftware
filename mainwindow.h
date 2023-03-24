#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "tasklist.h"
#include "mydialog.h"
#include<QtSql/QSqlDatabase>
#include <QMainWindow>
#include<QStandardItem>
#include<QSet>
#include<QFile>
#include<QTextStream>
#include<QDate>
#include<QDebug>

#define debug_location() qDebug()<<"问题出在"<<__FILE__<<"文件的"<<Q_FUNC_INFO<<"函数的"<<__LINE__<<"行"

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
    void refreshTaskID();
    void refreshTaskGroup();
    void readData();
    //读取当日总任务数（包含被删除的任务）
    int readLogFile(const QString& filePath,QDate taskDate);

private slots:
    void on_processInfo_pushButton_clicked();
    void on_addTaskGroup_pushButton_clicked();

    void on_addTask_pushButton_clicked();
    void on_del_pushButton_clicked();
    void on_update_pushButton_clicked();

private:
    Ui::MainWindow *ui;

    tasklist *task;
    MyDialog *dialog;
    QStandardItemModel *model;
    QSet<int> usedIDs;
    QSqlDatabase db;
};
#endif // MAINWINDOW_H

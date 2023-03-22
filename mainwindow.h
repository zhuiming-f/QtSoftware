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

#define debug1() qDebug()<<"问题出在"<<__FILE__<<"文件的"<<Q_FUNC_INFO<<"函数的"<<__LINE__<<"行";


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
    int readLogFile(const QString& filePath,QDate taskDate)
    {
        QFile file(filePath);

        int res=1;

        if(file.open(QIODevice::ReadWrite|QIODevice::Text)){  //以文本模式打开
            QTextStream stream(&file);

            QStringList lines;
            bool flag=false;
            while(!stream.atEnd()){  //遍历找到与当前日期相等的日志条目
                QString line=stream.readLine();
                QStringList parts=line.split(" "); //以空格为分隔符把QString拆分为QStringList
                //parts.at(0)与parts[0]区别在于索引出错时 at(1)返回无效值  而[1]直接抛出一个越界异常
                QString str=parts.at(0);
                QDate date=QDate::fromString(str,"yyyy-MM-dd");
                int num=parts.at(1).toInt();

                //处理过期信息  这里可以等过期信息多点再重置
                if(QDate::currentDate()<=date&&date!=taskDate)
                {
                    lines.append(line);
                }

                if(date==taskDate){
                    flag=true;
                    res=num;
                    num++;
                    lines.append(QString("%1 %2").arg(date.toString("yyyy-MM-dd")).arg(QString::number(num)));
                }
            }

            //全部清空 写入未过期信息
            file.resize(0);
            foreach(const QString &line,lines){
                stream<<line<<Qt::endl;
            }

            if(!flag){
                stream<<QString("%1 2").arg(taskDate.toString("yyyy-MM-dd"));
            }

            file.close();
        }
        else{
            debug1();
        }
        return res;
    }

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
    std::vector<std::pair<QDateTime,int>> vec;
};
#endif // MAINWINDOW_H

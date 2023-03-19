#ifndef TASKLIST_H
#define TASKLIST_H

#include <QDialog>

namespace Ui {
class tasklist;
}

class tasklist : public QDialog
{
    Q_OBJECT

public:
    explicit tasklist(QDialog *parent = nullptr);
    ~tasklist();

private:
    Ui::tasklist *ui;

private slots:
    void on_refresh_pushButton_clicked();
    void slotProcessInfo(const QString&processName);

public:
    QTimer *m_timer;
};

#endif // TASKLIST_H

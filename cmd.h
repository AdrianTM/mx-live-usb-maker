#ifndef CMD_H
#define CMD_H

#include <QProcess>
#include <QString>

class Cmd: public QProcess
{
    Q_OBJECT
public:
    explicit Cmd(QObject *parent = nullptr);
    void halt();
    bool run(const QString &cmd, bool quiet = false);
    bool run(const QString &cmd, QByteArray& output, bool quiet = false);
    bool run(QProcess &proc, const QString &cmd, QByteArray& output, bool quiet = false);
    QString getCmdOut(const QString &cmd, bool quiet = false);
    QString getCmdOut(QProcess &proc, const QString &cmd, bool quiet = false);
};

#endif // CMD_H

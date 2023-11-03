
#ifndef CMD_H
#define CMD_H

#include <QProcess>

class QTextStream;

class Cmd : public QProcess
{
    Q_OBJECT
public:
    [[nodiscard]] QString getCmdOut(const QString &cmd, bool quiet = false);
    [[nodiscard]] QString getCmdOutAsRoot(const QString &cmd, bool quiet = false);
    bool run(const QString &cmd, QString *output, bool quiet = false);
    bool run(const QString &cmd, bool quiet = false);
    bool runAsRoot(const QString &cmd, QString *output, bool quiet = false);
    bool runAsRoot(const QString &cmd, bool quiet = false);
    explicit Cmd(QObject *parent = nullptr);

signals:
    void errorAvailable(const QString &err);
    void finished();
    void outputAvailable(const QString &out);

private:
    QString elevate;
    QString helper;
    QString out_buffer;
};

#endif // CMD_H

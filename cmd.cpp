#include "cmd.h"

#include <QApplication>
#include <QDebug>
#include <QEventLoop>
#include <QFileInfo>

Cmd::Cmd(QObject *parent)
    : QProcess(parent),
      elevate {"pkexec"},
      helper {"/usr/lib/" + QApplication::applicationName() + "/helper"}
{
    if (!QFileInfo::exists("/usr/bin/pkexec")) {
        elevate = "su-to-root -X -c";
    }
    connect(this, &Cmd::readyReadStandardOutput, [this] { emit outputAvailable(readAllStandardOutput()); });
    connect(this, &Cmd::readyReadStandardError, [this] { emit errorAvailable(readAllStandardError()); });
    connect(this, &Cmd::outputAvailable, [this](const QString &out) { out_buffer += out; });
    connect(this, &Cmd::errorAvailable, [this](const QString &out) { out_buffer += out; });
}

bool Cmd::run(const QString &cmd, bool quiet)
{
    QString output;
    return run(cmd, &output, quiet);
}

bool Cmd::runAsRoot(const QString &cmd, bool quiet)
{
    return run(elevate + " " + helper + " " + cmd, quiet);
}

bool Cmd::runAsRoot(const QString &cmd, QString *output, bool quiet)
{
    return run(elevate + " " + helper + " " + cmd, output, quiet);
}

QString Cmd::getCmdOut(const QString &cmd, bool quiet)
{
    QString output;
    run(cmd, &output, quiet);
    return output;
}

QString Cmd::getCmdOutAsRoot(const QString &cmd, bool quiet)
{
    return getCmdOut(elevate + " " + helper + " " + cmd, quiet);
}

bool Cmd::run(const QString &cmd, QString *output, bool quiet)
{
    out_buffer.clear();
    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Cmd::finished);
    if (this->state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << this->program() << this->arguments();
        return false;
    }
    if (!quiet) {
        qDebug().noquote() << cmd;
    }
    QEventLoop loop;
    connect(this, &Cmd::finished, &loop, &QEventLoop::quit);
    start(QStringLiteral("/bin/bash"), {QStringLiteral("-c"), cmd});
    loop.exec();
    *output = out_buffer.trimmed();
    return (exitStatus() == QProcess::NormalExit && exitCode() == 0);
}

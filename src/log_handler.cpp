/****************************************************************************
**
** Copyright (C) 2011 Lorem Ipsum Mediengesellschaft m.b.H.
**
** GNU General Public License
** This file may be used under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation and
** appearing in the file LICENSE.GPL included in the packaging of this file.
**
****************************************************************************/

#include "log_handler.h"

#include <QDateTime>
#include <QTextStream>
#include <QDir>

#include "log_info.h"
#include "config_file_handler.h"

//----------------------------------------------------------------------
LogHandler::LogHandler() : file_("log-"+QDateTime::currentDateTime().toString("MM-yyyy")+".log")
{
    QDir current(".");
    QStringList filters;
    filters << "*.log";
    log_list_ << current.entryList(filters);

    QString msg("========== ");
    msg.append(QDateTime::currentDateTime().toString("ddd dd.MM.yyyy hh:mm:ss"));
    msg.append(" - Session started ==========\n");
    writeFile(msg);
}

//----------------------------------------------------------------------
LogHandler::~LogHandler()
{
    QString msg("========== ");
    msg.append(QDateTime::currentDateTime().toString("ddd dd.MM.yyyy hh:mm:ss"));
    msg.append(" - Session closed ==========\n");
    writeFile(msg);
}

//----------------------------------------------------------------------
void LogHandler::writeFile(const QString &msg)
{
    lock_.lockForWrite();
    if (!file_.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(&file_);
    out << msg;

    file_.close();
    lock_.unlock();
}

//----------------------------------------------------------------------
QString LogHandler::createString(const LogInfo &info)
{
    QString msg;
    msg.append(info.time_.toString("hh:mm:ss") + " ");
    msg.append("[" + info.getStatusString() + "] " + info.domain_ +
               ": [" + QString::number(info.code_) + "] " + info.msg_ + "\n");
    return msg;
}

//----------------------------------------------------------------------
LogHandler &LogHandler::getInstance()
{
    static LogHandler instance;
    return instance;
}

//----------------------------------------------------------------------
void LogHandler::logData(const LogInfo &info)
{
    if (info.status_ >= ConfigFileHandler::getInstance().getLogLevel())
    {
        writeFile(createString(info));

        signalLogMessage(info);
    }
}

//----------------------------------------------------------------------
void LogHandler::logDataSlot(const LogInfo &info)
{
    logData(info);
}

//----------------------------------------------------------------------
void LogHandler::logFromJs(const LogInfo &info)
{
    if (info.status_ >= ConfigFileHandler::getInstance().getLogLevel())
    {
        writeFile(createString(info));
    }
}

//----------------------------------------------------------------------
void LogHandler::setLogLevel(const unsigned &log_level)
{
    ConfigFileHandler::getInstance().setLogLevel(log_level);
}

//----------------------------------------------------------------------
const QStringList &LogHandler::getLogFileList() const
{
    return log_list_;
}

//----------------------------------------------------------------------
QString LogHandler::getLogFileContent(const QString &file_name) const
{
    if (log_list_.contains(file_name))
    {
        QFile log_file(file_name);
        if (!log_file.open(QIODevice::ReadOnly | QIODevice::Text))
            return "";
        QTextStream in(&log_file);
        QString result;
        while (!in.atEnd()) {
            QString line = in.readLine();
            result.append(line+"\n");
        }
        return result;
    }
    return "";
}

//----------------------------------------------------------------------
void LogHandler::deleteLogFile(const QString &file_name)
{
    int idx = log_list_.indexOf(file_name);
    if (idx != -1)
    {
        log_list_.removeAt(idx);
        QFile::remove(file_name);
    }
}

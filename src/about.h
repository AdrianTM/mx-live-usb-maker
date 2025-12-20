#pragma once

class QString;
class QWidget;

void displayDoc(const QString &url, const QString &title);
void displayAboutMsgBox(const QString &title, const QString &message, const QString &licenceUrl,
                        const QString &licenseTitle, QWidget *parent = nullptr);

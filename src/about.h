#pragma once

class QString;
class QWidget;

void displayDoc(const QString &path, const QString &title, bool largeWindow = false);
void displayHelpDoc(const QString &path, const QString &title);
void displayAboutMsgBox(const QString &title, const QString &message, const QString &licence_url,
                        const QString &license_title, QWidget *parent = nullptr);

# **********************************************************************
# * Copyright (C) 2017 MX Authors
# *
# * Authors: Adrian
# *          MX Linux <http://mxlinux.org>
# *
# * This is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this package. If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CUSTOMPROGRAMNAME
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp

HEADERS  += \
    mainwindow.h

FORMS    += \
    mainwindow.ui

TRANSLATIONS += translations/mx-live-usb-maker_am.ts \
                translations/mx-live-usb-maker_ca.ts \
                translations/mx-live-usb-maker_cs.ts \
                translations/mx-live-usb-maker_da.ts \
                translations/mx-live-usb-maker_de.ts \
                translations/mx-live-usb-maker_el.ts \
                translations/mx-live-usb-maker_es.ts \
                translations/mx-live-usb-maker_fi.ts \
                translations/mx-live-usb-maker_fr.ts \
                translations/mx-live-usb-maker_hi.ts \
                translations/mx-live-usb-maker_hr.ts \
                translations/mx-live-usb-maker_hu.ts \
                translations/mx-live-usb-maker_it.ts \
                translations/mx-live-usb-maker_ja.ts \
                translations/mx-live-usb-maker_kk.ts \
                translations/mx-live-usb-maker_lt.ts \
                translations/mx-live-usb-maker_nl.ts \
                translations/mx-live-usb-maker_pl.ts \
                translations/mx-live-usb-maker_pt.ts \
                translations/mx-live-usb-maker_pt_BR.ts \
                translations/mx-live-usb-maker_ro.ts \
                translations/mx-live-usb-maker_ru.ts \
                translations/mx-live-usb-maker_sk.ts \
                translations/mx-live-usb-maker_sv.ts \
                translations/mx-live-usb-maker_tr.ts \
                translations/mx-live-usb-maker_uk.ts \
                translations/mx-live-usb-maker_zh_TW.ts

RESOURCES += \
    images.qrc



unix:!macx: LIBS += -lcmd

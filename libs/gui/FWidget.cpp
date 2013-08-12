//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
//
//   Filu is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 2 of the License, or
//   (at your option) any later version.
//
//   Filu is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Filu. If not, see <http://www.gnu.org/licenses/>.
//

#include <QApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "FWidget.h"

#include "FiluU.h"
#include "RcFile.h"

FWidget::FWidget(FClass* parent, const QString& className)
       : QWidget(0)
       , FClass(parent, className)
{}

FWidget::FWidget(FWidget* parent, const QString& className)
       : QWidget(parent)
       , FClass(parent, className)
{}

FWidget::~FWidget()
{}

FMainApp::FMainApp(const QString& connectionName, QApplication& app)
        : QMainWindow()
        , FClass(connectionName)
{
  if(hasConfigError())
  {
    setWindowTitle(QObject::tr("%1 - *** Config Error ***").arg(connectionName));
    QPushButton* closeButton = new QPushButton(QObject::tr("Close"));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QVBoxLayout* vbox = new QVBoxLayout;
    QLabel* label = new QLabel(formatMessages("%x"));
    vbox->setMargin(20);
    vbox->addWidget(label);
    vbox->addStretch(1);

    QHBoxLayout* hbox = new QHBoxLayout;
    QIcon* icon = new QIcon(QIcon::fromTheme("weather-storm-day"));
    label = new QLabel();
    label->setPixmap(icon->pixmap(QSize(128, 128)));
    hbox->addWidget(label);
    hbox->addLayout(vbox);
    hbox->addStretch(1);

    vbox = new QVBoxLayout;
    vbox->setMargin(10);
    vbox->addLayout(hbox);
    vbox->addWidget(closeButton);
    vbox->addStretch(1);

    hbox = new QHBoxLayout;
    hbox->addLayout(vbox);
    hbox->addStretch(1);

    QWidget* dummy = new QWidget;
    dummy->setLayout(hbox);
    setCentralWidget(dummy);
  }
}

FMainApp::~FMainApp()
{}

//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010  loh.tar@googlemail.com
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

#include "ConfigPage.h"

ConfigPage::ConfigPage(FClass* parent)
          : ManagerPage(parent)
{
//qDebug() << "ConfigPage::ConfigPage(FClass* parent) : ManagerPage(parent)";
  mIcon = QIcon(":/icons/configure.xpm");
  mIconText = tr("Configuration");
  createPage();
}

void ConfigPage::createPage()
{
  QGroupBox* configGroup = new QGroupBox(tr("Provider Configuration"));

  QListWidget* providerList = new QListWidget;
  QDir dir("/usr/local/lib/Filu/provider");
  int loop;
  for (loop = 2; loop < (int) dir.count(); loop++)
  {
    QString provider = dir[loop];
    //providerList->addItem(dir[loop]);
    QListWidgetItem* providerItem = new QListWidgetItem(providerList);
    if(isProviderInstalled(provider))
      providerItem->setIcon(QIcon(":/icons/ok.xpm"));
    else providerItem->setIcon(QIcon(":/icons/disable_gray.xpm"));
    providerItem->setText(dir[loop]);
    providerList->addItem(providerItem);
  }

  QPushButton* togleProviderBtn = new QPushButton;
  togleProviderBtn->setText(tr("Activate"));

  QHBoxLayout* serverLayout = new QHBoxLayout;
  serverLayout->addWidget(providerList);
  serverLayout->addWidget(togleProviderBtn);

  QVBoxLayout* configLayout = new QVBoxLayout;
  configLayout->addLayout(serverLayout);
  configGroup->setLayout(configLayout);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(configGroup);
  mainLayout->addStretch(1);
  setLayout(mainLayout);
}

bool ConfigPage::isProviderInstalled(const QString& provider)
{
  if(provider == "yahoo") return true;
  else return false;
}


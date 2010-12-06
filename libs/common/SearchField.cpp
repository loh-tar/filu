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

#include "SearchField.h"

SearchField::SearchField(QWidget* parent) : QWidget(parent)
{
  mButton = new QToolButton;
  mButton->setToolTip(tr("Clear Field"));
  mButton->setAutoRaise(true);
  mButton->setArrowType(Qt::RightArrow);
  connect(mButton, SIGNAL(clicked()), this, SLOT(clearField()));

  mField = new QLineEdit;
  connect(mField, SIGNAL(textChanged(const QString &)), this, SIGNAL(textChanged()));
  connect(mField, SIGNAL(textChanged(const QString &)), this, SIGNAL(newtext(const QString &)));
  connect(mField, SIGNAL(returnPressed()), this, SIGNAL(returnPressed()));

  QGridLayout* layout = new QGridLayout;
  layout->setMargin(0);
  layout->addWidget(mButton, 0, 0);
  layout->addWidget(mField, 0, 1);

  setLayout(layout);
}

SearchField::~SearchField()
{}

QString SearchField::text()
{
  return mField->text();
}

void SearchField::setText(const QString& txt)
{
  mField->setText(txt);
}

QRegExp SearchField::filter()
{
  QRegExp filter(mField->text(), Qt::CaseInsensitive);
  return filter;
}

void SearchField::clearField()
{
  mField->clear();
  mField->setFocus();
  emit textChanged();
}

void SearchField::setFocus()
{
  mField->setFocus();
}

//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011, 2012  loh.tar@googlemail.com
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

#include <QListWidget>
// #include <QTimer>
#include <QToolButton>

#include "SearchField.h"

SearchField::SearchField(QWidget* parent) : LineEdit(parent)
{
  doNotHideClearButton();

  mHistory = new QListWidget(parent);
  mHistory->setWindowFlags(Qt::FramelessWindowHint);
//   mHistory->setMinimumSize(QSize(0,0));
  mHistory->hide();
  connect(mHistory, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(historyChosen(QListWidgetItem*)));

  connect(this, SIGNAL(textChanged(const QString &)), this, SIGNAL(textChanged()));
  connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(updateHistory()));
}

SearchField::~SearchField()
{
  delete mHistory;
}

QRegExp SearchField::filter()
{
  QRegExp filter(text(), Qt::CaseInsensitive);
  return filter;
}

void SearchField::clearBtnClicked()
{
  mHistory->hide();
  QPoint p = pos();
  p.setX(p.x() + width() - mHistory->width());
  p.setY(p.y() + height());
  mHistory->move(p);
  mHistory->raise();


  clear();

  // FIXME Works not perfect. The given time is not always bided. Qt Bug?
//   QTimer::singleShot(9000, mHistory, SLOT(hide()));
  mHistory->show();
}

void SearchField::updateHistory()
{
  static QString lastInserted;

  QString txt = text();
  if(txt.isEmpty()) return;

  mHistory->hide();

  if(lastInserted == txt) return;

  if(lastInserted.size() - txt.size() == 1)
  {
    // Probably 'DEL' pressed
    if(mHistory->count() > 0) delete mHistory->takeItem(0);
  }
  else
  {
    txt.chop(1);
    if(lastInserted == txt)
    {
      // Simple any char added
      if(mHistory->count() > 0) delete mHistory->takeItem(0);
    }

    lastInserted = text();
  }

  // Don't list entrys twice
  QList<QListWidgetItem *> has = mHistory->findItems(lastInserted, Qt::MatchExactly);
  if(has.size() > 0)
  {
    delete has.at(0);
  }

  mHistory->insertItem(0, lastInserted);
  if(mHistory->count() > 7) delete mHistory->takeItem(7);

  int maxWidth = 0;
  for(int i = 0; i < mHistory->count(); ++i)
  {
    maxWidth = qMax(maxWidth, mHistory->fontMetrics().width(mHistory->item(i)->text()));
  }

  mHistory->resize(maxWidth + 20
                 , mHistory->fontMetrics().height() * mHistory->count() + 7);

}

void SearchField::historyChosen(QListWidgetItem* item)
{
  mHistory->hide();
  setText(item->text());
  mHistory->removeItemWidget(item);
  setFocus();
}

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

#include "LineEdit.h"

#include <QToolButton>
#include <QStyle>

LineEdit::LineEdit(QWidget *parent)
        : QLineEdit(parent)
{
  mClearBtn = new QToolButton(this);
  //QIcon icon = QIcon::fromTheme("edit-clear-locationbar-rtl");
  //mClearBtn->setIcon(icon);
  mClearBtn->setArrowType(Qt::LeftArrow);
  mClearBtn->setCursor(Qt::ArrowCursor);
  mClearBtn->setStyleSheet("QToolButton { border: none; padding: 0px; }");
  mClearBtn->setToolTip(tr("Clear Field"));
  mClearBtn->hide();

  connect(mClearBtn, SIGNAL(clicked()), this, SLOT(clearBtnClicked()));
  connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateClearBtn(const QString&)));

  int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

  setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(mClearBtn->sizeHint().width() + frameWidth + 1));

  QSize msz = minimumSizeHint();
  setMinimumSize( qMax(msz.width(), mClearBtn->sizeHint().height() + frameWidth * 2 + 2)
               , qMax(msz.height(), mClearBtn->sizeHint().height() + frameWidth * 2 + 2) );
}

LineEdit::~LineEdit()
{}

void LineEdit::resizeEvent(QResizeEvent *)
{
  QSize sz = mClearBtn->sizeHint();

  int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

  mClearBtn->move( rect().right() - frameWidth - sz.width()
                , (rect().bottom() + 1 - sz.height()) / 2 );
}

void LineEdit::updateClearBtn(const QString& text)
{
  mClearBtn->setVisible(!text.isEmpty());
}

void LineEdit::clearBtnClicked()
{
  clear();
}

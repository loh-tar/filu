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

#include "StrategyEditor.h"

StrategyEditor::StrategyEditor(QWidget* parent)
              : QSplitter(Qt::Vertical, parent)
{
  addWidget(&mRule);
  addWidget(&mIndi);

  mRule.setLineWrapMode(QTextEdit::NoWrap);
  mRule.setFontFamily("Monospace");
  mIndi.setLineWrapMode(QTextEdit::NoWrap);
  mIndi.setFontFamily("Monospace");

  connect(&mRule, SIGNAL(textChanged()), this, SIGNAL(edited()));
  connect(&mIndi, SIGNAL(textChanged()), this, SIGNAL(edited()));
}

StrategyEditor::~StrategyEditor()
{

}

void StrategyEditor::setReadOnly(bool ro)
{
  mRule.setReadOnly(ro);
  mIndi.setReadOnly(ro);
}

void StrategyEditor::toggleOrientation()
{
  if(orientation() == Qt::Vertical) setOrientation(Qt::Horizontal);
  else setOrientation(Qt::Vertical );
}

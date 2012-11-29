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

#include <QAction>
#include <QApplication>
#include <QMouseEvent>

#include "FToolBar.h"

#include "RcFile.h"

FToolBar::FToolBar(const QString& title, FClass* parent)
           : QToolBar(title)
           , FClass(parent, FUNC)
           , mDragedAct(0)
{
  setAcceptDrops(true);
}

void FToolBar::saveSettings()
{
  QList<QAction*> al = actions();
  QStringList actNames;
  foreach(QAction* act, al)
  {
    actNames.append(act->objectName());
  }
  mRcFile->set(objectName(), actNames.join(";"));
}

void FToolBar::loadSettings()
{
  QStringList actNames = mRcFile->getST(objectName()).split(";");
  QList<QAction*> newActOrder;
  QList<QAction*> al = actions();

  clear();

  foreach(QString actName, actNames)
  {
    QAction* act = 0;
    foreach(act, al)
    {
      if(act->objectName() != actName) continue;
      break;
    }

    if(act)
    {
      newActOrder.append(act);
      al.removeOne(act);
    }
  }

  addActions(newActOrder);

  foreach(QAction* act, al) addAction(act);
}

bool FToolBar::event(QEvent* event)
{
  if(QEvent::ChildAdded == event->type())
  {
    QObject* obj = static_cast<QChildEvent*>(event)->child();

    if(true == obj->inherits("QWidget"))
    {
      obj->installEventFilter(this);
    }
  }

  return QToolBar::event(event);
}

bool FToolBar::eventFilter(QObject* obj, QEvent* event)
{
  static QPoint dragStartPosition;

  if(event->type() == QEvent::MouseButtonPress)
  {
    if(static_cast<QMouseEvent*>(event)->modifiers() & Qt::ControlModifier)
    {
      dragStartPosition = static_cast<QMouseEvent*>(event)->pos();
      mDragedAct        = actionAt(static_cast<QWidget*>(obj)->mapTo(this, dragStartPosition));
      return true;
    }
  }

  if((event->type() == QEvent::MouseMove) and mDragedAct)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(event);
    if(me->buttons() & Qt::LeftButton)
    {
      int distance = (me->pos() - dragStartPosition).manhattanLength();

      if(distance > QApplication::startDragDistance()) startDrag();
      return true;
    }
  }

  // Standard event processing
  return QObject::eventFilter(obj, event);
}

void FToolBar::startDrag()
{
  QMimeData* mimeData = new QMimeData;
  mimeData->setData("application/Filu_FToolBar", "foo");
  QDrag* drag = new QDrag(this);
  drag->setMimeData(mimeData);
  if(mDragedAct) drag->setPixmap(QPixmap::grabWidget(widgetForAction(mDragedAct)));
  if(drag->exec(Qt::MoveAction))
  {
    //DoSomething()
  }
  else
  {
    //DoSomething()
  }

  mDragedAct = 0;
}

void FToolBar::dropEvent(QDropEvent* event)
{
  event->accept();
}

void FToolBar::dragEnterEvent(QDragEnterEvent* event)
{
  if(event->source() != this) return;

  event->accept();
}

void FToolBar::dragMoveEvent(QDragMoveEvent* event)
{
  QAction* act = actionAt(event->pos());

  if( (act == mDragedAct) or (act == 0) ) return;

  // Avoid flicker when move around
  QWidget* widget  = widgetForAction(act);
  QPoint widgetPos = widget->geometry().center();

  if(orientation() == Qt::Horizontal)
  {
    if(abs(widgetPos.x() - event->pos().x()) > 3) return;
  }
  else
  {
    if(abs(widgetPos.y() - event->pos().y()) > 3) return;
  }

  // Swap the actions
  QList<QAction*> al = actions();
  int iAct = al.indexOf(act);
  int iDragedAct = al.indexOf(mDragedAct);

  if( (iAct < 0) or (iDragedAct < 0) ) return;

  clear();
  al.swap(iAct, iDragedAct);
  addActions(al);
}

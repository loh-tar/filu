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

#include <QCheckBox>
#include <QDialog>
#include <QPainter>
#include <QTextEdit>

#include "COLine.h"

COLine::COLine(IndicatorPainter* painter)
      : COType(painter, FUNC)
{
  mClicksLeftTillNewIsPlaced = 2;

  setAttribute("Type", "Line");

  setAttribute("ExtendLeft",  false, tr("Extend the line left"));
  setAttribute("ExtendRight", false, tr("Extend the line right"));
  setAttribute("Horizontal",  false, tr("Justify horizontal"));
  setAttribute("WatchDog",    false, tr("Use as watch dog"));
  setAttribute("WatchRef",       "", tr("Watch Variable(s)"));

//   QRectF rec(0, 0, 10, 10);
//   rec.moveCenter(QPointF(0, 0));
//
//   QPainterPath obj;
//   obj.addEllipse(rec);
//
//   mObject = obj;
}

COLine::~COLine()
{}

bool COLine::isInvolved(const QPoint& pos)
{
  // Move our help line to the mouse position
  mHelp.translate(-mHelp.p1());
  mHelp.translate(-mHelp.pointAt(0.5));
  mHelp.translate(pos);

  QPointF dummy;
  int iType = mLine.intersect(mHelp, &dummy);
  if(iType == QLineF::BoundedIntersection) return true;

  return false;
}

// void COLine::paint(QPaintDevice* sheet)
// {
//
// }
/***********************************************************************
*
*   Protected Slots
*
************************************************************************/
void COLine::editWindowChanged()
{
  // Place in a derivated class here code wich validate the user input.
  // Most important to observe the checkboxes, if some, and (de)activate
  // in order other widgets, if needed.
  //
  // You can obtain the name of the edited widget/attribute with:
  //
  //   QObject* edited = sender();
  //   QString attribute = edited->objectName();
  //
  // And any other widget/attribute with:
  //
  //  QWidget* widget = mEditWindow->findChild<QWidget*>(attributeName);

  QObject* edited = sender();
  QString attribute;

  if(edited)
  {
    attribute = edited->objectName();
    //mEditWindow->setWindowModified(true);
  }

  if(attribute == "WatchDog" or !edited)
  {
    QCheckBox* cbox = mEditWindow->findChild<QCheckBox*>("WatchDog");
    QTextEdit* tedt = mEditWindow->findChild<QTextEdit*>("WatchRef");

    QString txt = tedt->toPlainText();

    tedt->setEnabled(cbox->isChecked());
    if(cbox->isChecked())
    {
      tedt->setEnabled(true);
      tedt->setTextColor(QColor("red"));
      if(txt.isEmpty()) tedt->setPlainText("CLOSE");
    }
    else
    {
      tedt->setEnabled(false);
      tedt->setTextColor(QColor("blue"));
      if(txt == "CLOSE") tedt->setPlainText("");

    }
  }
}

/***********************************************************************
*
*   Protected Functions
*
************************************************************************/
void COLine::prepare(const QHash<QString, QString>& keyValue)
{
  //qDebug() << "COLine::prepare(QHash<>)" << keyValue;
  COType::prepare(keyValue);

  // Here we have to set all spezial settings
  setAttribute("ExtendLeft", keyValue.value("ExtendLeft") == "true");
  setAttribute("ExtendRight", keyValue.value("ExtendRight") == "true");
  setAttribute("Horizontal", keyValue.value("Horizontal") == "true");
  setAttribute("WatchDog", keyValue.value("WatchDog") == "true");
  setAttribute("WatchRef", keyValue.value("WatchRef"));

  setAttribute("LeftDate", QDate::fromString(keyValue.value("LeftDate"), Qt::ISODate));
  setAttribute("LeftValue", keyValue.value("LeftValue").toDouble());

  setAttribute("RightDate", QDate::fromString(keyValue.value("RightDate"), Qt::ISODate));
  setAttribute("RightValue",keyValue.value("RightValue").toDouble());

  readAttributes(true);
}

void COLine::readAttributes(bool firstCall/* = false*/)
{
  // Here we have to read the attributes and set all needed things.
  // This function is also called after edit window was closed. Therefore
  // we have to take care for that case.

  // Create and set the anchor
  COType::readAttributes(firstCall);

  if(firstCall)
  {
    Grip* grip;
    grip = createNewGrip(Grip::eLeft);
    grip->set( mAttribute.value("LeftDate").toDate()
             , mAttribute.value("LeftValue").toDouble() );

    grip = createNewGrip(Grip::eRight);
    grip->set( mAttribute.value("RightDate").toDate()
             , mAttribute.value("RightValue").toDouble() );
  }

  if(mAttribute.value("Horizontal").value<bool>())
  {
    double anchorVal = mAnchor->value();

    foreach(Grip* grip, mGrip)
    {
      QDate date = grip->date();
      grip->set(date, anchorVal);
    }
  }
}

void COLine::writeAttributes()
{
  // The counterpart of readAttributes().
  // Called before the edit window appears and save() is started.
  COType::writeAttributes();

  setAttribute("LeftDate", mGrip.value(Grip::eLeft)->mDate);
  setAttribute("LeftValue", mGrip.value(Grip::eLeft)->mValue);

  setAttribute("RightDate", mGrip.value(Grip::eRight)->mDate);
  setAttribute("RightValue", mGrip.value(Grip::eRight)->mValue);
}

bool COLine::paintObject()
{
  if(!clipLine()) return false;

  // To do the job at isInvolved(),
  // we create a tiny perpendicular line to our line
  mHelp = mLine.normalVector();
  mHelp.setLength(3.0);

  QPen pen(mAttribute.value("Color").value<QColor>());
  pen.setStyle(Qt::SolidLine);

  QPainter painter(mSheet);
  painter.setPen(pen);
  painter.setBrush(QBrush(mAttribute.value("Color").value<QColor>()));
  painter.setRenderHints(QPainter::Antialiasing);
  painter.translate(mChartArea.bottomLeft());

  painter.drawLine(mLine);

  return true;
}

bool COLine::clipLine()
{
  // Here we create our line as they will be painted
  // and clip the line at the chart area borders

  QPointF p1 = mGrip.value(Grip::eLeft)->pos();
  QPointF p2 = mGrip.value(Grip::eRight)->pos();

  mLine = QLineF(p1, p2);

  QLineF  border;   // A chart area border
  QPointF p3;       // The intersection point
  int     iType;    // Intersection type

  bool leftOK        = false; // Indicates if no more investigation
  bool rightOK       = false; // is needed about left/right grip

  bool leftExtended  = mAttribute.value("ExtendLeft").value<bool>();
  bool rightExtended = mAttribute.value("ExtendRight").value<bool>();

  QRect area = mChartArea;
  area.moveBottom(-area.top());
  //qDebug() << "p1 in" << area.contains(p1) << "p2 in" << area.contains(p2);
  //qDebug() << "p1 in" << area.contains(p1.toPoint()) << "p2 in" << area.contains(p2.toPoint());
  // Check special case vertical line
  if(p1.x() == p2.x())
  {
    // We say "left" has to be to lower point
    if(-p1.y() > -p2.y())
    {
      p3 = p1;
      p1 = p2;
      p2 = p3;
    }

    if(leftExtended)
    {
      p1.setY(area.bottom());
      if(-p2.y() > -area.top()) p2.setY(area.top());
    }

    if(rightExtended)
    {
      p2.setY(area.top());
      if(-p1.y() < -area.bottom()) p1.setY(area.bottom());
    }

    mLine = QLineF(p1, p2);
    if(area.contains(mLine.pointAt(0.5).toPoint())) return true;
    else return false;
  }

  // Check left
  border = QLineF(area.bottomLeft(), area.topLeft());
  iType  = mLine.intersect(border, &p3);

  if(   (iType == QLineF::BoundedIntersection)
     or (leftExtended ) or p1.x() < area.left() )
  {
    p1 = p3;
  }

  if(    (-p3.y() < -area.topLeft().y())
     and (-p3.y() > -area.bottomLeft().y()) )
  {
    leftOK = true;
  }

  // Check right
  border = QLineF(area.bottomRight(), area.topRight());
  iType  = mLine.intersect(border, &p3);

  if(  (iType == QLineF::BoundedIntersection)
    or (rightExtended) or p2.x() > area.right() )
  {
    p2 = p3;
  }

  if(    (-p3.y() < -area.topRight().y())
     and (-p3.y() > -area.bottomRight().y()) )
  {
    rightOK = true;
  }


  if(leftOK and rightOK)
  {
    mLine = QLineF(p1, p2);
    if(area.contains(mLine.pointAt(0.5).toPoint())) return true;
    else return false;
  }

  // Check top
  border = QLineF(area.topLeft(), area.topRight());
  iType  = mLine.intersect(border, &p3);

  if((-p1.y() > -p2.y()) and !leftOK)
  {
    if((iType == QLineF::BoundedIntersection) or leftExtended or p1.x() < area.left())
    {
      p1 = p3;
    }

    leftOK = true;
  }

  if(leftOK and rightOK)
  {
    mLine = QLineF(p1, p2);
    if(area.contains(mLine.pointAt(0.5).toPoint())) return true;
    else return false;
  }

  if((-p1.y() < -p2.y()) and !rightOK)
  {
    if((iType == QLineF::BoundedIntersection) or rightExtended or p2.x() > area.right())
    {
      p2 = p3;
    }

    rightOK = true;
  }

  if(leftOK and rightOK)
  {
    mLine = QLineF(p1, p2);
    if(area.contains(mLine.pointAt(0.5).toPoint())) return true;
    else return false;
  }

  // Check bottom
  border = QLineF(area.bottomLeft(), area.bottomRight());
  iType  = mLine.intersect(border, &p3);

  if((-p1.y() < -p2.y()) and !leftOK)
  {
    if((iType == QLineF::BoundedIntersection) or leftExtended or p1.x() < area.left())
    {
      p1 = p3;
    }

    leftOK = true;
  }

  if(leftOK and rightOK)
  {
    mLine = QLineF(p1, p2);
    if(area.contains(mLine.pointAt(0.5).toPoint())) return true;
    else return false;
  }

  if((-p1.y() > -p2.y()) and !rightOK)
  {
    if((iType == QLineF::BoundedIntersection) or rightExtended or p2.x() > area.right())
    {
      p2 = p3;
    }

    rightOK = true;
  }

  mLine = QLineF(p1, p2);
  if(area.contains(mLine.pointAt(0.5).toPoint())) return true;
  else return false;

}

Grip* COLine::completeBrandNew(const QPoint& pos)
{
  --mClicksLeftTillNewIsPlaced;

  switch(mClicksLeftTillNewIsPlaced)
  {
    case 0:
    {
      Grip* leftGrip  = mGrip.value(Grip::eLeft);
      Grip* rightGrip = mGrip.value(Grip::eRight);

      mLine   = QLineF(leftGrip->pos(), rightGrip->pos());
      mAnchor = createNewGrip(Grip::eAnchor, mLine.pointAt(0.5));
      return 0;
    }

    case 1:
      mAnchor = createNewGrip(Grip::eLeft, pos);
      return createNewGrip(Grip::eRight, pos);

    default:
      return 0;
  }
}

void COLine::gripMoved( const QPoint& deltaPos
                      , const int deltaIdx
                      , const double deltaValue)
{
  // Place in a derivated class here code wich updates* all* things
  // are nessessary when the the grip mMovingGrip was moved.
  //
  // Currently reports mMovingGrip->pos() the old position
  // where as
  //   deltaPos   = newPosition - mMovingGrip->pos()
  //   deltaIdx   = newPos.mIdxPos - mMovingGrip->mIdxPos
  //   deltaValue = newPos.mValue - mMovingGrip->mValue

  mMovingGrip->move(deltaPos, deltaIdx, deltaValue);

  Grip* leftGrip  = mGrip.value(Grip::eLeft);
  Grip* rightGrip = mGrip.value(Grip::eRight);

  // Check for horizontal line
  if(mAttribute.value("Horizontal").value<bool>())
  {
    // Move all grip on the same new y position
    double newY = mMovingGrip->pos().y();

    foreach(Grip* grip, mGrip)
    {
      QPointF pos = grip->pos();
      pos.setY(newY);
      grip->set(pos);
    }

    if(rightGrip->mIdxPos == leftGrip->mIdxPos)
    {
      mMovingGrip->mIdxPos += deltaIdx;
      mMovingGrip->update();
    }
  }
  else
  {
    if(rightGrip->mIdxPos == leftGrip->mIdxPos)
    {
      // We have a vertical line. Avoid that they comes too tiny
      mLine = QLineF(leftGrip->pos(), rightGrip->pos());
      if(mLine.length() < 15) mMovingGrip->set(mMovingGrip->pos() - deltaPos);
    }
  }

  // Check if we have to rename left and right grip
  if(rightGrip->mIdxPos < leftGrip->mIdxPos)
  {
    renameGrip(leftGrip, Grip::eRight);
    renameGrip(rightGrip, Grip::eLeft);
  }

  if(mClicksLeftTillNewIsPlaced) return; // Anchor not yet available

  // Calculate the new anchor position in the middle of both
  mLine  = QLineF(leftGrip->pos(), rightGrip->pos());
  QPointF anchorPos = mLine.pointAt(0.5);

  mAnchor->set(anchorPos);
}

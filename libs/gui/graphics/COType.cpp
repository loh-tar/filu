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

#include <math.h>

#include "COType.h"

#include "COTuple.h"
#include "IndicatorPainter.h"
#include "Scaler.h"

// All available types
#include "COLine.h"

COType::COType(IndicatorPainter* painter)
      : FObject(painter)
      , mAnchor(0)
      , mStatus(eBrandNew)
      , mHover(false)
      , mP(painter)
{
  setAttribute("Id", -1);
  setAttribute("FiId",     mP->mBars->fiId());
  setAttribute("MarketId", mP->mBars->marketId());
  setAttribute("ViewName", mP->mViewName);
  setAttribute("AnchorValue", 0.0,     tr("Anchor Value"));
  setAttribute("AnchorDate",  QDate(), tr("Anchor Date"));
  setAttribute("Type", "");
  setAttribute("Color", QColor("White"), tr("Color"));
}

COType::~COType()
{
  foreach(Grip* grip, mGrip)
  {
    delete grip;
  }
}

/***********************************************************************
*
*   Static Functions
*
************************************************************************/
void COType::getAllTypes(QStringList& list)
{
  // Extend these list when you add new fanzy chart object types
  list << "Line";
  //list << "foo"
}

COType* COType::createNew(COTuple* co, IndicatorPainter* painter)
{
  COType* newCO = COType::createNew(co->type(), painter);

  if(newCO) newCO->prepare(co);

  return newCO;
}

COType* COType::createNew(const QString& type, IndicatorPainter* painter)
{
  COType* newCO = 0;

  // Extend these if/else if tree when you add new fanzy chart object types
  if(!type.compare("Line")) newCO = new COLine(painter);
  //else if(type.compare("xxx")) newCO = new COxxx(painter);
  else
  {
    // Type unknown
  }

  return newCO;
}

void  COType::strToAttributes(const QString& str, QHash<QString, QString>& attr)
{
  QStringList keyValueList = str.split('\n');

  foreach(QString keyValueStr, keyValueList)
  {
    int idx = keyValueStr.indexOf("=");
    QString name  = keyValueStr.left(idx);
    QString value = keyValueStr.remove(0, ++idx);

    value.replace("/n", QString('\n'));
    value.replace("/=", "=");
    value.replace("//", "/");

    attr.insert(name, value);
  }
}

/***********************************************************************
*
*   Other Puplic Functions
*
************************************************************************/
void COType::erase()
{
  int id = mAttribute.value("Id").toInt();

  if(id < 1) return; // Unvalid, not saved, no need to erase

  mFilu->deleteRecord(":user", "co", id);
}

void COType::setStatus(Status status)
{
  mStatus = status;
}

COType::Status COType::status()
{
  return mStatus;
}

bool COType::isInvolved(const QPoint& pos)
{
   qDebug() << "odden" << pos << mPaintedObject.containsPoint(pos, Qt::OddEvenFill);
   qDebug() << "windi" << pos << mPaintedObject.containsPoint(pos, Qt::WindingFill);
   qDebug() << "conti" << pos << mPaintedObject.contains(pos);

  bool involved = mPaintedObject.containsPoint(pos, Qt::OddEvenFill);

/*  if(!involved and mHover)
  {
    mHover = false;
    mP->mUpdateStaticSheet = true;
  }*/

  return involved;
}

void COType::showEditWindow()
{
  // Update mAttribute
  writeAttributes();

  // Build the window automaticly by parsing attributes
  QDialog window;
  mEditWindow = &window;        // To access from editWindowChanged()

  QGridLayout layout(&window);
  int       row = 0;            // Count layout rows
  QString   name;
  QWidget*  widget;
  QLabel*   label;
  QVariant  attribute;
  int       type;

  // Set the title bar of the dialog
  name = tr("Edit : ") + mAttribute.value("Type").value<QString>();
  window.setWindowTitle(name);

//   attribute = mAttribute.value("Type");
//   widget = new QLabel(attribute.value<QString>());
//
//   label  = new QLabel("Object type");
//   label->setAlignment(Qt::AlignRight);
//
//   layout.addWidget(label, row, 0);
//   layout.addWidget(widget, row++, 1);

  // For easier access after window is closed,
  // we save detected types and names
  QList<int>       types;
  QStringList      names;

  for(int i = 0; i < mAttrOrder.size(); ++i)
  {
    name      = mAttrOrder.at(i);
    widget    = 0;
    label     = 0;
    attribute = mAttribute.value(name);
    type      = attribute.type();

    //qDebug() << "COType::showEditWindow()" << name << type;

    switch(type)
    {
      case QVariant::Bool:
      {
        QCheckBox* cbox = new QCheckBox(mAttrTxt.at(i));
        cbox->setChecked(attribute.value<bool>());
        connect(cbox, SIGNAL(released()), this, SLOT(editWindowChanged()));
        widget = cbox;
        label  = new QLabel();
        break;
      }

      case QVariant::Color:
      {
        QLineEdit* ledt = new QLineEdit(attribute.value<QColor>().name());
        connect(ledt, SIGNAL(editingFinished ()), this, SLOT(editWindowChanged()));
        widget =  ledt;
        label  = new QLabel(mAttrTxt.at(i));
        break;
      }

      case QVariant::Date:
      {
        QDateEdit* dedt = new QDateEdit(attribute.value<QDate>());
        connect(dedt, SIGNAL(editingFinished ()), this, SLOT(editWindowChanged()));
        widget = dedt;
        label  = new QLabel(mAttrTxt.at(i));
        break;
      }

      case QVariant::DateTime:
        break;

      case QVariant::Double:
      {
        double val = attribute.value<double>();
        int decimals;
        double step;
        double valAbs = fabs(val);
        if(valAbs < 2)
        {
          decimals = 4;
          step = 0.01;
        }
        else if(valAbs < 100)
        {
          decimals = 2;
          step = 0.1;
        }
        else if(valAbs < 1000)
        {
          decimals = 2;
          step = 1;
        }
        else
        {
          decimals = 0;
          step = 100.0;
        }

        QDoubleSpinBox* sbox = new QDoubleSpinBox();
        valAbs *= 100;
        sbox->setRange(-valAbs, valAbs);
        sbox->setDecimals(decimals);
        sbox->setSingleStep(step);
        sbox->setValue(val);
        connect(sbox, SIGNAL(editingFinished ()), this, SLOT(editWindowChanged()));
        widget = sbox;
        label  = new QLabel(mAttrTxt.at(i));
        break;
      }

      case QVariant::Font:
        break;
      case QVariant::Icon:
        break;
      case QVariant::Int:
        break;
      case QVariant::Size:
        break;
      case QVariant::String:
      {
        QTextEdit* tedt = new QTextEdit(attribute.value<QString>());
        tedt->setMinimumHeight(tedt->minimumSizeHint().height() / 2);
        tedt->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        connect(tedt, SIGNAL(textChanged()), this, SLOT(editWindowChanged()));
        widget = tedt;
        label  = new QLabel(mAttrTxt.at(i));
        break;
      }

      case QVariant::Time:
        break;
      default :
        break;
    }

    if(!widget)
    {
     qDebug() << "COType::showEditWindow: Oops!, Attribute" << name
      << "has type" << attribute.typeName() << "but is not here supported";
      continue;
    }

    types.append(type);
    names.append(name);

    widget->setObjectName(name);
    label->setAlignment(Qt::AlignRight);

    layout.addWidget(label, row, 0);
    layout.addWidget(widget, row++, 1);
  }

  QDialogButtonBox okCancel(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(&okCancel, SIGNAL(accepted()), &window, SLOT(accept()));
  connect(&okCancel, SIGNAL(rejected()), &window, SLOT(reject()));

  layout.addWidget(&okCancel, row, 1);

  editWindowChanged();
  window.setWindowModified(false);

  if(!window.exec()) return;

  int i = 0;
  foreach(type, types)
  {
    name = names.at(i);

    switch(type)
    {
      case QVariant::Bool:
      {
        QCheckBox* cbox = mEditWindow->findChild<QCheckBox*>(name);
        setAttribute(name, cbox->isChecked());
        break;
      }

      case QVariant::Color:
      {
        QLineEdit* ledt = mEditWindow->findChild<QLineEdit*>(name);
        setAttribute(name, QColor(ledt->text()));
        break;
      }

      case QVariant::Date:
      {
        QDateEdit* dedt = mEditWindow->findChild<QDateEdit*>(name);
        setAttribute(name, dedt->date());
        break;
      }

      case QVariant::DateTime:
        break;

      case QVariant::Double:
      {
        QDoubleSpinBox* sbox = mEditWindow->findChild<QDoubleSpinBox*>(name);
        setAttribute(name, sbox->value());
        break;
      }

      case QVariant::Font:
        break;
      case QVariant::Icon:
        break;
      case QVariant::Int:
        break;
      case QVariant::Size:
        break;

      case QVariant::String:
      {
        QTextEdit* tedt = mEditWindow->findChild<QTextEdit*>(name);
        setAttribute(name, tedt->toPlainText());
        break;
      }

      case QVariant::Time:
        break;
      default :
        break;
    }

    ++i;
  }
  //qDebug() << mAttribute;
  readAttributes();
}

COType::Status COType::handleEvent(QEvent* event)
{
  // Before we go ahead check if it's a mouse event,
  // if yes examine pos, buttons and keys
  QPoint pos;
  int    btn;
  Qt::MouseButtons      btns;
  Qt::KeyboardModifiers mods;

  switch(event->type())
  {
    case QEvent::ContextMenu:
      pos  = static_cast<QContextMenuEvent*>(event)->pos();
      break;

    case QEvent::KeyPress:
      //case QEvent::KeyRelease:
      if(static_cast<QKeyEvent*>(event)->key() == Qt::Key_Delete)
      {
        erase();
        mStatus = eToKill;
        event->accept();
        return mStatus;
      }
      return mStatus;
      break;

    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
      pos  = static_cast<QMouseEvent*>(event)->pos();
      btn  = static_cast<QMouseEvent*>(event)->button();
      btns = static_cast<QMouseEvent*>(event)->buttons();
      mods = static_cast<QMouseEvent*>(event)->modifiers();
      break;

    default:
      return mStatus;
      break;
  }

  if(!mP->mChartArea.contains(pos))
  {
    // Mouse event, but not in the interesting area
    return mStatus;
  }

  event->accept();

  // Transform the position to match the needs
  pos.setY((mP->mChartArea.bottom() - pos.y()) * -1);

  // Examine the event in detail
  switch(event->type())
  {
    case QEvent::ContextMenu:
      event->setAccepted(rightButtonPressed(pos));
      break;

    case QEvent::MouseMove:
      mP->setMousePos(pos);
      event->setAccepted(mouseMoved(pos, btns, mods));
      break;

    case QEvent::MouseButtonPress:
      if(btn == Qt::LeftButton)
      {
        if(leftButtonPressed(pos))
        {
          //mP->parent()->setFocus(); // FIXME: does'nt work
        }
        else
        {
          event->setAccepted(false);
        }
      }
      else event->ignore();
      break;

    case QEvent::MouseButtonRelease:
      if(btn == Qt::LeftButton) event->setAccepted(leftButtonReleased(pos));
      else event->ignore();
      break;

    default :
      event->ignore();
  }

  //if(event->isAccepted()) qDebug() << "COType::handleEvent(accepted)" << mStatus;
  return mStatus;
}

void COType::paint(QPaintDevice* sheet)
{
  mSheet     = sheet;
  mChartArea = mP->mChartArea;
  mScaler    = mP->mScaler;

  foreach(Grip* grip, mGrip)
  {
    grip->update();
  }

  if(!paintObject()) return;

  if(mHover)
  {
    paintHoverInfo();
    mHover = false;
    mP->mCOInProcess = 0;
  }

  if(mStatus == eNormal) return;

  if( (mStatus == eSelected) or (mStatus == eMoving) or mClicksLeftTillNewIsPlaced)
  {
    paintBoundingBox();
  }
}

/***********************************************************************
*
*   Protected Slots
*
************************************************************************/
void COType::editWindowChanged()
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
  //
  // NOTE: This funtion is also called just before the edit window appears
  //       the first time. Than is sender() == 0. But you may also have do
  //       some changes in the edit window.
  //
  // For an example take a look at COLine.cpp

  if(!sender()) return;

  //mEditWindow->setWindowModified(true);

  //QWidget* widget = static_cast<QWidget*>(sender());
  //QString  name   = widget->objectName();
  //qDebug() << "COType::editWindowChanged()" << widget << name;
}

/***********************************************************************
*
*   Protected Functions
*
************************************************************************/
void COType::prepare(COTuple* co)
{
  mStatus = eNormal;
  mClicksLeftTillNewIsPlaced = 0;

  // Add all standard attributes each CO has.
  // Of course these are direct available in the CO data base...
  setAttribute("Id", co->id());
  // FiId, MarketId, ViewName are already set by ctor
  setAttribute("AnchorDate", co->anchorDate());
  setAttribute("Type", co->type());

  // ...and some included in the parameter field..so extract them
  QHash<QString, QString> keyValue;
  strToAttributes(co->attributes(), keyValue);

  //...and add these too
  prepare(keyValue);
}

void COType::prepare(const QHash<QString, QString>& keyValue)
{
  //qDebug() << "COType::prepare(QHash<>)";
  // Here we have to add all spezial attributes
  setAttribute("AnchorValue"
              , keyValue.value("AnchorValue").toDouble());

  setAttribute("Color"
              , QColor(keyValue.value("Color")));

  COType::readAttributes(true);
}

void COType::readAttributes(bool firstCall/* = false*/)
{
  // Here we have to read the attributes and set all needed things.
  // This function is also called after edit window was closed. Therefore
  // we have to take care for that case.

  // Create and set the anchor
  if(firstCall)
  {
    mAnchor = createNewGrip(Grip::eAnchor);
    mAnchor->set( mAttribute.value("AnchorDate").toDate()
                , mAttribute.value("AnchorValue").toDouble() );
  }
  else
  {
    // Aha, after edit. check if the anchor was moved
    Grip dummy(mP, Grip::eAnchor);
    dummy.set( mAttribute.value("AnchorDate").toDate()
             , mAttribute.value("AnchorValue").toDouble() );

    if(   (mAnchor->mDate  != dummy.mDate )
       or (mAnchor->mValue != dummy.mValue) )
    {
      // Yes, was moved. so move the object
      dummy.update();
      QPoint deltaPos   = dummy.pos() - mAnchor->pos();
      int    deltaIdx   = dummy.mIdxPos  - mAnchor->mIdxPos;
      double deltaValue = dummy.mValue - mAnchor->mValue;
      objectMoved(deltaPos, deltaIdx, deltaValue);
    }
  }
}

void COType::writeAttributes()
{
  // The counterpart of readAttributes().
  // Called before the edit window appears and save() is started.

  setAttribute("AnchorDate", mAnchor->mDate);
  setAttribute("AnchorValue", mAnchor->mValue);
}

bool COType::save()
{
  writeAttributes();

  // Don't add standard attributes to parameters
  QSet<QString> toSkip;
  toSkip << "Id" << "FiId" << "MarketId" << "ViewName"
         << "AnchorDate" << "Type";

  // But add them to a COTuple
  COTuple co(1);
  co.next();
  co.setId(mAttribute.value("Id").value<int>());
  co.setFiId(mAttribute.value("FiId").value<int>());
  co.setMarketId(mAttribute.value("MarketId").value<int>());
  co.setViewName(mAttribute.value("ViewName").value<QString>());
  co.setAnchorDate(mAttribute.value("AnchorDate").value<QDate>());
  co.setType(mAttribute.value("Type").value<QString>());

  QStringList paraList;
  QHashIterator<QString, QVariant> i(mAttribute);
  while (i.hasNext())
  {
    i.next();

    QString  name      = i.key();

    if(toSkip.contains(name)) continue;

    QString  value     = "";
    QVariant attribute = i.value();
    int      type      = attribute.type();

    switch(type)
    {
      case QVariant::Bool:
        if(attribute.toBool() == true) value = attribute.toString();
        break;

      case QVariant::Color:
        value = attribute.value<QColor>().name();
        break;

      case QVariant::Date:
        value = attribute.value<QDate>().toString(Qt::ISODate);
        break;

      case QVariant::DateTime:
        break;
      case QVariant::Double:
        value = attribute.toString();
        break;

      case QVariant::Font:
        break;
      case QVariant::Icon:
        break;
      case QVariant::Int:
        break;
      case QVariant::Size:
        break;
      case QVariant::String:
        value = attribute.toString();
        break;

      case QVariant::Time:
        break;
      default :
        break;
    }

    if(!value.isEmpty())
    {
      packParm(value);
      paraList.append(name + "=" + value);
    }
  }

  QString parameters = paraList.join("\n");

  co.setAttributes(parameters);

  // Write to DB
  mFilu->putCOs(co);
//   qDebug() << co.id();
//   qDebug() << co.fiId();
//   qDebug() << co.marketId();
//   qDebug() << co.viewName();
//   qDebug() << co.anchorDate();
//   qDebug() << parameters;

  // In case a brand new CO was first time saved fetch the ID
  setAttribute("Id", co.id());

  return false;
}

void COType::packParm(QString& parameter)
{
  parameter.replace("/", "//");
  parameter.replace("=", "/=");
  parameter.replace(QString('\n'), "/n");
}

bool COType::paintObject()
{
  //qDebug() << "COType::paintObject" << mP->mFirstBarToShow << mAnchor->mIdxPos << mAnchor->mValue << mAnchor->mDate;
  QPen pen(mAttribute.value("Color").value<QColor>());
  pen.setStyle(Qt::SolidLine);

  QPainter painter(mSheet);
  painter.setPen(pen);
  painter.setBrush(QBrush(mAttribute.value("Color").value<QColor>()));
  painter.setRenderHints(QPainter::Antialiasing);
  painter.translate(mChartArea.bottomLeft());

  QPointF anchor = mAnchor->pos();
  QMatrix matrix;
  matrix.translate(anchor.x(), anchor.y());
  mPaintedObject = mObject.toFillPolygon(matrix);

  painter.drawPolygon(mPaintedObject);

  return true;
}

void COType::paintHoverInfo()
{
 //qDebug() << "COType::paintHoverInfo:" << mAttribute.value("Type").toString();
}

void COType::paintBoundingBox()
{
  foreach(Grip* grip, mGrip)
  {
    grip->paint(mSheet, mChartArea.bottomLeft());
  }
}

Grip * COType::completeBrandNew(const QPoint& pos)
{
  // Normaly we should do here --mClicksLeftTillNewIsPlaced;
  // but because this is the base class we go on the save side
  mClicksLeftTillNewIsPlaced = 0;

  mAnchor = createNewGrip(Grip::eAnchor, pos);

  return 0;
}

void COType::gripMoved( const QPoint &/*deltaPos*/
                      , const int /*deltaIdx*/
                      , const double /*deltaValue*/)
{
  // Place in a derivated class here code wich updates* all* things
  // are nessessary when the the grip mMovingGrip was moved.
  //
  // Currently reports mMovingGrip->pos() the old position
  // where as
  //   deltaPos   = newPosition - mMovingGrip->pos()
  //   deltaIdx   = newPos.mIdxPos - mMovingGrip->mIdxPos
  //   deltaValue = newPos.mValue - mMovingGrip->mValue

  // Calling mMovingGrip->move(deltaIdx, deltaValue) may enough.
}

void COType::objectMoved( const QPoint &/*deltaPos*/
                        , const int deltaIdx
                        , const double deltaValue)
{
  // Place in a derivated class here code wich updates* all* things
  // are nessessary to move the object by a step of delta.
  // But I think there is no need to reimplement this function...hmm...
  //
  // Currently reports mAnchor->pos() the old position
  // where as
  //   deltaPos   = newPosition - mAnchor->pos()
  //   deltaIdx   = newPos.mIdxPos - mAnchor->mIdxPos
  //   deltaValue = newPos.mValue - mAnchor->mValue

  foreach(Grip* grip, mGrip)
  {
    grip->move(deltaIdx, deltaValue);
  }
}

void COType::setAttribute( const QString& name, const QVariant& value)
{
  mAttribute.insert(name, value);
}

void COType::setAttribute( const QString& name
                         , const QVariant& value
                         , const QString& userTxt, int pos/* = eAppend*/)
{
  int exist = mAttrOrder.indexOf(name) + 1;
  if(exist)
  {
    mAttrOrder.removeAt(exist - 1);
    mAttrTxt.removeAt(exist - 1);
  }

  if(pos > mAttrOrder.size()) pos = mAttrOrder.size();

  mAttrOrder.insert(pos, name);
  mAttrTxt.insert(pos, userTxt);

  mAttribute.insert(name, value);
}

Grip* COType::createNewGrip(Grip::Type type, const QPointF& pos/* = QPoint(-1, -1)*/)
{
  // Don't create a grip twice
  // Qt doku says if value not exist 0 is returned.
  Grip* existGrip = mGrip.value(type);

  if(existGrip) return existGrip;

  Grip* newGrip = new Grip(mP, type, pos);

  mGrip.insert(type, newGrip);

  return newGrip;
}

Grip* COType::renameGrip(Grip* grip, const Grip::Type newType)
{
  // Save probably existing grip.
  // Qt doku says if value not exist 0 is returned.
  Grip* existGrip = mGrip.value(newType);

  grip->mType = newType;
  mGrip.insert(newType, grip);

  return existGrip;
}

Grip* COType::gripInvolved(const QPoint& pos)
{
  foreach(Grip* grip, mGrip)
  {
    if(grip->contains(pos)) return grip;
  }

  return 0;
}

/***********************************************************************
*
*   Special Event Handler
*
************************************************************************/
bool COType::mouseMoved(const QPoint& pos
                       , const Qt::MouseButtons btns
                       , const Qt::KeyboardModifiers /*mod*/)
{
  switch(mStatus)
  {
    case eBrandNew:
    case eMoving:
    {
      // Fetch values of new position
      Grip newPos(mP, Grip::eRight, pos);
      newPos.set(pos);

      QPoint deltaPos   = pos - mMovingGrip->pos();
      int    deltaIdx   = newPos.mIdxPos - mMovingGrip->mIdxPos;
      double deltaValue = newPos.mValue - mMovingGrip->mValue;

      if(mMovingGrip == mAnchor) objectMoved(deltaPos, deltaIdx, deltaValue);
      else gripMoved(deltaPos, deltaIdx, deltaValue);

      return true;
    }

    case eNormal:
      if(!mP->mCOInProcess) // Only show hover when no other CO is in process
      {
        //qDebug() << "COType::mouseMoved:case eNormal(to hover)";
        mHover = true;
        mP->mCOInProcess = this;
      }
      return true;

    case eSelected:
      if(btns & Qt::LeftButton)
      {
       //qDebug() << "COType::mouseMoved:case eSelected:if(btns & Qt::LeftButton)";
        //mP->mUpdateStaticSheet = true;
        return true;
        //return false;
      }
      else return false;

    default:
      return false;
  }
}

bool COType::leftButtonPressed(const QPoint& pos)
{
  //qDebug() << "COType::leftButtonPressed()" << pos << mStatus;
  switch(mStatus)
  {
    case eBrandNew:
      mMovingGrip = completeBrandNew(pos);
      if(!mMovingGrip)
      {
        mStatus = eSelected;
      }
      return true;

    case eNormal:
      // No need to check isInvolved(pos),
      // we are here because we are involded
      mStatus = eSelected;
      return true;

    case eSelected:
      mMovingGrip = gripInvolved(pos);
      if(mMovingGrip)
      {
        mStatus = eMoving;
        mP->mUpdateStaticSheet = true;
      }
      else if(isInvolved(pos))
      {
        mMovingGrip = mAnchor;
        mStatus = eMoving;
        mP->mUpdateStaticSheet = true;
      }
      else
      {
        // Somewhere else but not the CO clicked
        mStatus = eNormal;
        save();
      }
      return true;

    case eMoving:
      mStatus = eSelected;
      return true;

    default:
      return false;
  }
}

bool COType::leftButtonReleased(const QPoint &/*pos*/)
{
  return false;
}

bool COType::rightButtonPressed(const QPoint& pos)
{
  switch(mStatus)
  {
    case eBrandNew:
      mStatus = eToKill;
      return true;

    case eNormal:
      // No need to check isInvolved(pos),
      // we are here because we are involded
      mStatus = eSelected;
      return true;

    case eSelected:
      if(isInvolved(pos) or gripInvolved(pos))
      {
        showEditWindow();
      }
      else
      {
        mStatus = eNormal;
        save();
      }
      return true;

    case eMoving:
      mStatus = eSelected;
      return true;

    default:
      return false;
  }
}

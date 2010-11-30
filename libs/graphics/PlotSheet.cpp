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

#include "PlotSheet.h"
#include "IndicatorPainter.h"
#include "Indicator.h"
#include "COType.h"

PlotSheet::PlotSheet(FWidget* parent)
         : FWidget(parent)
{
  init();
}

PlotSheet::PlotSheet(const QString& connectionName, QWidget* parent)
         : FWidget(connectionName, parent)
{
  init();
}

void PlotSheet::init()
{
  mMouseEvent.sender = this;
  mOldMouseXPos = -1;
  mBars   = 0;
  mMyBars = 0;

  setMouseTracking(true);
  setBackgroundRole(QPalette::Base);
  setAutoFillBackground(true);

  mPainter = new IndicatorPainter(this);
  mPainter->useSheet(this);

  //dont work here: mCOInProcess = 0;

  // Create the context menu actions
  mActShowGrid = new QAction("Show &Grid", this);
  mActShowGrid->setCheckable(true);
  mActShowGrid->setChecked(mPainter->mShowGrid);
  connect(mActShowGrid, SIGNAL(triggered(bool)), this, SLOT(showGrid(bool)));
  addAction(mActShowGrid);

  mActShowXScale = new QAction("Show &X-Scale", this);
  mActShowXScale->setCheckable(true);
  mActShowXScale->setChecked(mPainter->mShowXScale);
  connect(mActShowXScale, SIGNAL(triggered(bool)), this, SLOT(showXScale(bool)));
  addAction(mActShowXScale);

  mActShowYScale = new QAction("Show &Y-Scale", this);
  mActShowYScale->setCheckable(true);
  mActShowYScale->setChecked(mPainter->mShowYScale);
  connect(mActShowYScale, SIGNAL(triggered(bool)), this, SLOT(showYScale(bool)));
  addAction(mActShowYScale);
}

PlotSheet::~PlotSheet()
{
  if(mMyBars) delete mMyBars;
  delete mPainter;
}

QSize PlotSheet::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize PlotSheet::sizeHint() const
{
    return QSize(300, 200);
}

bool PlotSheet::useIndicator(const QString& file)
{
  clearErrors();

  if(mPainter->useIndicatorFile(file))
  {
    emit newIndicator(file);
    update();
    return true;
  }

  addErrorText(mPainter->errorText());
  update();
  return false;
}

void PlotSheet::showBarData(BarTuple* bars)
{
  mBars = bars;
  mPainter->useData(mBars);
  update();
}

void PlotSheet::showFiIdMarketId(int fiId, int marketId)
{
  // FIXME: only used by managerf, think remove there too
  if(mMyBars) delete mMyBars;
  mFilu->setFromDate(mFromDate.toString(Qt::ISODate));
  mFilu->setToDate(mToDate.toString(Qt::ISODate));
  mFilu->setFiId(fiId);
  mFilu->setMarketId(marketId);
  mMyBars = mFilu->getBars();

  showBarData(mMyBars);
}

void PlotSheet::setDateRange(const QDate& fromDate, const QDate& toDate)
{
  mFromDate = fromDate;
  mToDate   = toDate;
}

/******************************************************************
*
*                   private / protected functions
*
*******************************************************************/

void PlotSheet::paintEvent(QPaintEvent * /*event*/)
{
  mPainter->paint();
  addErrorText(mPainter->errorText());

  printError(); // only if some happend
  return;
}


void PlotSheet::densityChanged(int step)
{
  if(!mPainter->densityChanged(step)) return;

  mMouseEvent.type = mMouseEvent.type | eDensityChanged;
  mMouseEvent.density = mPainter->mDensity;
  mMouseEvent.place4Bars = mPainter->mPlace4Bars;
  mMouseEvent.firstBarToShow = mPainter->mFirstBarToShow;
  emit mouse(&mMouseEvent);

  update();
}

void PlotSheet::scrollChart(QPoint mousePos, int modifiers)
{
  int mouseXPos = mousePos.x();

  if(mouseXPos < 0)
  {
    mOldMouseXPos = -1;
    return;
  }

  if(mOldMouseXPos < 0)
  {
    mOldMouseXPos = mouseXPos;
    return;
  }

  int scrollCount = mOldMouseXPos - mouseXPos;

  if(modifiers and Qt::ControlModifier)
  {
    scrollCount *= 10; // "turbo" mode
  }

  if(mPainter->scrollHorizontal(scrollCount))
  {
    mOldMouseXPos = mouseXPos;

    mMouseEvent.type = mMouseEvent.type | eScrollChart;
    //mMouseEvent.type = mMouseEvent.type | eMouseMove;
    mMouseEvent.firstBarToShow = mPainter->mFirstBarToShow;
    mMouseEvent.mousePos = mousePos;
    emit mouse(&mMouseEvent);

    update();
  }
}

void PlotSheet::printError()
{
  if(!hasError()) return;

  QString error = errorText().join("\n");

  int x = size().width() / 2;
  int y = size().height() / 2;

  QFontMetrics fm(mPainter->mPlotFont);
  QPainter painter(this);
  painter.setFont(mPainter->mPlotFont);

  QRect rc = fm.boundingRect(0, 0, 1, 1, Qt::AlignLeft /*||FIXME: Qt::TextExpandTabs*/, error);
  rc.moveCenter(QPoint(x,y));
  painter.setPen(QColor(Qt::red));
  painter.drawText(rc, Qt::AlignLeft, error);

  clearErrors();
}

void PlotSheet::mouseSlot(MyMouseEvent* mme)
{
  if(mme->sender == this) return;

  if(mme->type & eMouseMove)
  {
    mPainter->setMousePos(mme->mousePos);
    update();
  }

  if(mme->type & eScrollChart)
  {
    mPainter->mFirstBarToShow = mme->firstBarToShow;
    mPainter->mUpdateStaticSheet = true;
    update();
  }

  if(mme->type & eDensityChanged)
  {
    mPainter->mDensity = mme->density;
    mPainter->mFirstBarToShow = mme->firstBarToShow;
    mPainter->mPlace4Bars = mme->place4Bars;
    mPainter->mUpdateStaticSheet = true;
    update();
  }

  if(mme->type & eShowYScale)
  {
    mPainter->mShowYScale = mme->place4Bars;  // place4Bars is abused
    mPainter->mUpdateStaticSheet = true;
    mActShowYScale->setChecked(mme->place4Bars);
    //update();
  }

  if(mme->type == eSyncReq)
  {
    mMouseEvent.type = eSyncDec;
    mMouseEvent.val1 = (int)mme->sender;  // Set val1 to the addressee which is the SyncRequester
    mMouseEvent.mousePos.setX(mPainter->mMouseXPos);
    mMouseEvent.density        = mPainter->mDensity;
    mMouseEvent.place4Bars     = mPainter->mPlace4Bars;
    mMouseEvent.firstBarToShow = mPainter->mFirstBarToShow;

    emit mouse(&mMouseEvent);
  }

  if((mme->type == eSyncDec) and (mme->val1.toInt() == (int)this))
  {
    mPainter->mMouseXPos      = mme->mousePos.x();
    mPainter->mDensity        = mme->density;
    mPainter->mPlace4Bars     = mme->place4Bars;
    mPainter->mFirstBarToShow = mme->firstBarToShow;

    if(mBars)
    {
      mBars->rewind(mme->firstBarToShow + mme->mousePos.x());
      mPainter->mMouseDate = mBars->date();
    }
  }
}
void PlotSheet::useFont(const QFont& font)
{
  mPainter->mPlotFont = font;
  mPainter->mUpdateStaticSheet = true;
}

void PlotSheet::useScaleColor(const QColor& color)
{
  mPainter->mScaleColor = color;
  mPainter->mUpdateStaticSheet = true;
}

void PlotSheet::useGridColor(const QColor& color)
{
  mPainter->mGridColor = color;
  mPainter->mUpdateStaticSheet = true;
}

void PlotSheet::useSheetColor(const QColor& color)
{
  mPainter->mSheetColor = color;
  mPainter->mUpdateStaticSheet = true;
}

void PlotSheet::setDensity(float density)
{
  mPainter->mDensity = density;
  mPainter->densityChanged(0);
}

void PlotSheet::setScaleToScreen(int minRange)
{
  // minRange is a percent value. when set to 0, the whole data set is used
  // and no adjustment will done. A minRange of 10 means that the
  // minLow and maxHigh of displayed range at least 10% differs
  mPainter->mScaleToScreen = minRange;
  mPainter->mUpdateStaticSheet = true;
}

void PlotSheet::showGrid(bool yes)
{
  mPainter->mShowGrid = yes;
  mPainter->mUpdateStaticSheet = true;
  mActShowGrid->setChecked(yes);
}

void PlotSheet::showXScale(bool yes)
{
  mPainter->mShowXScale = yes;
  mPainter->mUpdateStaticSheet = true;
  mActShowXScale->setChecked(yes);
}

void PlotSheet::showYScale(bool yes)
{
  mPainter->mShowYScale = yes;
  mPainter->mUpdateStaticSheet = true;
  mActShowYScale->setChecked(yes);

  mMouseEvent.type = mMouseEvent.type | eShowYScale;
  mMouseEvent.place4Bars = yes;             // place4Bars is abused
  emit mouse(&mMouseEvent);
}

/******************************************************************
*
*                        event handler
*
*******************************************************************/
bool PlotSheet::event(QEvent* event)
{
  event->accept();

  mMouseEvent.type = 0;

  if(event->type() == QEvent::MouseMove)
  {
    if(checkForCO(event)) return true;

    QMouseEvent* qme = (QMouseEvent*)event;
    QPoint pos = qme->pos();

    // transform the position to match the needs
    pos.setY((mPainter->mChartArea.bottom() - pos.y()) * -1);

    if(qme->buttons() & Qt::LeftButton)
    {
      //mPainter->setMousePos(pos);
      scrollChart(pos, qme->modifiers());

      return true;
    }

    if(mPainter->setMousePos(pos))
    {
      mMouseEvent.type = mMouseEvent.type | eMouseMove;
      mMouseEvent.mousePos = pos;
      mMouseEvent.mousePos.setY(1); // mark as unvalid, valid values are all negative!
      emit mouse(&mMouseEvent);
    }

    update();

    return true;
  }

  if(event->type() == QEvent::MouseButtonPress)
  {
    if(checkForCO(event)) return true;

    QMouseEvent* qme = (QMouseEvent*)event;

    if(qme->button() == Qt::LeftButton)
    {
      // check if a new chart object has to be placed
      if(!mNewCOType.isEmpty() and mBars and !mPainter->mCOInProcess)
      {
        mPainter->mCOInProcess = COType::createNew(mNewCOType, mPainter);

        if(!mPainter->mCOInProcess)
        {
          // you should never read this
         qDebug() << "PlotSheet::event: New chart object type" << mNewCOType << "unknown";
        }
        else
        {
          qDebug() <<"PlotSheet::event() new";
          checkForCO(event);
        }
      }

      mOldMouseXPos = -1;
      update();

      return true;
    }

    if(qme->button() == Qt::MidButton)
    {
      mPainter->mShowPercentScale = true;
      update();
    }
  }

  if(event->type() == QEvent::MouseButtonRelease)
  {
    QMouseEvent* qme = static_cast<QMouseEvent*>(event);

    if(qme->button() == Qt::MidButton)
    {
      mPainter->mShowPercentScale = false;
      //update();
    }
  }

  if(event->type() == QEvent::ContextMenu)
  {
    if(checkForCO(event)) return true;
  }

  if(event->type() == QEvent::Wheel)
  {
    QWheelEvent* qwe = (QWheelEvent*)event;
    densityChanged(qwe->delta() / 120);

    return true;
  }

  if(event->type() == QEvent::KeyPress)
  {
    if(checkForCO(event)) return true;
  }

  if(event->type() == QEvent::Resize)
  {
    mPainter->calcSizes();
    return true;
  }

  event->ignore();

  return QWidget::event(event);
}

bool PlotSheet::checkForCO(QEvent* event)
{
  // Returns true if event was taken by a CO. But do *not* mark event as ignored
  // if event was not used!

  // Well, we use the event anyway to test if event is used,
  // so take care that is later set back to accepted if it is not used.
  event->ignore();

  if(mPainter->mCOInProcess)
  {
    COType::Status status = mPainter->mCOInProcess->handleEvent(event);

    if(status == COType::eToKill)
    {
      // remove chart object from list
      mPainter->mCObjects.remove(mPainter->mCOInProcess);
      delete mPainter->mCOInProcess;
      mPainter->mCOInProcess = 0;
      mPainter->mUpdateStaticSheet = true;
    }
    else if(status == COType::eNormal)
    {
      mPainter->mCObjects.insert(mPainter->mCOInProcess);  // no need to check if already inside
      mPainter->mCOInProcess = 0;
      mPainter->mUpdateStaticSheet = true;
      mOldMouseXPos = -1;
    }

    if(event->isAccepted())
    {
      setFocus(); // FIXME: find a way to call from COType.cpp
      update();
      return true;
    }
  }

  if(   (event->type() == QEvent::MouseMove)
     or (event->type() == QEvent::MouseButtonPress)
     or (event->type() == QEvent::MouseButtonRelease) )
  {
    QPoint pos = static_cast<QMouseEvent*>(event)->pos();

    // transform the position to match the needs
    pos.setY((mPainter->mChartArea.bottom() - pos.y()) * -1);

    // find a chart object who ansers "yes, it's me"
    COType* co = 0;
    foreach(co, mPainter->mCObjects)
    {
      if(co->isInvolved(pos)) break;

      co = 0;
    }

    if(co)
    {
      //qDebug() << "PlotSheet::checkForCO() found co" << co;
      COType::Status status = co->handleEvent(event);

      if(status != COType::eNormal)
      {
        mPainter->mCOInProcess = co;
        mPainter->mUpdateStaticSheet = true;
      }
    }
  }

  if(event->isAccepted())
  {
    mOldMouseXPos = -1;
    update();
    return true;
  }
  else
  {
    // Here we are, set back to accepted
    event->accept();
    return false;
  }
}

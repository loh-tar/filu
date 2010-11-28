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

#include <float.h>
#include <math.h>

#include "IndicatorPainter.h"

#include "Indicator.h"
#include "PlotSheet.h"
#include "PlotType.h"
#include "Scaler.h"
#include "XScaleTicker.h"
#include "DataTupleSet.h"
#include "COType.h"

IndicatorPainter::IndicatorPainter(PlotSheet* parent)
                : FWidget(parent)
                , mIndicator(0)
                , mPlotType(0)
                , mPrepareError(false)
                , mCOInProcess(0)
                , mBars(0)
                , mData(0)
                , mSheet(0)
                , mStaticSheet(0)
                , mVolatileSheet(0)
                , mUpdateStaticSheet(true)
                , mScaler(0)
                , mDensity(0.0)
                , mPlace4Bars(0)
                , mFirstBarToShow(0)
                , mMouseXPos(0)
                , mMouseYValue(0.0)
                , mXSTicker(0)
                , mShowXScale(true)
                , mShowYScale(true)
                , mScaleToScreen(5)
                , mShowGrid(true)
                , mShowPercentScale(false)
{
  mPlotType  = new PlotType();
  mScaler    = new Scaler(this);
  mIndicator = new Indicator(this);
  mIndicator->ignorePlot(false);

  readSettings();
}

IndicatorPainter::~IndicatorPainter()
{
  // clean up mPlotCommands
  for(int i = 0; i < mPlotCommands.size(); i++)
  {
    delete mPlotCommands[i];
  }

  // delete old chart objects, if some
  COType* oldCO;
  foreach(oldCO, mCObjects)
  {
    delete oldCO;
  }

  if(mXSTicker)      delete mXSTicker;
  delete mPlotType;
  delete mScaler;
  delete mIndicator;
  if(mStaticSheet)   delete mStaticSheet;
  if(mVolatileSheet) delete mVolatileSheet;
}

bool IndicatorPainter::useIndicatorFile(const QString& file)
{
  clearErrors();

  // only call mPlotCommands.clear() is not enough
  for(int i = 0; i < mPlotCommands.size(); i++)
  {
    delete mPlotCommands[i];
  }
  mPlotCommands.clear();

  mPlotDataKeys.clear();
  mPrimaryValue.clear();

  QStringList* indiFile = mIndicator->useFile(file);

  if(!indiFile)
  {
    addErrorText(mIndicator->errorText());
    return false;
  }

  mViewName = mIndicator->viewName();

  if(!prepare(indiFile))  return false;
  if(!calculate()) return false;

  return true;
}

bool IndicatorPainter::prepare(QStringList* indiFile)
{
  QStringList plotCommands = indiFile->filter("PLOT(");
  plotCommands.replaceInStrings(" ","");

  if(plotCommands.isEmpty())
  {
    addErrorText("IndicatorPainter::prepare: No plot statements found");
    return false;
  }

  for(int i = 0; i < plotCommands.size(); i++)
  {
    QStringList command = plotCommands.at(i).split(",");
    command[0].remove("PLOT(");
    command[(command.size() - 1)].remove(")");

    PlotType* newPlotType = mPlotType->createNew(command.at(0));
    if(!newPlotType)
    {
      addErrorText(mPlotType->errorText());
      continue;
    }

    mPlotCommands.append(newPlotType);

    if(!newPlotType->prepare(command, mPlotDataKeys))
    {
      addErrorText(newPlotType->errorText());
      continue;
    }
  }

  QStringList primary = indiFile->filter("PRIMARY(");
  primary.replaceInStrings(QRegExp(".*\\(|\\).*"), "");
  primary.replaceInStrings(" ","");

  for(int i = 0; i < primary.size(); i++)
  {
    mPrimaryValue = primary.at(i);
  }

  if(mPrimaryValue.isEmpty())
  {
    // no PRIMARY set, create a default
    if(mPlotDataKeys.contains("CLOSE")) mPrimaryValue = "CLOSE";
    else if(mPlotDataKeys.contains("VOLUME")) mPrimaryValue = "VOLUME";
    else if(mPlotDataKeys.size() > 0) mPrimaryValue = mPlotDataKeys.at(0);
  }
  else if(!mPlotDataKeys.contains(mPrimaryValue))
  {
    addErrorText("IndicatorPainter::prepare: PRIMARY not found: " + mPrimaryValue);
  }

  if(hasError())
  {
    mPrepareError = true;
    return false;
  }

  mPrepareError = false;

  return true;
}

bool IndicatorPainter::useIndicator(QStringList &/*indicator*/)
{
  addErrorText("IndicatorPainter::useIndicator: Not yet implemented");
  return false;
  //FIXME:use an already parsed indicator,
  //could be useful while interactive design
}

void IndicatorPainter::useData(BarTuple* bars)
{
  mBars = bars;

  if(!bars) return;

  bool ok = calculate();

  // set to last data
  mMouseXPos = mPlace4Bars;
  mFirstBarToShow = mBars->count() - mPlace4Bars - 1;

  if(!ok) return;

  // set mMouseYValue to the last close...or something similar
  mData->rewind(mBars->count() - 1);
  mData->getValue(mPrimaryValue, mMouseYValue);
  mData->getDate(mMouseDate);
}

bool IndicatorPainter::calculate()
{
  // delete old chart objects, if some
  COType* oldCO;
  foreach(oldCO, mCObjects)
  {
    delete oldCO;
  }
  mCObjects.clear();
  mCOInProcess = 0;

  if(!mBars) return false;
  if(mPrepareError) return false;

  clearErrors();

  mData = mIndicator->calculate(mBars);

  calcSizes();

  if(!mData)
  {
    addErrorText(mIndicator->errorText());
    return false;
  }

  // load chart objects
  mFilu->setFiId(mBars->mFiId);
  mFilu->setMarketId(mBars->mMarketId);
  mBars->rewind(0);
  mFilu->setFromDate(mBars->date().toString(Qt::ISODate));
  mBars->rewind(mBars->count() - 1);
  mFilu->setToDate(mBars->date().toString(Qt::ISODate));

  COTuple* co = mFilu->getCOs();

  if(co)
  {
    while(co->next())
    {
      if(co->viewName() != mViewName) continue;

      COType* newCO = COType::createNew(co, this);
      mCObjects.insert(newCO);
    }

    delete co;
  }

  return true;
}

void IndicatorPainter::useSheet(QPaintDevice* sheet)
{
  mSheet = sheet;
}

void IndicatorPainter::calcSizes()
{
  mChartArea = QRect(QPoint(0, 0), QPoint(mSheet->width(), mSheet->height()));

  QFontMetrics fm(mPlotFont);

  if(mShowXScale)
  {
    mChartArea.setHeight(mSheet->height() - fm.height() - 10);
  }

  if(mShowYScale)
  {
    mChartArea.setWidth(mSheet->width()
                        - fm.boundingRect("99.999.99").width() - 8);
  }

  int lastBarShowing = mFirstBarToShow + mPlace4Bars;
  mPlace4Bars = (int)(mChartArea.width() / mDensity);
  mFirstBarToShow = lastBarShowing - mPlace4Bars;
  if(mFirstBarToShow < 0) mFirstBarToShow = 0;

  if(mStaticSheet)   delete mStaticSheet;
  mStaticSheet = new QPixmap(mSheet->width(), mSheet->height());

  if(mVolatileSheet) delete mVolatileSheet;
  mVolatileSheet = new QPixmap(mSheet->width(), mSheet->height());

  if(!mData)
  {
    mScaler->setHighLow(0, 0);
    return;
  }

  mData->setRange(mFirstBarToShow, mPlace4Bars);

  // find the highest and lowest value
  double minLow  =  1000000000;//DBL_MAX;
  double maxHigh = -1000000000;//DBL_MIN;
  for(int i = 0; i < mPlotDataKeys.size(); ++i)
  {
    double low  =  1000000000;//DBL_MAX;
    double high = -1000000000;//DBL_MIN;

    mData->getExtrema(mPlotDataKeys.at(i), low, high);

    if(low < minLow)   minLow  = low;
    if(high > maxHigh) maxHigh = high;
  }

  mScaler->setHighLow(maxHigh, minLow);

  mUpdateStaticSheet = true;

  // Try to keep the cursor at the last date position
  mMouseXPos = mBars->findDate(mMouseDate) - mFirstBarToShow;
  if(mMouseXPos < 1)
  {
    mMouseXPos = 1;
    mBars->rewind(mFirstBarToShow + 1);
    mMouseDate = mBars->date();
  }
  else if(mMouseXPos > mPlace4Bars)
  {
    mMouseXPos =  mPlace4Bars;
    mBars->rewind(mFirstBarToShow + mPlace4Bars);
    mMouseDate = mBars->date();
  }

}

bool IndicatorPainter::densityChanged(int step)
{
  if(!mBars) return false; // no data, nothing to recalc
  //if(hasError()) return false;

  int lastBarToShow = mFirstBarToShow + mPlace4Bars;
  if(lastBarToShow > mBars->count() - 1) lastBarToShow = mBars->count() - 1;

  // change the density by 10 percent
  if(step > 0) mDensity = mDensity * (1 + 0.1 * step);
  if(step < 0) mDensity = mDensity / (1 + 0.1 * -step);
  if(mDensity < 2 * FLT_EPSILON) mDensity = 2 * FLT_EPSILON;

  mPlace4Bars = (int)(mChartArea.width() / mDensity);
  mFirstBarToShow = lastBarToShow - mPlace4Bars;// - 1;

  if(mFirstBarToShow < 0) mFirstBarToShow = 0;

  if(mFirstBarToShow > mBars->count() - 1)
  {
    mFirstBarToShow = mBars->count() - 1;
    addErrorText("IndicatorPainter::densityChanged: ??? should never heappens", eCritical);
  }

  mUpdateStaticSheet = true;
  return true;
}

bool IndicatorPainter::scrollHorizontal(int step)
{
  if(!mBars) return false; // no data, nothing to scroll

  double adjustedStep = step / mDensity;

  if(abs(adjustedStep) < 1) return false;

  mFirstBarToShow += (int)round(adjustedStep);

  if(mFirstBarToShow + mPlace4Bars > mBars->count() - 1)
    mFirstBarToShow = mBars->count() - mPlace4Bars - 1;

  if(mFirstBarToShow < 0)
    mFirstBarToShow = 0;

  if(mFirstBarToShow > mBars->count() - 1)
    mFirstBarToShow = mBars->count() - 1;

  mUpdateStaticSheet = true;

  return true;
}

bool IndicatorPainter::setMousePos(const QPoint& pos)
{
  // set mMouseXPos
  // returns true if mMouseXPos has changed and false if not

  if(!mBars) return false;

  double value;
  int    newMouseXPos;
  QDate  newMouseDate;

  int valid = mScaler->pixelToValue(pos, newMouseXPos, value, newMouseDate);
  //qDebug() << "IndicatorPainter::setMousePos " << this << valid << pos <<  newMouseXPos << value;
  if(!(valid & Scaler::eXValid))
  {
    if(newMouseXPos > mPlace4Bars) newMouseXPos =  mPlace4Bars;
    else if(newMouseXPos > (mBars->count() - 1)) newMouseXPos = mBars->count() - 1;
    else if(newMouseXPos < 1) newMouseXPos = 1;
  }

  if(valid & Scaler::eYValid)
  {
    mMouseYValue = value;
  }
  else
  {
    if(mData)
    {
      mData->rewind(mFirstBarToShow + newMouseXPos);
      mData->getValue(mPrimaryValue, mMouseYValue);
    }
  }

  if(newMouseXPos == mMouseXPos) return false;

  mMouseXPos = newMouseXPos;
  mMouseDate = newMouseDate;

  return true;
}

void IndicatorPainter::setIndicatingRange(int from, int count)
{
  if(!mData) return;
  mData->setRange(from, count);
}

bool IndicatorPainter::paint()
{
  static const char* errTxt("IndicatorPainter::paint: No data to paint something");

  if(!mBars)
  {
    addErrorText(errTxt);
  }
  else
  {
    if(hasError())
    {
      removeErrorText(errTxt);
    }
  }

  if(hasError()) return false;

  if(mUpdateStaticSheet)
  {
    calcSizes();

    mUpdateStaticSheet = false;

    mStaticSheet->fill(mSheetColor);

    // paint the scales
    paintXScale();
    paintYScale();

    // paint the chart itself
    for(int i = 0; i < mPlotCommands.size(); i++)
    {
      mPlotCommands.at(i)->paint(mStaticSheet, mChartArea, mData, mScaler);
    }

    // paint the chart objects
    COType* co;
    foreach(co, mCObjects)
    {
      if(co->status() != COType::eNormal) continue;
      co->paint(mStaticSheet);
    }
  }

  // copy mStaticSheet into mVolatileSheet
  // I think this way is more effektive than simple mVolatileSheet = mStaticSheet
  // because then must be each time the whole memory new allocated (or I'm wrong?)
  QPainter painter;
  painter.begin(mVolatileSheet);
  painter.drawPixmap(0, 0, *mStaticSheet);
  painter.end();

  if(mCOInProcess)
  {
    mCOInProcess->paint(mVolatileSheet);
  }

  paintCrosshair();

  // paint the final chart onto the widget (or destination)
  painter.begin(mSheet);
  painter.drawPixmap(0, 0, *mVolatileSheet);

  return true;
}

void IndicatorPainter::paintXScale()
{
  if(!mShowXScale and !mShowGrid) return;
  if(!mXSTicker) mXSTicker = new XScaleTicker(this);

  QPen scalePen(mScaleColor);
  scalePen.setStyle(Qt::SolidLine);

  QPen gridPen(mGridColor);
  gridPen.setStyle(Qt::DotLine);

  QPainter painter(mStaticSheet);
  painter.setFont(mPlotFont);
  painter.setPen(scalePen);

  // calculate the vertical position of the scale
  painter.translate(mChartArea.bottomLeft());
  // 2 points more, to avoid overwriting the base line later
  painter.translate(0, 2);

  // draw the ticks in the scale with some descriptive text
  int x = 0;  // the x value in pixel
  int y = 0;  // the y value in pixel
  mXSTicker->prepare();
  while(mXSTicker->nextTick())
  {
    if(mXSTicker->paintYearTick(x, y)) painter.drawLine(x, 0, x, y);
    if(mXSTicker->paintQuarterTick(x, y)) painter.drawLine(x, 0, x, y);
    if(mXSTicker->paintMonthTick(x, y)) painter.drawLine(x, 0, x, y);
    if(mXSTicker->paintWeekTick(x, y)) painter.drawLine(x, 0, x, y);
    if(mXSTicker->paintDayTick(x, y)) painter.drawLine(x, 0, x, y);

    QString text;
    if(mXSTicker->paintTickText(text))
    {
      QRectF rect;
      rect = painter.boundingRect(rect, Qt::AlignCenter, text);
      rect.moveCenter(QPointF(x, 0));
      rect.moveBottom(rect.height() + 8);

      // clear a piece left from the text
//       rect.moveLeft(rect.left() - 3);
//       painter.setPen(mSheetColor);
//       painter.setBrush(QBrush(mSheetColor));
//       painter.drawRect(rect);
//       rect.moveLeft(rect.left() + 3);
//       painter.setPen(scalePen);

      painter.drawText(rect, Qt::AlignCenter, text);
      mXSTicker->setLastTextRightEdge((int)rect.right());
    }

    if(mXSTicker->paintGrid(x, y))
    {
      painter.setPen(gridPen);
      painter.drawLine(x, 0, x, y);
      painter.setPen(scalePen);
    }
  }

  // and draw the base line over the whole width
  // ...don't forget the 2 points more from the Y-Scale
  if(mShowXScale) painter.drawLine(0, 0, mChartArea.right() + 2, 0);

}

void IndicatorPainter::paintYScale()
{
  if(!mShowYScale and !mShowGrid) return;

  QPen scalePen(mScaleColor);
  scalePen.setStyle(Qt::SolidLine);

  QPen gridPen(mGridColor);
  gridPen.setStyle(Qt::DotLine);

  QPainter painter(mStaticSheet);
  painter.setFont(mPlotFont);
  painter.setPen(scalePen);

  // calculate the vertical position of the scale...
  painter.translate(mChartArea.bottomRight());
  // 2 points more, to avoid overwriting the base line later
  painter.translate(2, 0);

  // draw the ticks in the scale with some descriptive text
  if(!mScaler->beginYTicking())
  {
    // something wrong with data to plot a y scale
    // but paint anyway the base line
    if(mShowYScale) painter.drawLine(0, 2, 0, mScaler->topEdge());
    return;
  }

  QString text;
  int y;
  while(mScaler->nextYTick(y, text))
  {
    if(mShowYScale)
    {
      painter.drawLine(0, y, 4, y);

      QRectF rect;
      rect = painter.boundingRect(rect, Qt::AlignLeft, text);
      rect.moveCenter(QPoint(0, y));
      rect.moveLeft(0 + 8);

      painter.drawText(rect, Qt::AlignLeft, text);
    }

    if(mShowGrid)
    {
      painter.setPen(gridPen);
      painter.drawLine(0, y, -mChartArea.right(), y);
      painter.setPen(scalePen);
    }
  }

  // Draw the base line
  // ...don't forget the 2 points more from the X-Scale
  if(mShowYScale) painter.drawLine(0, 2, 0, mScaler->topEdge());

}

void IndicatorPainter::paintCrosshair()
{
  //if(!mShowXScale and !mShowYScale) return;

  QColor color("yellow");
  QPen crossPen(color);
  crossPen.setStyle(Qt::SolidLine);

  QPainter painter(mVolatileSheet);
  painter.setFont(mPlotFont);
  painter.setPen(crossPen);
  painter.setBrush(QBrush(mSheetColor));
  painter.setBackground(QBrush(mSheetColor));

  //
  // Paint the cursor into the chart
  double value;
  QPoint p;
  mData->rewind(mFirstBarToShow + mMouseXPos);
  mData->getValue(mPrimaryValue, value);
  mScaler->valueToPixel(mMouseXPos, value, p);
  painter.translate(mChartArea.bottomLeft());
  // Tiny line as cursor
  painter.drawLine(p.x() - 1, p.y(), p.x() +1, p.y());

  QString text;
  QRectF rect, box;
  double boxOverSizeX = 8.0;

  //
  // paint x-scale hair
  if(mShowXScale)
  {
    // calculate the vertical position of the scale...
    painter.resetTransform();
    painter.translate(mChartArea.bottomLeft());
    // ...and the 2 points more
    painter.translate(0, 2);

    QPoint p;
    mScaler->valueToPixel(mMouseXPos, mMouseYValue, p);

    // cross hair type, simple arrow
    painter.drawLine(p.x(), 0, p.x() - 4, 4);
    painter.drawLine(p.x(), 0, p.x() + 4, 4);

    // cross hair type, little tick
    //painter.drawLine(p.x(), 0, p.x(), 4);

    text = mMouseDate.toString(Qt::SystemLocaleDate);

    rect = painter.boundingRect(rect, Qt::AlignCenter, text);
    box  = rect;

    // paint a box around the text...
    box.adjust(0, 0, boxOverSizeX, 1);
    box.moveCenter(QPointF(p.x(), 0));
    if(box.left() < 0.0) box.moveLeft(0.0);
    if(box.right() > mChartArea.right()) box.moveRight(mChartArea.right());
    box.moveBottom(box.height() + 7);
    //painter.drawRect(box); // ...or not
    painter.eraseRect(box);

    // paint the text
    rect.moveCenter(QPointF(p.x(), 0));
    if(rect.left() < (boxOverSizeX / 2)) rect.moveLeft(boxOverSizeX / 2);
    if(rect.right() > mChartArea.right() - (boxOverSizeX / 2)) rect.moveRight(mChartArea.right() - (boxOverSizeX / 2));
    rect.moveBottom(rect.height() + 8);
    painter.drawText(rect, Qt::AlignCenter, text);
  }

  //
  // paint y-scale hair
  if(mShowYScale)
  {
    painter.resetTransform();
    painter.translate(mChartArea.bottomRight());
    // ...and the 2 points more
    painter.translate(2, 0);

    mScaler->beginYPercentTicking();

    bool paintBox = mShowPercentScale;
    int y;
    while(mScaler->nextYPercentTick(y, text))
    {
      painter.drawLine(0, y, 4, y);

      rect = painter.boundingRect(rect, Qt::AlignLeft, text);
      box  = rect;

      // once more, paint a box...
      box.adjust(0, 0, boxOverSizeX, 1);
      box.moveCenter(QPoint(0, y));
      box.moveLeft(-3 + 8);

      if(box.top() < -mChartArea.height() + 1) box.moveTop(-mChartArea.height() + 1);
      if(box.bottom() > -1) box.moveBottom(-1);

      if(paintBox)
      {
        paintBox = false;      // Paint only around the first value (mouse position) a box
        painter.drawRect(box);
      }
      else
      {
        painter.eraseRect(box);
      }

      // and paint the text
      rect.moveCenter(QPoint(0, y));
      if(rect.top() < -mChartArea.height() + 2) rect.moveTop(-mChartArea.height() + 2);
      if(rect.bottom() > -1) rect.moveBottom(-1);
      rect.moveLeft(0 + 8);
      painter.drawText(rect, Qt::AlignLeft, text);
    }
  }
}

/***********************************************************************
*
*                              private stuff
*
************************************************************************/

bool IndicatorPainter::parse(const QStringList& indiFile)
{
  // unused (!?)
  qDebug() << indiFile;
  return true;
}

void  IndicatorPainter::readSettings()
{
  // unused (!?)
}

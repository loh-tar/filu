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

#ifndef COTYPE_HPP
#define COTYPE_HPP

#include <QPaintDevice>
#include <QPainterPath>
#include <QPolygonF>
#include <QVariant>
class QDialog;

#include "FObject.h"
#include "Grip.h"
class COTuple;
class IndicatorPainter;
class Scaler;

/***********************************************************************
*
*   This is the base class of all kind of chart objects
*
************************************************************************/

class COType : public FObject
{
  Q_OBJECT

  public:

    typedef enum
    {
      eBrandNew,
      eSelected,
      eNormal,
      eMoving,
      eToKill
    }Status;

    enum AttrPos      // The position of attribute in the edit window
    {
      ePrepend = 0,
      eAppend  = 1000 // Should more than enough editable attributes
    };

    typedef QHash<QString, QVariant> Attributes;
    typedef QHash<Grip::Type, Grip*> Grips;

                    COType(IndicatorPainter* painter, const QString& className);
    virtual        ~COType();

    static void     getAllTypes(QStringList& list);
    static COType*  createNew(COTuple* co, IndicatorPainter* painter);
    static COType*  createNew(const QString& type, IndicatorPainter* painter);

    void            erase(); // Needed?
    void            setStatus(Status status); // Needed?
    Status          status();

            void    paint(QPaintDevice* sheet);
    virtual bool    isInvolved(const QPoint& pos);
    virtual void    showEditWindow(); // Must public?

    Status          handleEvent(QEvent* event);

  protected slots:
    virtual void    editWindowChanged();

  protected:
            void    prepare(COTuple* co);
    virtual void    prepare(const QHash<QString, QString>& keyValue);
    virtual void    readAttributes(bool firstCall = false);
    virtual void    writeAttributes();
            bool    save();
            void    packParm(QString& parameter);
    virtual bool    paintObject();
    virtual void    paintHoverInfo();
    virtual void    paintBoundingBox();
    virtual Grip*   completeBrandNew(const QPoint& pos);

    // These two are called from mouseMoved() and has to recalc all nessessary things
    virtual void    gripMoved( const QPoint& deltaPos
                             , const int deltaIdx
                             , const double deltaValue);

    virtual void    objectMoved( const QPoint& pos
                               , const int deltaIdx
                               , const double deltaValue);

            void    setAttribute( const QString& name, const QVariant& value);
            void    setAttribute( const QString& name
                                , const QVariant& value
                                , const QString& userTxt, int pos = eAppend);

            Grip*   createNewGrip(const Grip::Type type, const QPointF& pos = QPoint(-1, -1));
            Grip*   renameGrip(Grip* grip, const Grip::Type newType);
            Grip*   gripInvolved(const QPoint& point);      // Returns the grip wich is hit by point

    // ***   event handler   ***
    virtual bool    mouseMoved(const QPoint& pos
                              , const Qt::MouseButtons btns
                              , const Qt::KeyboardModifiers mod);

    virtual bool    leftButtonPressed(const QPoint& pos);
    virtual bool    leftButtonReleased(const QPoint& pos);
    virtual bool    rightButtonPressed(const QPoint& pos);

    QDialog*        mEditWindow;
    Attributes      mAttribute;           // Holds any other data an CO could have
    QStringList     mAttrOrder;           // Holds the attribute names in the order to show in edit window
    QStringList     mAttrTxt;             // Holds the user readable name of the attribute
    QPainterPath    mObject;
    QPolygonF       mPaintedObject;
    Grips           mGrip;                // All available grips
    Grip*           mMovingGrip;          // User is editing this
    Grip*           mAnchor;              // It's the "master" position
    Status          mStatus;
    bool            mHover;               // Not a extra status!
    int             mClicksLeftTillNewIsPlaced;

    QRect             mChartArea;
    Scaler*           mScaler;            // Unused here, useful in subclasses?
    IndicatorPainter* mP;                 //
    QPaintDevice*     mSheet;             // Holds the pointer given by paint(...)
};
/*
class MyTextEdit : public QTextEdit
{
  public:
    MyTextEdit(const QString& text, QWidget* parent = 0) : QTextEdit(text, parent)
    {
      setMinimumHeight(minimumSizeHint().height() / 2);
      setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    }

    //QSize sizeHint() { return minimumSizeHint(); }
};*/


#endif

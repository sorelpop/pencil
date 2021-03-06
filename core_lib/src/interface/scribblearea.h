/*

Pencil - Traditional Animation Software
Copyright (C) 2005-2007 Patrick Corrieri & Pascal Naidon
Copyright (C) 2012-2018 Matthew Chiawen Chang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/


#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <cstdint>
#include <ctime>
#include <deque>
#include <memory>

#include <QColor>
#include <QTransform>
#include <QPoint>
#include <QWidget>
#include <QPixmapCache>

#include "movemode.h"
#include "log.h"
#include "pencildef.h"
#include "bitmapimage.h"
#include "colourref.h"
#include "vectorselection.h"
#include "canvaspainter.h"
#include "preferencemanager.h"

class Layer;
class Editor;
class BaseTool;
class StrokeManager;


class ScribbleArea : public QWidget
{
    Q_OBJECT

    friend class MoveTool;
    friend class EditTool;
    friend class SmudgeTool;
    friend class BucketTool;

public:
    ScribbleArea( QWidget *parent );
    ~ScribbleArea();

    bool init();
    void setCore( Editor* pCore ) { mEditor = pCore; }

    void deleteSelection();
    void setSelection( QRectF rect );
    void adjustSelection(float offsetX, float offsetY, qreal rotatedAngle);
    void applySelectionChanges();
    void displaySelectionProperties();
    void resetSelectionProperties();

    bool isSomethingSelected() const;
    QRectF getSelection() const { return mySelection; }
    QRectF mySelection;
    QRectF myTransformedSelection;
    QRectF myTempTransformedSelection;
    qreal myRotatedAngle = 0.0;
    QList<int> mClosestCurves;

    bool areLayersSane() const;
    bool isLayerPaintable() const;
    bool allowSmudging();

    void flipSelection(bool flipVertical);

    QVector<QPoint> calcSelectionCenterPoints();

    void setEffect( SETTING e, bool isOn );

    int showAllLayers() const { return mShowAllLayers; }
    qreal getCurveSmoothing() const { return mCurveSmoothingLevel; }
    bool usePressure() const { return mUsePressure; }
    bool makeInvisible() const { return mMakeInvisible; }

    void setMoveMode( MoveMode moveMode ) { mMoveMode = moveMode; }
    MoveMode getMoveMode() const { return mMoveMode; }
    void findMoveModeOfCornerInRange();
    MoveMode getMoveModeForSelectionAnchor();

    QPointF whichAnchorPoint(QPointF anchorPoint);

    QRectF getCameraRect();
    QPointF getCentralPoint();

    void updateCurrentFrame();
    void updateFrame( int frame );
    void updateAllFrames();
    void updateAllVectorLayersAtCurrentFrame();
    void updateAllVectorLayersAt(int frameNumber);

    bool shouldUpdateAll() const { return mNeedUpdateAll; }
    void setAllDirty() { mNeedUpdateAll = true; }

    BaseTool* currentTool();
    BaseTool* getTool( ToolType eToolMode );
    void setCurrentTool( ToolType eToolMode );
    void setTemporaryTool( ToolType eToolMode );
    void setPrevTool();

    StrokeManager* getStrokeManager() const { return mStrokeManager.get(); }

    Editor* editor() const { return mEditor; }

    void floodFillError( int errorType );

    bool isMouseInUse() const { return mMouseInUse; }
    bool isTemporaryTool() const { return instantTool; }

    void manageSelectionOrigin(QPointF currentPoint, QPointF originPoint);

signals:
    void modification( int );
    void multiLayerOnionSkinChanged( bool );
    void refreshPreview();

public slots:
    void clearImage();
    void calculateSelectionRect();
    QTransform getSelectionTransformation() const { return selectionTransformation; }
    void calculateSelectionTransformation();
    void paintTransformedSelection();
    void applyTransformedSelection();
    void cancelTransformedSelection();
    void setModified( int layerNumber, int frameNumber );

    inline bool transformHasBeenModified() {
        return (mySelection != myTempTransformedSelection) || myRotatedAngle != 0;
    }

    void selectAll();
    void deselectAll();

    void setCurveSmoothing( int );
    void toggleThinLines();
    void toggleOutlines();
    void toggleShowAllLayers();

    void updateToolCursor();
    void paletteColorChanged(QColor);

protected:
    void tabletEvent( QTabletEvent* ) override;
    void wheelEvent( QWheelEvent* ) override;
    void mousePressEvent( QMouseEvent* ) override;
    void mouseMoveEvent( QMouseEvent* ) override;
    void mouseReleaseEvent( QMouseEvent* ) override;
    void mouseDoubleClickEvent( QMouseEvent* ) override;
    void keyPressEvent( QKeyEvent* ) override;
    void keyReleaseEvent( QKeyEvent* ) override;
    void paintEvent( QPaintEvent* ) override;
    void resizeEvent( QResizeEvent* ) override;

public:
    void drawPolyline(QPainterPath path, QPen pen, bool useAA );
    void drawLine( QPointF P1, QPointF P2, QPen pen, QPainter::CompositionMode cm );
    void drawPath( QPainterPath path, QPen pen, QBrush brush, QPainter::CompositionMode cm );
    void drawPen( QPointF thePoint, qreal brushWidth, QColor fillColour, bool useAA = true );
    void drawPencil( QPointF thePoint, qreal brushWidth, qreal fixedBrushFeather, QColor fillColour, qreal opacity );
    void drawBrush( QPointF thePoint, qreal brushWidth, qreal offset, QColor fillColour, qreal opacity, bool usingFeather = true, int useAA = 0 );
    void blurBrush( BitmapImage *bmiSource_, QPointF srcPoint_, QPointF thePoint_, qreal brushWidth_, qreal offset_, qreal opacity_ );
    void liquifyBrush( BitmapImage *bmiSource_, QPointF srcPoint_, QPointF thePoint_, qreal brushWidth_, qreal offset_, qreal opacity_ );

    void paintBitmapBuffer();
    void paintBitmapBufferRect(const QRect& rect);
    void paintCanvasCursor(QPainter& painter);
    void clearBitmapBuffer();
    void refreshBitmap( const QRectF& rect, int rad );
    void refreshVector( const QRectF& rect, int rad );
    void setGaussianGradient( QGradient &gradient, QColor colour, qreal opacity, qreal offset );

    void updateCanvasCursor();

    /// Call this when starting to use a paint tool. Checks whether we are drawing
    /// on an empty frame, and if so, takes action according to use preference.
    void handleDrawingOnEmptyFrame();

    BitmapImage* mBufferImg = nullptr; // used to pre-draw vector modifications
    BitmapImage* mStrokeImg = nullptr; // used for brush strokes before they are finalized

    QPixmap mCursorImg;
    QPixmap mTransCursImg;

    QPointF getTransformOffset() { return mOffset; }

private:
    void drawCanvas( int frame, QRect rect );
    void settingUpdated(SETTING setting);
    void paintSelectionVisuals(QPainter& painter);

    MoveMode mMoveMode = MoveMode::NONE;
    ToolType mPrevTemporalToolType = ERASER;
    ToolType mPrevToolType = PEN; // previous tool (except temporal)

    BitmapImage mBitmapSelection; // used to temporary store a transformed portion of a bitmap image

    std::unique_ptr< StrokeManager > mStrokeManager;

    Editor* mEditor = nullptr;

    bool mIsSimplified  = false;
    bool mShowThinLines = false;
    bool mQuickSizing = true;
    int  mShowAllLayers = 1;
    bool mUsePressure   = true;
    bool mMakeInvisible = false;
    bool mToolCursors   = true;
    qreal mCurveSmoothingLevel = 0.0;
    bool mMultiLayerOnionSkin; // future use. If required, just add a checkbox to updated it.
    QColor mOnionColor;

    bool mNeedUpdateAll = false;
  

private: 
    bool mKeyboardInUse = false;
    bool mMouseInUse    = false;
    bool mMouseRightButtonInUse = false;
    QPointF mLastPixel;
    QPointF mCurrentPixel;
    QPointF mLastPoint;
    QPointF mCurrentPoint;

    qreal selectionTolerance = 8.0;
    QList<VertexRef> mClosestVertices;
    QPointF mOffset;
    QPoint mCursorCenterPos;

    QPointF transformedCursorPos;

    //instant tool (temporal eg. eraser)
    bool instantTool = false; //whether or not using temporal tool
    bool mSomethingSelected = false;

    VectorSelection vectorSelection;
    QTransform selectionTransformation;

    PreferenceManager* mPrefs = nullptr;

    QPixmap mCanvas;
    CanvasPainter mCanvasPainter;

	// Pixmap Cache keys
	std::vector<QPixmapCache::Key> mPixmapCacheKeys;

    // debug
    QRectF mDebugRect;
    QLoggingCategory mLog;
    std::deque< clock_t > mDebugTimeQue;

    QPolygonF mCurrentTransformSelection;
    QPolygonF mLastTransformSelection;
};

#endif

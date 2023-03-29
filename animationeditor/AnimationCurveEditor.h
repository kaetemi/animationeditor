/*

Copyright (C) 2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This library contains code that was generated using ChatGPT and Copilot.

*/

#pragma once
#ifndef ANIMATION_CURVE_EDITOR__H
#define ANIMATION_CURVE_EDITOR__H

#include "AnimationEditorGlobal.h"

#include <QWidget>
#include <QSet>
#include <QRect>

#include "AnimationTrack.h"

class QTreeWidget;
class QMenu;
class QAction;

class AnimationCurveEditor : public QWidget
{
	Q_OBJECT

public:
	explicit AnimationCurveEditor(QWidget *parent, QTreeWidget *dimensionalReference);
	virtual ~AnimationCurveEditor();

	// Set and get the animation tracks
	void setAnimationTracks(const QList<AnimationTrack *> &tracks);
	const QList<AnimationTrack *> &animationTracks() const;

	// Set and get the keyframe selection
	void setKeyframeSelection(const QSet<ptrdiff_t> &selection);
	QSet<ptrdiff_t> keyframeSelection() const;

signals:
	void rangeChanged(double fromTime, double toTime);
	void trackRemoved(AnimationTrack *track);
	void trackChanged(AnimationTrack *track);
	void selectionChanged(const QSet<ptrdiff_t> &selection);

protected:
	// Override paintEvent to customize drawing
	void resizeEvent(QResizeEvent *event) override;
	void paintEvent(QPaintEvent *event) override;

	// Additional event handlers for mouse interactions
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;
	void enterEvent(QEnterEvent *event) override;
	void leaveEvent(QEvent *event) override;
	bool eventFilter(QObject *watched, QEvent *event) override;
	void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
	// Context menu actions
	void removeTrack();
	void addKeyframe();
	void removeKeyframe();
	void onContextMenuClosed();

private:
	// Context menu management
	void createContextMenu();

	// Enum for user interaction state
	enum class InteractionState
	{
		None = 0,
		SelectMove,
		MoveOnly,
		SelectMoveLeftHandle,
		MoveLeftHandleOnly,
		SelectMoveRightHandle,
		MoveRightHandleOnly,
		MultiSelect
	};

	// Utility functions for working with keyframes and control points
	QRect gridRect() const;
	QPointF keyframePointF(double time, double value) const;
	QPoint keyframePointOffset(double time, double value) const;
	QPoint keyframePoint(double time, double value) const;
	double timeAtX(double x) const;
	double timeAtX(int x) const;
	QPointF timeValueAtXY(const QPoint &pos) const;
	ptrdiff_t keyframeAtPosition(const QPoint &pos, AnimationTrack **trackRes = nullptr) const;
	ptrdiff_t leftHandleAtPosition(const QPoint &pos, AnimationTrack **trackRes = nullptr) const;
	ptrdiff_t rightHandleAtPosition(const QPoint &pos, AnimationTrack **trackRes = nullptr) const;
	QSet<ptrdiff_t> keyframesAtPosition(const QPoint &pos) const;
	QSet<ptrdiff_t> keyframesInRect(const QRect &rect) const;
	void recalculateGridInverval();

	// Paint and layout helper functions
	void paintEditorBackground(QPainter &painter);
	void paintGrid(QPainter &painter);
	void paintValueRuler(QPainter &painter);
	void paintCurve(QPainter &painter, AnimationTrack *track, const QColor &curveColor);
	void paintKeyframe(QPainter &painter, const QRect &rect, bool selected, bool hover, bool active);
	void paintInterpolationHandle(QPainter &painter, const QRect &rect, bool selected, bool hover, bool active);

	// Mouse interaction helper functions
	void updateMousePosition(const QPoint &pos, bool ctrlHeld);
	void updateMouseSelection(bool ctrlHeld);
	void restoreAnimationTracks();

private:
	QTreeWidget *m_DimensionalReference;
	QList<AnimationTrack *> m_AnimationTracks;
	QList<AnimationTrack::KeyframeMap> m_BackupAnimationTracks;
	InteractionState m_InteractionState = InteractionState::None;
	QSet<ptrdiff_t> m_SelectedKeyframes;
	QSet<ptrdiff_t> m_SelectedLeftInterpolationHandles;
	QSet<ptrdiff_t> m_SelectedRightInterpolationHandles;
	QSet<ptrdiff_t> m_BackupSelectedKeyframes;
	QSet<ptrdiff_t> m_BackupSelectedLeftInterpolationHandles;
	QSet<ptrdiff_t> m_BackupSelectedRightInterpolationHandles;

	// Mouse interaction state
	ptrdiff_t m_ActiveKeyframe = -1;
	ptrdiff_t m_ActiveLeftInterpolationHandle = -1;
	ptrdiff_t m_ActiveRightInterpolationHandle = -1;
	AnimationTrack *m_ActiveTrack = nullptr;
	ptrdiff_t m_HoverKeyframe = -1;
	ptrdiff_t m_HoverLeftInterpolationHandle = -1;
	ptrdiff_t m_HoverRightInterpolationHandle = -1;
	AnimationTrack *m_HoverTrack = nullptr;
	QPoint m_MouseLeftPressPosition;
	QPointF m_MouseLeftPressTimeValue;
	QPoint m_MouseRightPressPosition;
	QPointF m_MouseRightPressTimeValue;
	QPoint m_MouseMovePosition;
	bool m_SkipContextMenu = false;
	bool m_SelectMoveTresholdPassed = false;

	// Context menu actions
	QMenu *m_ContextMenu = nullptr;
	QAction *m_RemoveTrackAction = nullptr;
	QAction *m_AddKeyframeAction = nullptr;
	QAction *m_RemoveKeyframeAction = nullptr;

	// Grid
	double m_HorizontalPrimaryTimeInterval = 1.0;
	double m_HorizontalSecondaryTimeInterval = 0.1;
	double m_VerticalPrimaryValueInterval = 1.0;
	double m_VerticalSecondaryValueInterval = 0.1;

	// Scale
	double m_VerticalCenterValue = 0;
	double m_VerticalPixelPerValue = 400.0 / 10.0;
	double m_FromTime = 0.0;
	double m_ToTime = 10.0;
};

#endif /* ANIMATION_CURVE_EDITOR__H */

/* end of file */

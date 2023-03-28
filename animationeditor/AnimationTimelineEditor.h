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
#ifndef ANIMATION_TIMELINE_EDITOR__H
#define ANIMATION_TIMELINE_EDITOR__H

#include "AnimationEditorGlobal.h"

#include <QWidget>
#include <QList>
#include <QSet>
#include <QMenu>

#include "AnimationTrack.h"

class QMouseEvent;
class QWheelEvent;
class QTreeWidget;
class QMenu;
class QAction;

class ANIMATIONEDITOR_EXPORT AnimationContextMenu : public QMenu
{
	Q_OBJECT

public:
	explicit AnimationContextMenu(QWidget *parent = nullptr);

signals:
	void menuClosed();

protected:
	void hideEvent(QHideEvent *event) override;
};

class ANIMATIONEDITOR_EXPORT AnimationTimelineEditor : public QWidget
{
	Q_OBJECT

public:
	explicit AnimationTimelineEditor(QWidget *parent = nullptr);
	virtual ~AnimationTimelineEditor();

	// Set and get the animation tracks
	void setAnimationTracks(const QList<AnimationTrack *> &tracks);
	const QList<AnimationTrack *> &animationTracks() const;

	// Set and get the keyframe selection
	void setKeyframeSelection(const QSet<ptrdiff_t> &selection);
	QSet<ptrdiff_t> keyframeSelection() const;

	QRect visualTrackRect(AnimationTrack *track) const;

signals:
	void rangeChanged(double fromTime, double toTime);
	void trackRemoved(AnimationTrack *track); // Widget doesn't actually remove, editor needs to handle it
	void trackChanged(AnimationTrack *track);
	void selectionChanged(const QSet<ptrdiff_t> &selection);

protected:
	// Override paintEvent to customize drawing
	void paintEvent(QPaintEvent *event) override;

	// Additional event handlers
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
	// Create the context menu and actions
	void createContextMenu();

	// Paint and layout functions
	QRect rowsRect();
	QRect visualTrackRectInWidgetSpace(AnimationTrack *track);
	QRect keyframeRect(AnimationTrack *track, double time);
	AnimationTrack *trackAtPosition(const QPoint &pos);
	ptrdiff_t keyframeAtPosition(AnimationTrack *track, const QPoint &pos);
	void paintEditorBackground(QPainter &painter);
	void paintKeyframe(QPainter &painter, const QRect &keyframeRect, bool isSelected, bool isHovered, bool isPressed);

	// Mouse updates
	void updateMouseHover(const QPoint &pos);
	void updateMouseSelection(bool ctrlHeld);

	// Helper functions
	int timeToX(double time) const;
	double xToTime(int x) const;

	// Context menu actions
	QMenu *m_ContextMenu = nullptr;
	QAction *m_RemoveTrackAction = nullptr;
	QAction *m_AddKeyframeAction = nullptr;
	QAction *m_RemoveKeyframeAction = nullptr;

	// Associated QTreeWidget
	QTreeWidget *m_TreeWidget = nullptr;

	// List of animation tracks
	QList<AnimationTrack *> m_AnimationTracks;

	// Original animation tracks backup
	QList<QMap<double, AnimationKeyframe>> m_OriginalAnimationTracks;

	// Keyframe selection and backup
	QSet<ptrdiff_t> m_SelectedKeyframes;
	QList<ptrdiff_t> m_SelectedKeyframesBackup;

	// Hover and pressed keyframe and track
	ptrdiff_t m_HoverKeyframe;
	ptrdiff_t m_CurrentHoverKeyframe;
	ptrdiff_t m_PressedKeyframe;
	ptrdiff_t m_RightPressedKeyframe;
	AnimationTrack *m_HoverTrack = nullptr;
	AnimationTrack *m_CurrentHoverTrack = nullptr;
	AnimationTrack *m_ContextMenuTrack = nullptr;

	// Mouse hover state
	bool m_MouseHover = false;

	// Mouse move position and related variables
	QPoint m_MouseMovePosition;
	QPoint m_TrackMoveStart;
	QPoint m_SelectionStart;
	QPoint m_ContextMousePosition;
	bool m_SkipContextMenu = false;
	bool m_ContextMenuOpen = false;

	// The duration range of the animation timeline
	double m_FromTime;
	double m_ToTime;

}; /* class AnimationTimelineEditor */

#endif /* ANIMATION_TIMELINE_EDITOR__H */

/* end of file */

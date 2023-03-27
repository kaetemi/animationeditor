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

#include "AnimationTrack.h"

class QMouseEvent;
class QWheelEvent;

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

private:
	QList<AnimationTrack *> m_AnimationTracks;
	QList<QMap<double, AnimationKeyframe>> m_OriginalAnimationTracks;
	QSet<ptrdiff_t> m_SelectedKeyframes;
	QPoint m_MousePressPosition;

	// The duration range of the animation timeline
	double m_FromTime;
	double m_ToTime;

private:
	// Helper function to calculate the X position for a given time
	int timeToX(double time) const;

	// Helper function to calculate the time for a given X position
	double xToTime(int x) const;

}; /* class AnimationTimelineEditor */

#endif /* ANIMATION_TIMELINE_EDITOR__H */

/* end of file */

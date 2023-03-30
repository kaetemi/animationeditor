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
#ifndef ANIMATION_TIME_SCRUBBER__H
#define ANIMATION_TIME_SCRUBBER__H

#include "AnimationEditorGlobal.h"

#include <QWidget>

class QTreeWidget;

class AnimationTimeScrubber : public QWidget
{
	Q_OBJECT

public:
	explicit AnimationTimeScrubber(QWidget *parent, QTreeWidget *dimensionalReference);
	virtual ~AnimationTimeScrubber();

	// Set and get the current time
	void setCurrentTime(double time);
	double currentTime() const;

	// Set and get the active range
	void setActiveRange(double fromTime, double toTime);
	void activeRange(double &fromTime, double &toTime) const;

signals:
	void currentTimeChanged(double time);
	void activeRangeChanged(double fromTime, double toTime);

protected:
	// Override paintEvent to customize drawing
	void resizeEvent(QResizeEvent *event) override;
	void paintEvent(QPaintEvent *event) override;

	// Additional event handlers for mouse interactions
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;

private:
	// Enum for user interaction state
	enum class InteractionState
	{
		None = 0,
		Scrubbing,
		MovingActiveRange,
		ResizingLeftActiveRange,
		ResizingRightActiveRange,
		Zooming,
	};

	// Utility functions for working with time and range handles
	int xAtTime(double time) const;
	double timeAtX(int x) const;

	// Paint and layout helper functions
	void recalculateRulerInverval();
	QRect rulerRect() const;
	QRect activeRangeRect() const;
	QRect scrubberHandleRect() const;
	QRect fromHandleRect() const;
	QRect toHandleRect() const;
	void paintBackground(QPainter &painter);
	void paintRuler(QPainter &painter);
	void paintActiveRange(QPainter &painter);
	void paintScrubber(QPainter &painter);

	// Mouse interaction helper functions
	void updateMouseInteraction(const QPoint &pos);

private:
	QTreeWidget *m_DimensionalReference = nullptr;
	double m_HorizontalPrimaryTimeInterval = 1.0;
	double m_HorizontalSecondaryTimeInterval = 0.1;
	double m_CurrentTime = 2;
	double m_FromTime = 0;
	double m_ToTime = 10;
	InteractionState m_InteractionState;
	QPoint m_MouseLeftPressPosition;
	double m_MouseLeftPressTime = 0;
	bool m_ScrubberActive = false;
	bool m_ScrubberHovered = false;

}; /* class AnimationEditor */

#endif /* ANIMATION_TIME_SCRUBBER__H */

/* end of file */

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

class ANIMATIONEDITOR_EXPORT AnimationTimeScrubber : public QWidget
{
	Q_OBJECT

public:
	explicit AnimationTimeScrubber(QWidget *parent = nullptr);
	virtual ~AnimationTimeScrubber();

	// Set the duration of the animation timeline.
	void setDuration(double duration);

	// Set the current time of the scrubber.
	void setCurrentTime(double time);

	// Get the current time of the scrubber.
	double currentTime() const;

signals:
	// Signal emitted when the current time changes.
	void currentTimeChanged(double time);

protected:
	// Override the paint event to draw the time ruler and scrubber handle.
	void paintEvent(QPaintEvent *event) override;

	// Override the mouse events to handle scrubber interactions.
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;

private:
	// Convert the timeline duration to a pixel value.
	int timeToPixel(double time) const;

	// Convert a pixel value to the timeline duration.
	double pixelToTime(int pixel) const;

	// Helper method to calculate the width of the ruler.
	int rulerWidth() const;

	// The duration of the animation timeline.
	double m_Duration;

	// The current time of the scrubber.
	double m_CurrentTime;

	// A flag indicating whether the user is currently dragging the scrubber.
	bool m_IsDragging;
}; /* class AnimationEditor */

#endif /* ANIMATION_TIME_SCRUBBER__H */

/* end of file */

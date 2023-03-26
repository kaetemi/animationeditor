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

#include "AnimationTimeScrubber.h"

#include <QMouseEvent>
#include <QPainter>

AnimationTimeScrubber::AnimationTimeScrubber(QWidget *parent)
    : QWidget(parent)
    , m_duration(0.0f)
    , m_currentTime(0.0f)
    , m_isDragging(false)
{
	setMinimumHeight(50);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

AnimationTimeScrubber::~AnimationTimeScrubber()
{
}

void AnimationTimeScrubber::setDuration(float duration)
{
	m_duration = duration;
	update();
}

void AnimationTimeScrubber::setCurrentTime(float time)
{
	m_currentTime = qBound(0.0f, time, m_duration);
	update();
}

float AnimationTimeScrubber::currentTime() const
{
	return m_currentTime;
}

int AnimationTimeScrubber::timeToPixel(float time) const
{
	return static_cast<int>((time / m_duration) * rulerWidth());
}

float AnimationTimeScrubber::pixelToTime(int pixel) const
{
	return (static_cast<float>(pixel) / rulerWidth()) * m_duration;
}

int AnimationTimeScrubber::rulerWidth() const
{
	return width() - 2; // 1 pixel margin on each side
}

void AnimationTimeScrubber::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		int scrubberHandlePosition = timeToPixel(m_currentTime);
		int clickPosition = event->pos().x();

		// Check if the click is within a threshold of the scrubber handle position
		if (abs(clickPosition - scrubberHandlePosition) <= 5) // 5 pixels as an example threshold
		{
			m_isDragging = true;
			event->accept();
		}
	}
}

void AnimationTimeScrubber::mouseMoveEvent(QMouseEvent *event)
{
	if (m_isDragging)
	{
		float newTime = pixelToTime(event->pos().x());
		setCurrentTime(newTime);
		emit currentTimeChanged(newTime);
		event->accept();
	}
}

void AnimationTimeScrubber::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton && m_isDragging)
	{
		m_isDragging = false;
		event->accept();
	}
}

void AnimationTimeScrubber::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	int width = rulerWidth();
	int height = rect().height();
	int tickHeight = height / 3;

	// Draw the background
	painter.fillRect(rect(), Qt::white);

	// Draw the tick marks and labels
	for (int i = 0; i <= m_duration; ++i)
	{
		int x = timeToPixel(i);
		painter.setPen(Qt::black);
		painter.drawLine(x, height - tickHeight, x, height);

		QString label = QString::number(i);
		QRect labelRect(x - 20, 0, 40, height - tickHeight - 2);
		painter.drawText(labelRect, Qt::AlignCenter, label);
	}

	// Draw the scrubber handle
	int handleX = timeToPixel(m_currentTime);
	int handleHeight = height / 2;
	QRect handleRect(handleX - 5, height - handleHeight - 1, 10, handleHeight);
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::blue);
	painter.drawRoundedRect(handleRect, 2, 2);
}

/* end of file */
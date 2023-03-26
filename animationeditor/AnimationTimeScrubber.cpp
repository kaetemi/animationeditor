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
    , m_Duration(90.0)
    , m_CurrentTime(10.0)
    , m_IsDragging(false)
{
	setMinimumHeight(50);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

AnimationTimeScrubber::~AnimationTimeScrubber()
{
}

void AnimationTimeScrubber::setDuration(double duration)
{
	m_Duration = duration;
	update();
}

void AnimationTimeScrubber::setCurrentTime(double time)
{
	m_CurrentTime = qBound(0.0f, time, m_Duration);
	update();
}

double AnimationTimeScrubber::currentTime() const
{
	return m_CurrentTime;
}

int AnimationTimeScrubber::timeToPixel(double time) const
{
	return static_cast<int>((time / m_Duration) * rulerWidth());
}

double AnimationTimeScrubber::pixelToTime(int pixel) const
{
	return (static_cast<double>(pixel) / rulerWidth()) * m_Duration;
}

int AnimationTimeScrubber::rulerWidth() const
{
	return width() - 2; // 1 pixel margin on each side
}

void AnimationTimeScrubber::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		int scrubberHandlePosition = timeToPixel(m_CurrentTime);
		int clickPosition = event->pos().x();

		// Check if the click is within a threshold of the scrubber handle position
		if (abs(clickPosition - scrubberHandlePosition) <= 5) // 5 pixels as an example threshold
		{
			m_IsDragging = true;
			event->accept();
		}
	}
}

void AnimationTimeScrubber::mouseMoveEvent(QMouseEvent *event)
{
	if (m_IsDragging)
	{
		double newTime = pixelToTime(event->pos().x());
		setCurrentTime(newTime);
		emit currentTimeChanged(newTime);
		event->accept();
	}
}

void AnimationTimeScrubber::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton && m_IsDragging)
	{
		m_IsDragging = false;
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
	for (int i = 0; i <= m_Duration; ++i)
	{
		int x = timeToPixel(i);
		painter.setPen(Qt::black);
		painter.drawLine(x, height - tickHeight, x, height);

		QString label = QString::number(i);
		QRect labelRect(x - 20, 0, 40, height - tickHeight - 2);
		painter.drawText(labelRect, Qt::AlignCenter, label);
	}

	// Draw the scrubber handle
	int handleX = timeToPixel(m_CurrentTime);
	int handleHeight = height / 2;
	QRect handleRect(handleX - 5, height - handleHeight - 1, 10, handleHeight);
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::blue);
	painter.drawRoundedRect(handleRect, 2, 2);
}

/* end of file */
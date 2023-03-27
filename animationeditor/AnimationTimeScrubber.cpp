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

#include <QApplication>

#include <QMouseEvent>
#include <QPainter>

#include <QStyle>
#include <QStyleOptionSlider>

AnimationTimeScrubber::AnimationTimeScrubber(QWidget *parent)
    : QWidget(parent)
    , m_FromTime(0.0)
	, m_ToTime(10.0)
	, m_FrameRate(30)
    , m_CurrentTime(2.5)
    , m_IsDragging(false)
{
	setMinimumHeight(50);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

AnimationTimeScrubber::~AnimationTimeScrubber()
{
}

void AnimationTimeScrubber::setDuration(double from, double to)
{
	m_FromTime = from;
	m_ToTime = to;
	update();
}

void AnimationTimeScrubber::setFrameRate(double frameRate)
{
	m_FrameRate = frameRate;
	update();
}

int AnimationTimeScrubber::frameRate() const
{
	return m_FrameRate;
}

void AnimationTimeScrubber::setCurrentTime(double time)
{
	m_CurrentTime = qBound(m_FromTime, time, m_ToTime);
	update();
}

double AnimationTimeScrubber::currentTime() const
{
	return m_CurrentTime;
}

int AnimationTimeScrubber::timeToPixel(double time) const
{
	double timeRange = m_ToTime - m_FromTime;
	double pixelsPerSecond = static_cast<double>(width()) / timeRange;
	return static_cast<int>((time - m_FromTime) * pixelsPerSecond);
}

double AnimationTimeScrubber::pixelToTime(int pixel) const
{
	double timeRange = m_ToTime - m_FromTime;
	double secondsPerPixel = timeRange / static_cast<double>(width());
	return m_FromTime + pixel * secondsPerPixel;
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

/*
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
*/

void AnimationTimeScrubber::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter painter(this);

	// Get the colors from the widget's palette.
	QColor rulerColor = palette().color(QPalette::Window);
	QColor tickColor = palette().color(QPalette::WindowText);
	QColor scrubberColor = palette().color(QPalette::Highlight);

	// Draw the ruler.
	const int rulerTopMargin = 10;
	const int rulerHeight = height() - rulerTopMargin;
	const int rulerWidth = this->rulerWidth();

	painter.fillRect(0, rulerTopMargin, rulerWidth, rulerHeight, rulerColor);

	// Draw tick marks for each second.
	const int tickHeight = 10;
	const int frameTickHeight = 5;

	for (double time = m_FromTime; time <= m_ToTime; time += 1.0)
	{
		int x = timeToPixel(time);

		if (fmod(time, 1.0) == 0.0)
		{
			// Draw a long tick mark for every second.
			painter.fillRect(x, rulerTopMargin, 1, tickHeight * 2, tickColor);
		}
		else if (m_FrameRate > 0 && fmod(time * m_FrameRate, 1.0) == 0.0)
		{
			// Draw a short tick mark for every frame.
			painter.fillRect(x, rulerTopMargin, 1, frameTickHeight, tickColor);
		}
		else
		{
			// Draw a short tick mark for every second.
			painter.fillRect(x, rulerTopMargin, 1, tickHeight, tickColor);
		}
	}

	// Draw the scrubber handle.
	const int scrubberSize = 20;
	int scrubberX = timeToPixel(m_CurrentTime) - scrubberSize / 2;
	int scrubberY = height() - 20;
	QRect scrubberRect(scrubberX, scrubberY, scrubberSize, scrubberSize);

	QStyleOptionSlider option;
	option.initFrom(this);
	option.minimum = 0;
	option.maximum = 1;
	option.sliderPosition = 0;
	option.subControls = QStyle::SC_SliderHandle;
	option.activeSubControls = QStyle::SC_SliderHandle;
	option.rect = scrubberRect;
	option.orientation = Qt::Horizontal;

	// Get the handle style from the widget style and draw it.
	const QStyle* style = QApplication::style();
	style->drawComplexControl(QStyle::CC_Slider, &option, &painter, this);

	// Draw the current time and frame label.
	QString currentTimeText = QString::number(m_CurrentTime, 'f', 2);
	QString currentFrameText = QString::number(timeToPixel(m_CurrentTime), 'f', 0);
	QRect textRect = QRect(scrubberRect.x() + scrubberSize / 2, scrubberY - 20, 100, 20);
	painter.drawText(textRect, Qt::AlignLeft | Qt::AlignTop, currentTimeText + "s / " + currentFrameText + "f @ " + QString::number(m_FrameRate) + "fps");
}


/* end of file */
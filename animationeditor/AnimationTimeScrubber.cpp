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
    , m_CurrentTime(0)
// m_StartTime(0),
// m_EndTime(10),
// m_PlaybackLoopStart(0),
// m_PlaybackLoopEnd(10),
// m_ZoomLevel(1),
// m_IsScrubberPressed(false)
{
	setMouseTracking(true);
	setFixedHeight(40);
}

AnimationTimeScrubber::~AnimationTimeScrubber()
{
}

void AnimationTimeScrubber::setCurrentTime(double time)
{
	if (m_CurrentTime != time)
	{
		m_CurrentTime = time;
		update();
		emit currentTimeChanged(time);
	}
}

double AnimationTimeScrubber::currentTime() const
{
	return m_CurrentTime;
}

void AnimationTimeScrubber::setActiveRange(double fromTime, double toTime)
{
	if (m_FromTime != fromTime || m_ToTime != toTime)
	{
		m_FromTime = fromTime;
		m_ToTime = toTime;
		update();
		emit activeRangeChanged(fromTime, toTime);
	}
}

void AnimationTimeScrubber::activeRange(double &fromTime, double &toTime) const
{
	fromTime = m_FromTime;
	toTime = m_ToTime;
}

QRect AnimationTimeScrubber::rulerRect() const
{
	return QRect(1, 1, width() - 2, height() - 2);
}

int AnimationTimeScrubber::xAtTime(double time) const
{
	QRect ruler = rulerRect();
	double pixelPerSecond = static_cast<double>(ruler.width()) / (m_ToTime - m_FromTime);
	int x = ruler.left() + static_cast<int>((time - m_FromTime) * pixelPerSecond);
	return qBound(ruler.left(), x, ruler.right());
}

double AnimationTimeScrubber::timeAtX(int x) const
{
	QRect ruler = rulerRect();
	double pixelPerSecond = static_cast<double>(ruler.width()) / (m_ToTime - m_FromTime);
	double time = m_FromTime + static_cast<double>(x - ruler.left()) / pixelPerSecond;
	return qBound(m_FromTime, time, m_ToTime);
}

void AnimationTimeScrubber::updateMouseInteraction(const QPoint &pos)
{
	QRect activeRange = activeRangeRect();
	QRect scrubberHandle = scrubberHandleRect();
	QRect fromHandle = fromHandleRect();
	QRect toHandle = toHandleRect();

	if (m_InteractionState == InteractionState::None)
	{
		if (scrubberHandle.contains(pos))
		{
			setCursor(Qt::SizeHorCursor);
			// m_IsScrubberHovered = true;
		}
		else
		{
			setCursor(Qt::ArrowCursor);
			// m_IsScrubberHovered = false;
		}
	}
	else if (m_InteractionState == InteractionState::Scrubbing)
	{
		double newTime = timeAtX(pos.x());
		setCurrentTime(newTime);
	}
	else if (m_InteractionState == InteractionState::MovingActiveRange)
	{
		// double timeOffset = timeAtX(pos.x()) - m_MouseLeftPressTime;
		// double newFromTime = m_MouseLeftPressFromTime + timeOffset;
		// double newToTime = m_MouseLeftPressToTime + timeOffset;
		// setActiveRange(newFromTime, newToTime);
	}
	else if (m_InteractionState == InteractionState::ResizingLeftActiveRange)
	{
		double newFromTime = timeAtX(pos.x());
		setActiveRange(newFromTime, m_ToTime);
	}
	else if (m_InteractionState == InteractionState::ResizingRightActiveRange)
	{
		double newToTime = timeAtX(pos.x());
		setActiveRange(m_FromTime, newToTime);
	}
	else if (m_InteractionState == InteractionState::Zooming)
	{
		// Implement zooming functionality
	}
}

void AnimationTimeScrubber::mousePressEvent(QMouseEvent *event)
{
	updateMouseInteraction(event->pos());

	if (m_InteractionState == InteractionState::Scrubbing)
	{
		m_ScrubberActive = true;
	}
	else
	{
		m_ScrubberActive = false;
	}

	m_MouseLeftPressPosition = event->pos();
	m_MouseLeftPressTime = m_CurrentTime;

	update();
}

void AnimationTimeScrubber::mouseMoveEvent(QMouseEvent *event)
{
	updateMouseInteraction(event->pos());
	update();
}

void AnimationTimeScrubber::mouseReleaseEvent(QMouseEvent *event)
{
	m_ScrubberActive = false;
	updateMouseInteraction(event->pos());
	update();
}

void AnimationTimeScrubber::wheelEvent(QWheelEvent *event)
{
	int delta = event->angleDelta().y();
	if (event->modifiers() & Qt::ControlModifier)
	{
		// Implement zooming functionality here
	}
	else
	{
		// Implement scrolling functionality here, if desired
	}
}


void AnimationTimeScrubber::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);

	// Draw the timeline ruler
	paintRuler(painter);

	// Draw the active range
	paintActiveRange(painter);

	// Draw the scrubber
	paintScrubber(painter);

	/*
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	paintBackground(painter);
	paintTimeRuler(painter);
	paintLoopRange(painter);

	QRect scrubberRect = scrubberHandleRect();

	QStyleOptionSlider option;
	option.initFrom(this);
	option.minimum = 0;
	option.maximum = 1;
	option.sliderPosition = 0;
	option.subControls = QStyle::SC_SliderHandle;
	option.activeSubControls = QStyle::SC_SliderHandle;
	option.rect = scrubberRect;
	option.orientation = Qt::Horizontal;

	// Change the style state depending on user interaction
	if (m_IsScrubberPressed)
	{
	    option.state |= QStyle::State_Sunken;
	}
	else if (scrubberRect.contains(mapFromGlobal(QCursor::pos())))
	{
	    option.state |= QStyle::State_MouseOver;
	}

	// Get the handle style from the widget style and draw it.
	const QStyle* style = QApplication::style();
	style->drawComplexControl(QStyle::CC_Slider, &option, &painter, this);
	*/
}

static QColor lerp(const QColor &color1, const QColor &color2, qreal t)
{
	float r1, g1, b1, a1;
	float r2, g2, b2, a2;

	color1.getRgbF(&r1, &g1, &b1, &a1);
	color2.getRgbF(&r2, &g2, &b2, &a2);

	float r = r1 + (r2 - r1) * t;
	float g = g1 + (g2 - g1) * t;
	float b = b1 + (b2 - b1) * t;
	float a = a1 + (a2 - a1) * t;

	return QColor::fromRgbF(r, g, b, a);
}

void AnimationTimeScrubber::paintRuler(QPainter &painter)
{
	// Paint background
	QRect rulerRect(0, 0, width(), height());
	QBrush baseBrush = palette().brush(QPalette::Base);
	painter.fillRect(rulerRect, baseBrush);

	// Draw frame using QStyle
	QStyleOptionFrame frameOption;
	frameOption.initFrom(this);
	frameOption.rect = rulerRect;
	frameOption.frameShape = QFrame::NoFrame;
	frameOption.lineWidth = 0;
	frameOption.midLineWidth = 0;
	frameOption.state |= QStyle::State_Sunken;
	frameOption.features = QStyleOptionFrame::Flat;
	style()->drawControl(QStyle::CE_ShapedFrame, &frameOption, &painter, this);

	// Draw ruler ticks and labels
	QColor baseColor = palette().color(QPalette::Base);
	QPen lightPen = QPen(baseColor.lighter(200));
	QPen basePen = QPen(lerp(palette().color(QPalette::Button), palette().color(QPalette::ButtonText), 0.2));
	QPen darkPen = QPen(baseColor.darker(200));

	int primaryLength = 12;
	int secondaryLength = 6;
	double pixelPerTime = static_cast<double>(width()) / (m_ToTime - m_FromTime);

	// Draw primary time ticks and labels
	for (double time = m_FromTime; time <= m_ToTime; ++time)
	{
		int x = xAtTime(time);
		painter.setPen(darkPen);
		painter.drawLine(x, height() - primaryLength, x, height());
		painter.setPen(lightPen);
		painter.drawLine(x, 0, x, height() - primaryLength);

		QRect labelRect(x - 24, 0, 48, height() - primaryLength);
		painter.setPen(basePen);
		painter.drawText(labelRect, Qt::AlignHCenter | Qt::AlignTop, QString::number(time, 'f', 2));

		// Draw secondary time ticks
		for (double secondaryTime = time + 1.0 / pixelPerTime; secondaryTime <= (time + 1.0 - 1.0 / pixelPerTime); secondaryTime += 1.0 / pixelPerTime)
		{
			int secondaryX = xAtTime(secondaryTime);
			painter.setPen(darkPen);
			painter.drawLine(secondaryX, height() - secondaryLength, secondaryX, height());
			painter.setPen(lightPen);
			painter.drawLine(secondaryX, 0, secondaryX, height() - secondaryLength);
		}
	}
}

void AnimationTimeScrubber::paintActiveRange(QPainter &painter)
{
	QRect rangeRect = activeRangeRect();
	QColor rangeColor = palette().color(QPalette::Highlight);
	rangeColor.setAlpha(128);
	QBrush rangeBrush(rangeColor);

	painter.setPen(Qt::NoPen);
	painter.setBrush(rangeBrush);
	painter.drawRect(rangeRect);
}

void AnimationTimeScrubber::paintScrubber(QPainter &painter)
{
	const int scrubberSize = 20;
	int scrubberX = xAtTime(m_CurrentTime) - scrubberSize / 2;
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

	if (m_ScrubberActive)
	{
		option.state |= QStyle::State_Sunken;
	}
	else if (m_ScrubberHovered)
	{
		option.state |= QStyle::State_MouseOver;
	}

	// Get the handle style from the widget style and draw it.
	const QStyle *style = QApplication::style();
	style->drawComplexControl(QStyle::CC_Slider, &option, &painter, this);
}

QRect AnimationTimeScrubber::activeRangeRect() const
{
	int fromX = xAtTime(m_FromTime);
	int toX = xAtTime(m_ToTime);
	int rangeHeight = 10; // Define the height of the range indicator

	return QRect(fromX, height() - rangeHeight, toX - fromX, rangeHeight);
}

QRect AnimationTimeScrubber::scrubberHandleRect() const
{
	const int scrubberSize = 20;
	int scrubberX = xAtTime(m_CurrentTime) - scrubberSize / 2;
	int scrubberY = height() - 20;

	return QRect(scrubberX, scrubberY, scrubberSize, scrubberSize);
}

QRect AnimationTimeScrubber::fromHandleRect() const
{
	int handleWidth = 10;
	int handleHeight = 20;
	int fromX = xAtTime(m_FromTime) - handleWidth / 2;
	int handleY = height() - handleHeight;

	return QRect(fromX, handleY, handleWidth, handleHeight);
}

QRect AnimationTimeScrubber::toHandleRect() const
{
	int handleWidth = 10;
	int handleHeight = 20;
	int toX = xAtTime(m_ToTime) - handleWidth / 2;
	int handleY = height() - handleHeight;

	return QRect(toX, handleY, handleWidth, handleHeight);
}

/* end of file */
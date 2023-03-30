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
#include <QTreeWidget>

AnimationTimeScrubber::AnimationTimeScrubber(QWidget *parent, QTreeWidget *dimensionalReference)
    : QWidget(parent)
    , m_DimensionalReference(dimensionalReference)
{
	setMouseTracking(true);
	// setFocusPolicy(Qt::StrongFocus);
	// qApp->installEventFilter(this);
	recalculateRulerInverval();
	// createContextMenu();
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
	QRect rect = QRect(0, 0, width(), height());

	if (m_DimensionalReference)
	{
		int outlineSize = m_DimensionalReference->frameWidth();
		rect.adjust(outlineSize, outlineSize, -outlineSize, -outlineSize);
	}

	return rect;
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

void AnimationTimeScrubber::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	recalculateRulerInverval();
}

void AnimationTimeScrubber::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);

	// Frame and plain background
	paintBackground(painter);
	QRect rect = rulerRect();
	painter.setClipRect(rect);

	// Draw the active range
	paintActiveRange(painter);

	// Draw the timeline ruler
	paintRuler(painter);

	// Draw the scrubber
	paintScrubber(painter);
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

void AnimationTimeScrubber::paintBackground(QPainter &painter)
{
	// Paint background
	QRect rect = rulerRect();
	QBrush baseBrush = palette().brush(QPalette::Base);
	painter.fillRect(rect, baseBrush);

	// Draw frame using QStyle
	QStyleOptionFrame frameOption;
	frameOption.initFrom(this);
	frameOption.rect = QRect(0, 0, width(), height());
	frameOption.frameShape = m_DimensionalReference->frameShape();
	frameOption.lineWidth = m_DimensionalReference->frameWidth();
	frameOption.midLineWidth = m_DimensionalReference->midLineWidth();
	frameOption.state |= QStyle::State_Sunken;
	frameOption.features = QStyleOptionFrame::Flat;
	style()->drawControl(QStyle::CE_ShapedFrame, &frameOption, &painter, this);
}

void AnimationTimeScrubber::paintRuler(QPainter &painter)
{
	// Draw ruler ticks and labels
	QRect rect = rulerRect();
	QColor baseColor = palette().color(QPalette::Base);
	QPen lightPen = QPen(baseColor.lighter(200));
	QPen basePen = QPen(lerp(palette().color(QPalette::Button), palette().color(QPalette::ButtonText), 0.2));
	QPen darkPen = QPen(baseColor.darker(200));

	int primaryLength = 12;
	int secondaryLength = 6;
	// double pixelPerTime = static_cast<double>(width()) / (m_ToTime - m_FromTime);

	double startTime = floor(m_FromTime / m_HorizontalPrimaryTimeInterval) * m_HorizontalPrimaryTimeInterval;
	double endTime = ceil(m_ToTime / m_HorizontalPrimaryTimeInterval) * m_HorizontalPrimaryTimeInterval;

	for (double time = startTime; time <= endTime; time += m_HorizontalPrimaryTimeInterval)
	{
		// Draw primary time ticks
		int primaryX = xAtTime(time);
		painter.setPen(darkPen);
		painter.drawLine(primaryX - 1, 0, primaryX - 1, primaryLength);
		painter.setPen(lightPen);
		painter.drawLine(primaryX, 0, primaryX, primaryLength);

		QRect labelRect(primaryX - 24, 0, 48, primaryLength);
		painter.setPen(basePen);
		painter.drawText(labelRect, Qt::AlignHCenter | Qt::AlignTop, QString::number(time, 'f', 2));

		for (double secondaryTime = time + m_HorizontalSecondaryTimeInterval; secondaryTime <= (time + m_HorizontalPrimaryTimeInterval - (m_HorizontalSecondaryTimeInterval / 2.0)); secondaryTime += m_HorizontalSecondaryTimeInterval)
		{
			// Draw secondary time ticks
			int secondaryX = xAtTime(secondaryTime);
			painter.setPen(darkPen);
			painter.drawLine(secondaryX - 1, 0, secondaryX - 1, secondaryLength);
			painter.setPen(lightPen);
			painter.drawLine(secondaryX, 0, secondaryX, secondaryLength);
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

void AnimationTimeScrubber::recalculateRulerInverval()
{
	QRect grid = rulerRect();

	const double targetPixelsPerPrimary = 100.0; // Target pixels per primary
	double pixelsPerTime = grid.width() / (m_ToTime - m_FromTime);

	const double pixelsPerSecond = pixelsPerTime;
	const double pixelsPerMinute = pixelsPerSecond * 60.0;
	const double pixelsPerHour = pixelsPerSecond * 3600.0;
	const double pixelsPerDay = pixelsPerSecond * 86400.0;

	double timeMultiplier = 1.0;
	if (abs(pixelsPerSecond - targetPixelsPerPrimary) > abs(pixelsPerMinute - targetPixelsPerPrimary))
	{
		timeMultiplier = 60.0;
		if (abs(pixelsPerMinute - targetPixelsPerPrimary) > abs(pixelsPerHour - targetPixelsPerPrimary))
		{
			timeMultiplier = 3600.0;
			if (abs(pixelsPerHour - targetPixelsPerPrimary) > abs(pixelsPerDay - targetPixelsPerPrimary))
				timeMultiplier = 86400.0;
		}
	}
	pixelsPerTime *= timeMultiplier;
	double primaryTimeInterval = targetPixelsPerPrimary / pixelsPerTime;
	primaryTimeInterval = pow(10, round(log10(primaryTimeInterval)));
	if (abs(pixelsPerTime * primaryTimeInterval - targetPixelsPerPrimary) > abs(pixelsPerTime * primaryTimeInterval * 2.0 - targetPixelsPerPrimary))
		primaryTimeInterval *= 2.0;
	else if (abs(pixelsPerTime * primaryTimeInterval - targetPixelsPerPrimary) > abs(pixelsPerTime * primaryTimeInterval * 0.5 - targetPixelsPerPrimary))
		primaryTimeInterval *= 0.5;
	m_HorizontalPrimaryTimeInterval = primaryTimeInterval * timeMultiplier;
	m_HorizontalSecondaryTimeInterval = primaryTimeInterval * timeMultiplier * 0.1;

	update();
}

/* end of file */
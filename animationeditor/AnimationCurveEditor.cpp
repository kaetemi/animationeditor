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

#include "AnimationCurveEditor.h"

#include <QPainter>
#include <QPaintEvent>
#include <QTreeWidget>
#include <QPainterPath>

AnimationCurveEditor::AnimationCurveEditor(QWidget *parent, QTreeWidget *dimensionalReference) : QWidget(parent), m_DimensionalReference(dimensionalReference)
{
}

AnimationCurveEditor::~AnimationCurveEditor()
{
}

void AnimationCurveEditor::setAnimationTracks(const QList<AnimationTrack *> &tracks)
{
	m_AnimationTracks = tracks;
	update();
}

const QList<AnimationTrack *> &AnimationCurveEditor::animationTracks() const
{
	return m_AnimationTracks;
}

void AnimationCurveEditor::setKeyframeSelection(const QSet<ptrdiff_t> &selection)
{
	// Implement setting the keyframe selection
}

QSet<ptrdiff_t> AnimationCurveEditor::keyframeSelection() const
{
	// Implement returning the keyframe selection
	static QSet<ptrdiff_t> dummy;
	return dummy;
}

void AnimationCurveEditor::mousePressEvent(QMouseEvent *event)
{
	// Implement mouse press event handling
}

void AnimationCurveEditor::mouseMoveEvent(QMouseEvent *event)
{
	// Implement mouse move event handling
}

void AnimationCurveEditor::mouseReleaseEvent(QMouseEvent *event)
{
	// Implement mouse release event handling
}

void AnimationCurveEditor::wheelEvent(QWheelEvent *event)
{
	// Implement wheel event handling
}

void AnimationCurveEditor::enterEvent(QEnterEvent *event)
{
	// Implement enter event handling
}

void AnimationCurveEditor::leaveEvent(QEvent *event)
{
	// Implement leave event handling
}

bool AnimationCurveEditor::eventFilter(QObject *watched, QEvent *event)
{
	// Implement event filter
	return false;
}

void AnimationCurveEditor::contextMenuEvent(QContextMenuEvent *event)
{
	// Implement context menu event handling
}

QRect AnimationCurveEditor::gridRect() const
{
	QRect rect = QRect(0, 0, width(), height());

	if (m_DimensionalReference)
	{
		int outlineSize = m_DimensionalReference->frameWidth();
		rect.adjust(outlineSize, outlineSize, -outlineSize, -outlineSize);
	}

	return rect;
}

QPoint AnimationCurveEditor::keyframePoint(double time, double value) const
{
	// Get the grid rectangle
	QRect grid = gridRect();

	// Calculate the horizontal position (x) within the grid rectangle
	double normalizedTime = (time - m_FromTime) / (m_ToTime - m_FromTime);
	double x = grid.left() + normalizedTime * grid.width();

	// Calculate the vertical position (y) within the grid rectangle
	double normalizedValue = (value - m_VerticalCenterValue) * m_VerticalPixelPerValue;
	double y = grid.center().y() - normalizedValue;

	return QPoint(round(x), round(y));
}

AnimationTrack *AnimationCurveEditor::trackAtPosition(const QPoint &pos) const
{
	// Implement logic to find the AnimationTrack at the given position
	return nullptr;
}

AnimationKeyframe AnimationCurveEditor::keyframeAtPosition(const AnimationTrack *track, const QPoint &pos) const
{
	// Implement logic to find the AnimationKeyframe at the given position
	return AnimationKeyframe();
}

void AnimationCurveEditor::paintEditorBackground(QPainter &painter)
{
	// Paint background
	QRect gridRect = this->gridRect();
	QBrush baseBrush = palette().brush(QPalette::Base);
	painter.fillRect(gridRect, baseBrush);

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

void AnimationCurveEditor::paintKeyframe(QPainter &painter, const QRect &rect, bool selected, bool hover, bool active)
{
	// Adjust the keyframe size
	QRect adjustedRect = rect.adjusted(2, 2, -1, -1);

	// Path
	QPointF top = QPointF((adjustedRect.left() + adjustedRect.right()) / 2.0, adjustedRect.top());
	QPointF right = QPointF(adjustedRect.right(), (adjustedRect.top() + adjustedRect.bottom()) / 2.0);
	QPointF bottom = QPointF((adjustedRect.left() + adjustedRect.right()) / 2.0, adjustedRect.bottom());
	QPointF left = QPointF(adjustedRect.left(), (adjustedRect.top() + adjustedRect.bottom()) / 2.0);

	// Adjust the shadow size
	QRect shadowRect = adjustedRect.adjusted(-2, -2, 2, 2);

	// Shadow path
	QPointF shadowTop = QPointF((shadowRect.left() + shadowRect.right()) / 2.0, shadowRect.top());
	QPointF shadowRight = QPointF(shadowRect.right(), (shadowRect.top() + shadowRect.bottom()) / 2.0);
	QPointF shadowBottom = QPointF((shadowRect.left() + shadowRect.right()) / 2.0, shadowRect.bottom());
	QPointF shadowLeft = QPointF(shadowRect.left(), (shadowRect.top() + shadowRect.bottom()) / 2.0);

	QColor shadow = palette().color(QPalette::Shadow).darker(300);
	shadow.setAlpha(128);

	QBrush shadowBrush(shadow);

	QPainterPath shadowPath;
	shadowPath.moveTo(shadowTop);
	shadowPath.lineTo(shadowRight);
	shadowPath.lineTo(shadowBottom);
	shadowPath.lineTo(shadowLeft);
	shadowPath.closeSubpath();

	// Create the gradient brush
	QLinearGradient gradient(top, bottom);

	QColor baseColor;
	QColor lightColor;
	QColor darkColor;
	if (selected)
	{
		baseColor = palette().color(QPalette::Highlight);
		lightColor = baseColor.lighter(150);
		darkColor = baseColor.darker(125);
	}
	else
	{
		baseColor = lerp(palette().color(QPalette::Button), palette().color(QPalette::ButtonText), 0.4);
		lightColor = baseColor.lighter(110);
		darkColor = baseColor.darker(110);
	}

	if (active)
	{
		lightColor = lightColor.darker(110);
		darkColor = darkColor.darker(110);
	}
	else if (hover)
	{
		lightColor = lightColor.lighter(125);
		darkColor = darkColor.lighter(125);
	}

	gradient.setColorAt(0, lightColor);
	gradient.setColorAt(1, darkColor);

	QBrush brush(gradient);

	QPainterPath path;
	path.moveTo(top);
	path.lineTo(right);
	path.lineTo(bottom);
	path.lineTo(left);
	path.closeSubpath();

	painter.save();
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.fillPath(shadowPath, shadowBrush);
	painter.fillPath(path, brush);

	// Draw the border
	QPen borderPen(lightColor.lighter(110));
	borderPen.setWidthF(1.0);

	painter.setPen(borderPen);
	painter.drawPath(path);
	painter.restore();
}

void AnimationCurveEditor::paintGrid(QPainter &painter)
{
	// Paint the grid background with primary control lines and light grid lines
	QRect grid = gridRect();
}

void AnimationCurveEditor::paintValueRuler(QPainter &painter)
{
	QRect grid = gridRect();

	// Calculate the range of values to be displayed
	double minValue = m_VerticalCenterValue - ((grid.height() / 2.0) / m_VerticalPixelPerValue);
	double maxValue = m_VerticalCenterValue + ((grid.height() / 2.0) / m_VerticalPixelPerValue);
	minValue = floor(minValue / m_VerticalPrimaryValueInterval) *m_VerticalPrimaryValueInterval;
	maxValue = ceil(maxValue / m_VerticalPrimaryValueInterval) *m_VerticalPrimaryValueInterval;

	QColor baseColor = palette().color(QPalette::Base);
	QPen lightPen = QPen(baseColor.lighter(200));
	QPen basePen = QPen(lerp(palette().color(QPalette::Button), palette().color(QPalette::ButtonText), 0.2));
	QPen darkPen = QPen(baseColor.darker(200));

	int primaryLength = 48;
	int secondaryLength = 12;

	// Draw primary value ticks and labels
	for (double value = minValue; value <= maxValue; value += m_VerticalPrimaryValueInterval)
	{
		QPoint tickPoint = keyframePoint(m_FromTime, value);
		painter.setPen(darkPen);
		painter.drawLine(tickPoint.x(), tickPoint.y() - 1, tickPoint.x() + primaryLength, tickPoint.y() - 1);
		painter.setPen(lightPen);
		painter.drawLine(tickPoint.x(), tickPoint.y(), tickPoint.x() + primaryLength, tickPoint.y());

		QRect labelRect(tickPoint.x(), tickPoint.y() - 16, 48, 16);
		painter.setPen(basePen);
		painter.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, QString::number(value, 'f', 2));

		// Draw secondary value ticks
		for (double secondaryValue = value + m_VerticalSecondaryValueInterval; secondaryValue <= (value + m_VerticalPrimaryValueInterval - (m_VerticalSecondaryValueInterval / 2.0)); secondaryValue += m_VerticalSecondaryValueInterval)
		{
			tickPoint = keyframePoint(m_FromTime, secondaryValue);
			painter.setPen(darkPen);
			painter.drawLine(tickPoint.x(), tickPoint.y() - 1, tickPoint.x() + secondaryLength, tickPoint.y() - 1);
			painter.setPen(lightPen);
			painter.drawLine(tickPoint.x(), tickPoint.y(), tickPoint.x() + secondaryLength, tickPoint.y());
		}
	}
}

void AnimationCurveEditor::paintCurve(QPainter &painter, AnimationTrack *track, const QColor &curveColor)
{
	// Paint the curve for the given track
	// You'll need to interpolate the curve based on the track's keyframes and interpolation method
}

void AnimationCurveEditor::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter painter(this);

	// Paint the background with frame
	paintEditorBackground(painter);

	// Paint the grid
	QRect grid = gridRect();
	painter.setClipRect(grid);
	paintGrid(painter);
	paintValueRuler(painter);

	// Paint the curves
	for (AnimationTrack *track : m_AnimationTracks)
	{
		QColor curveColor = track->color();
		paintCurve(painter, track, curveColor);

		const QMap<double, AnimationKeyframe> &keyframes = track->keyframes();
		for (QMap<double, AnimationKeyframe>::const_iterator it = keyframes.begin(); it != keyframes.end(); ++it)
		{
			QPoint point = keyframePoint(it.key(), it.value().Value);
			bool isSelected = m_SelectedKeyframes.contains(it.value().Id);
			bool isHovered = false; // Update this based on mouse hover state
			bool isPressed = false; // Update this based on mouse press state
			QRect keyframeRect = QRect(point.x() - 6, point.y() - 6, 12, 12);
			paintKeyframe(painter, keyframeRect, isSelected, isHovered, isPressed);
		}
	}
}

void AnimationCurveEditor::createContextMenu()
{
	// Implement creating the context menu
}

void AnimationCurveEditor::removeTrack()
{
	// Implement removing the track
}

void AnimationCurveEditor::addKeyframe()
{
	// Implement adding a keyframe
}

void AnimationCurveEditor::removeKeyframe()
{
	// Implement removing a keyframe
}

void AnimationCurveEditor::onContextMenuClosed()
{
	// Implement handling the closing of the context menu
}

void AnimationCurveEditor::updateMouseHover(const QPoint &pos)
{
	// Implement updating the mouse hover state
}

void AnimationCurveEditor::updateMouseSelection(bool ctrlHeld)
{
	// Implement updating the mouse selection
}

/* end of file */
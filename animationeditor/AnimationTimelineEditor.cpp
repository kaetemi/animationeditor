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

#include "AnimationTimelineEditor.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QStyleOption>
#include <QTreeWidget>

AnimationTimelineEditor::AnimationTimelineEditor(QWidget *parent)
    : QWidget(parent)
    , m_FromTime(0.0)
    , m_ToTime(10.0)
{
}

AnimationTimelineEditor::~AnimationTimelineEditor()
{
}

void AnimationTimelineEditor::setAnimationTracks(const QList<AnimationTrack *> &tracks)
{
	m_AnimationTracks = tracks;
	update();
}

const QList<AnimationTrack *> &AnimationTimelineEditor::animationTracks() const
{
	return m_AnimationTracks;
}

int AnimationTimelineEditor::timeToX(double time) const
{
	return static_cast<int>((time - m_FromTime) / (m_ToTime - m_FromTime) * width());
}

double AnimationTimelineEditor::xToTime(int x) const
{
	return m_FromTime + (static_cast<double>(x) / width()) * (m_ToTime - m_FromTime);
}

void AnimationTimelineEditor::setKeyframeSelection(const QSet<ptrdiff_t> &selection)
{
	m_SelectedKeyframes = selection;
	update();
}

QSet<ptrdiff_t> AnimationTimelineEditor::keyframeSelection() const
{
	return m_SelectedKeyframes;
}

QRect AnimationTimelineEditor::visualTrackRect(AnimationTrack *track) const
{
	if (!track || !track->m_TreeWidgetItem)
		return QRect();

	// Calculate the track rectangle based on the item rectangle and the widget's dimensions
	QRect rect = track->m_TreeWidgetItem->treeWidget()->visualItemRect(track->m_TreeWidgetItem);
	if (rect.isEmpty())
		return QRect();

	rect.setLeft(0);
	rect.setWidth(width());
	return rect;
}

void AnimationTimelineEditor::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter painter(this);

	// Draw the background using the widget's palette
	QStyleOption option;
	option.initFrom(this);
	style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);

	// Draw the tracks and keyframes using the widget's palette
	for (AnimationTrack *track : m_AnimationTracks)
	{
		QRect trackRect = visualTrackRect(track);
		if (trackRect.isEmpty())
		{
			continue;
		}

		// Draw the track background
		QBrush trackBackgroundBrush = palette().brush(QPalette::AlternateBase);
		painter.fillRect(trackRect, trackBackgroundBrush);

		// Draw a line under the track

		// Draw keyframes
		QBrush keyframeBrush = palette().brush(QPalette::Highlight);
		for (QMap<double, AnimationKeyframe>::const_iterator keyframe = track->keyframes().begin(); keyframe != track->keyframes().end(); ++keyframe)
		{
			int keyframeX = timeToX(keyframe.key());
			QRect keyframeRect(keyframeX - 4, trackRect.y() + (trackRect.height() - 8) / 2, 8, 8);
			painter.fillRect(keyframeRect, keyframeBrush);
		}
	}
}

void AnimationTimelineEditor::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_MousePressPosition = event->pos();

		// Backup all tracks before modifying keyframes
		m_OriginalAnimationTracks.clear();
		for (const AnimationTrack *track : m_AnimationTracks)
		{
			m_OriginalAnimationTracks.append(track->keyframes());
		}

		// Find the clicked keyframe
		for (AnimationTrack *track : m_AnimationTracks)
		{
			QRect trackRect = visualTrackRect(track);
			if (trackRect.contains(event->pos()))
			{
				for (QMap<double, AnimationKeyframe>::const_iterator keyframe = track->keyframes().begin(); keyframe != track->keyframes().end(); ++keyframe)
				{
					int keyframeX = timeToX(keyframe.key());
					QRect keyframeRect(keyframeX - 4, trackRect.y() + (trackRect.height() - 8) / 2, 8, 8);
					if (keyframeRect.contains(event->pos()))
					{
						m_SelectedKeyframes = { keyframe.value().Id };
						emit selectionChanged(m_SelectedKeyframes);
						break;
					}
				}
			}
		}
	}
}

void AnimationTimelineEditor::mouseMoveEvent(QMouseEvent *event)
{
	if (!m_SelectedKeyframes.isEmpty() && event->buttons() & Qt::LeftButton)
	{
		double timeDelta = xToTime(event->pos().x()) - xToTime(m_MousePressPosition.x());

		for (int i = 0; i < m_AnimationTracks.size(); ++i)
		{
			AnimationTrack *track = m_AnimationTracks[i];
			QMap<double, AnimationKeyframe> &originalKeyframes = m_OriginalAnimationTracks[i];

			// Restore the original tracks before modifying keyframes
			track->setKeyframes(originalKeyframes);
			bool changed = false;

			for (QMap<double, AnimationKeyframe>::const_iterator keyframe = originalKeyframes.begin(); keyframe != originalKeyframes.end(); ++keyframe)
			{
				if (m_SelectedKeyframes.contains(keyframe.value().Id))
				{
					double newTime = keyframe.key() + timeDelta;
					track->moveKeyframe(keyframe.key(), newTime);
					changed = true;
				}
			}

			emit trackChanged(track);
		}

		update();
	}
}

void AnimationTimelineEditor::mouseReleaseEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	m_MousePressPosition = QPoint();
	m_OriginalAnimationTracks.clear();
}

void AnimationTimelineEditor::wheelEvent(QWheelEvent *event)
{
	if (event->modifiers() & Qt::ShiftModifier)
	{
		// Handle zooming in/out with Shift + Scroll
		double rangeDelta = event->angleDelta().y() / 1200.0 * (m_ToTime - m_FromTime);
		m_FromTime -= rangeDelta;
		m_ToTime += rangeDelta;
		emit rangeChanged(m_FromTime, m_ToTime);
		update();
	}
	else
	{
		QWidget::wheelEvent(event); // Pass the event to the base class for default behavior
	}
}

/* end of file */
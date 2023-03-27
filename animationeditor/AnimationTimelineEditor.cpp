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

AnimationTimelineEditor::AnimationTimelineEditor(QWidget *parent)
    : QWidget(parent),
	m_FromTime(0.0),
	m_ToTime(10.0)
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

int AnimationTimelineEditor::trackHeight() const
{
	return 20; // Adjust the track height as needed
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
	m_KeyframeSelection = selection;
	update();
}

QSet<ptrdiff_t> AnimationTimelineEditor::keyframeSelection() const
{
	return m_KeyframeSelection;
}

void AnimationTimelineEditor::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	// Set up colors and styles
	QColor trackBackgroundColor(240, 240, 240);
	QColor trackBorderColor(180, 180, 180);
	QColor keyframeColor(80, 80, 80);
	int trackHeight = 20;
	int keyframeWidth = 6;
	int keyframeHeight = 12;

	// Draw the tracks
	int trackY = 0;
	for (const auto &track : m_AnimationTracks)
	{
		QRect trackRect(0, trackY, width(), trackHeight);
		painter.fillRect(trackRect, trackBackgroundColor);
		painter.setPen(trackBorderColor);
		painter.drawRect(trackRect);

		// Draw the keyframes for each track
		painter.setPen(Qt::NoPen);
		painter.setBrush(keyframeColor);
		for (QMap<double, AnimationKeyframe>::const_iterator keyframe = track->keyframes().begin(); keyframe != track->keyframes().end(); ++keyframe)
		{
			// Calculate the X position for the keyframe based on its time
			int keyframeX = static_cast<int>(keyframe.key() * 100); // Assuming 100 pixels per second

			// Check if the keyframe is within the visible area
			if (keyframeX >= 0 && keyframeX < width())
			{
				QRect keyframeRect(keyframeX - keyframeWidth / 2, trackY + (trackHeight - keyframeHeight) / 2, keyframeWidth, keyframeHeight);
				painter.drawRect(keyframeRect);
			}
		}

		trackY += trackHeight;
	}
}


AnimationTrack *AnimationTimelineEditor::keyframeAt(const QPoint &pos, double &time)
{
	int trackHeight = 20;
	int keyframeWidth = 6;
	int trackIndex = pos.y() / trackHeight;

	if (trackIndex < m_AnimationTracks.size())
	{
		AnimationTrack *track = m_AnimationTracks[trackIndex];
		double keyframeTime = static_cast<double>(pos.x()) / 100.0; // Assuming 100 pixels per second

		for (QMap<double, AnimationKeyframe>::const_iterator keyframe = track->keyframes().begin(); keyframe != track->keyframes().end(); ++keyframe)
		{
			double keyframeX = keyframe.key() * 100;
			if (std::abs(keyframeX - pos.x()) < keyframeWidth / 2)
			{
				time = keyframe.key();
				return track;
			}
		}
	}

	return nullptr;
}

void AnimationTimelineEditor::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		// Find the keyframe under the mouse cursor
		double time;
		AnimationTrack *track = keyframeAt(event->pos(), time);
		if (track)
		{
			ptrdiff_t keyframeId = track->keyframes()[time].Id;
			if (!m_KeyframeSelection.contains(keyframeId))
			{
				m_KeyframeSelection.clear();
				m_KeyframeSelection.insert(keyframeId);
				emit keyframeSelectionChanged();
			}
			update();
		}
	}
}

void AnimationTimelineEditor::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::LeftButton && !m_KeyframeSelection.isEmpty())
	{
		double newTime = static_cast<double>(event->pos().x()) / 100.0; // Assuming 100 pixels per second

		for (const auto &track : m_AnimationTracks)
		{
			for (QMap<double, AnimationKeyframe>::const_iterator keyframe = track->keyframes().begin(); keyframe != track->keyframes().end(); ++keyframe)
			{
				if (m_KeyframeSelection.contains(keyframe.value().Id))
				{
					double oldTime = keyframe.key();
					track->moveKeyframe(oldTime, newTime);
					emit animationTrackChanged(track);
					update();
					break;
				}
			}
		}
	}
}

void AnimationTimelineEditor::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton && !m_KeyframeSelection.isEmpty())
	{
		// Handle the end of dragging the keyframe selection
		update();
	}
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
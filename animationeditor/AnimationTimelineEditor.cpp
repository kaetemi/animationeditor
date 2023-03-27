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

#include <QApplication>
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
	setMouseTracking(true);
	// setFocusPolicy(Qt::StrongFocus);
	qApp->installEventFilter(this);
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

QRect AnimationTimelineEditor::rowsRect()
{
	// Find tree view if we haven't already
	if (!m_TreeWidget)
	{
		for (AnimationTrack *track : m_AnimationTracks)
		{
			if (track->m_TreeWidgetItem && track->m_TreeWidgetItem->treeWidget())
			{
				m_TreeWidget = track->m_TreeWidgetItem->treeWidget();
				break;
			}
		}
	}

	QRect rect = QRect(0, 0, width(), height());

	if (m_TreeWidget)
	{
		int outlineSize = m_TreeWidget->frameWidth();
		rect.adjust(outlineSize, outlineSize, -outlineSize, -outlineSize);
	}

	return rect;
}

void AnimationTimelineEditor::paintEditorBackground(QPainter &painter)
{
	// Paint background
	QRect bgRect = rowsRect();
	QBrush baseBrush = palette().brush(QPalette::Base);
	painter.fillRect(bgRect, baseBrush);

	// Draw frame using QStyle
	QStyleOptionFrame frameOption;
	frameOption.initFrom(this);
	frameOption.rect = QRect(0, 0, width(), height());
	frameOption.frameShape = m_TreeWidget ? m_TreeWidget->frameShape() : QFrame::StyledPanel;
	frameOption.lineWidth = m_TreeWidget ? m_TreeWidget->frameWidth() : 1; // Set the frame width
	frameOption.midLineWidth = m_TreeWidget ? m_TreeWidget->midLineWidth() : 0; // No mid-line width
	frameOption.state |= QStyle::State_Sunken; // Optional: for a sunken frame appearance
	frameOption.features = QStyleOptionFrame::Flat;
	style()->drawControl(QStyle::CE_ShapedFrame, &frameOption, &painter, this);
}

void AnimationTimelineEditor::paintKeyframe(QPainter &painter, const QRect &keyframeRect, bool isSelected, bool isHovered, bool isClicked)
{
	QBrush keyframeBrush = palette().brush(QPalette::Window);
	keyframeBrush.setColor(keyframeBrush.color().lighter(200));
	QBrush keyframeBrushSelected = palette().brush(QPalette::Highlight);

	QBrush brush = isSelected ? keyframeBrushSelected : keyframeBrush;

	if (isHovered)
		brush.setColor(brush.color().lighter(125));

	painter.fillRect(keyframeRect, brush);
}

void AnimationTimelineEditor::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter painter(this);
	QRect rect = rowsRect();

	// Draw the background
	paintEditorBackground(painter);

	// Draw the track backgrounds
	int lineWidth = (m_TreeWidget ? m_TreeWidget->frameWidth() : 1);
	for (int i = 0; i < m_AnimationTracks.size(); ++i)
	{
		AnimationTrack *track = m_AnimationTracks[i];
		QRect trackRect = visualTrackRect(track);
		if (trackRect.isEmpty())
			continue;
		trackRect = QRect(rect.x(), trackRect.y() + rect.y() + lineWidth, rect.width(), trackRect.height() - (lineWidth * 2));

		// Draw the track background
		QBrush trackBackgroundBrush = palette().brush(QPalette::AlternateBase);
		if (i & 1)
			trackBackgroundBrush.setColor(trackBackgroundBrush.color().lighter(105));
		else
			trackBackgroundBrush.setColor(trackBackgroundBrush.color().darker(105));
		painter.fillRect(trackRect, trackBackgroundBrush);

		// Draw keyframes
		for (QMap<double, AnimationKeyframe>::const_iterator keyframe = track->keyframes().begin(); keyframe != track->keyframes().end(); ++keyframe)
		{
			int keyframeX = timeToX(keyframe.key());
			QRect keyframeRect(keyframeX - 4, trackRect.y() + (trackRect.height() - 8) / 2, 8, 8);
			bool isSelected = m_SelectedKeyframes.contains(keyframe.value().Id);
			bool isHovered = (keyframe.value().Id == m_HoverKeyframe);
			paintKeyframe(painter, keyframeRect, isSelected, isHovered, false);
		}
	}

	if (!m_SelectionStart.isNull())
	{
		QRect selectionRect = QRect(m_SelectionStart, m_MouseMovePosition).normalized();
		QStyleOptionRubberBand rubberBandOption;
		rubberBandOption.initFrom(this);
		rubberBandOption.rect = selectionRect;
		style()->drawControl(QStyle::CE_RubberBand, &rubberBandOption, &painter, this);
	}
}

QRect AnimationTimelineEditor::keyframeRect(AnimationTrack *track, double time)
{
	QRect rect = rowsRect();
	QRect trackRect = visualTrackRect(track);
	trackRect.setY(trackRect.y() + rect.y());
	trackRect.setLeft(rect.left());
	trackRect.setRight(rect.right());
	int keyframeX = timeToX(time);
	int centerY = trackRect.y() + (trackRect.height() - 8) / 2;
	QRect keyframeRect(keyframeX - 4, centerY, 8, 8);
	return keyframeRect;
}

void AnimationTimelineEditor::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_TrackMoveStart = event->pos();

		// Backup all tracks before modifying keyframes
		m_OriginalAnimationTracks.clear();
		for (const AnimationTrack *track : m_AnimationTracks)
		{
			m_OriginalAnimationTracks.append(track->keyframes());
		}

		bool ctrlHeld = event->modifiers() & Qt::ControlModifier;
		bool clickedKeyframe = false;

		// Find the clicked keyframe
		for (AnimationTrack *track : m_AnimationTracks)
		{
			QRect trackRect = visualTrackRect(track);
			if (trackRect.contains(event->pos()))
			{
				for (QMap<double, AnimationKeyframe>::const_iterator keyframe = track->keyframes().begin(); keyframe != track->keyframes().end(); ++keyframe)
				{
					QRect keyframeRect = this->keyframeRect(track, keyframe.key());
					if (keyframeRect.contains(event->pos()))
					{
						if (ctrlHeld)
						{
							// If the keyframe is already selected, deselect it
							if (m_SelectedKeyframes.contains(keyframe.value().Id))
							{
								m_SelectedKeyframes.remove(keyframe.value().Id);
							}
							else
							{
								// Add the keyframe to the selection
								m_SelectedKeyframes.insert(keyframe.value().Id);
							}
						}
						else
						{
							// If the keyframe is not already selected, clear the selection and select the new keyframe
							if (!m_SelectedKeyframes.contains(keyframe.value().Id))
							{
								m_SelectedKeyframes = { keyframe.value().Id };
							}
						}
						m_HoverKeyframe = -1;
						clickedKeyframe = true;
						emit selectionChanged(m_SelectedKeyframes);
						break;
					}
				}
			}
		}

		if (!clickedKeyframe)
		{
			m_SelectedKeyframesBackup = QList<ptrdiff_t>(m_SelectedKeyframes.begin(), m_SelectedKeyframes.end());
			if (!ctrlHeld)
			{
				m_SelectedKeyframes.clear();
			}
			m_SelectionStart = event->pos();
			emit selectionChanged(m_SelectedKeyframes);
		}
		else
		{
			m_SelectionStart = QPoint();
		}
	}

	if (event->button() == Qt::RightButton)
	{
		if (!m_SelectionStart.isNull())
		{
			// Abort rectangle selection on right click
			m_SelectedKeyframes.clear();
			for (int i = 0; i < m_SelectedKeyframesBackup.size(); i++)
				m_SelectedKeyframes.insert(m_SelectedKeyframesBackup[i]);
			emit selectionChanged(m_SelectedKeyframes);
		}
	}

	update();
}

void AnimationTimelineEditor::updateMouseSelection(bool ctrlHeld)
{
	if (!m_SelectionStart.isNull())
	{
		QRect selectionRect = QRect(m_SelectionStart, m_MouseMovePosition).normalized();
		QSet<ptrdiff_t> newSelectedKeyframes;

		for (AnimationTrack *track : m_AnimationTracks)
		{
			QRect trackRect = visualTrackRect(track);
			for (QMap<double, AnimationKeyframe>::const_iterator keyframe = track->keyframes().begin(); keyframe != track->keyframes().end(); ++keyframe)
			{
				QRect keyframeRect = this->keyframeRect(track, keyframe.key());
				if (selectionRect.intersects(keyframeRect))
				{
					newSelectedKeyframes.insert(keyframe.value().Id);
				}
			}
		}

		m_SelectedKeyframes = newSelectedKeyframes;
		if (ctrlHeld)
		{
			for (int i = 0; i < m_SelectedKeyframesBackup.size(); i++)
				m_SelectedKeyframes.insert(m_SelectedKeyframesBackup[i]);
		}

		emit selectionChanged(m_SelectedKeyframes);
		update();
	}
}

void AnimationTimelineEditor::mouseMoveEvent(QMouseEvent *event)
{
	m_MouseMovePosition = event->pos();

	if (!m_SelectedKeyframes.isEmpty() && (event->buttons() & Qt::LeftButton) && m_SelectionStart.isNull())
	{
		double timeDelta = xToTime(event->pos().x()) - xToTime(m_TrackMoveStart.x());

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

		m_HoverKeyframe = -1;
		update();
	}
	else if (m_MouseHover)
	{
		updateMouseHover(event->pos());
	}

	if (event->buttons() & Qt::LeftButton)
	{
		bool ctrlHeld = event->modifiers() & Qt::ControlModifier;
		updateMouseSelection(ctrlHeld);
	}
}

void AnimationTimelineEditor::enterEvent(QEnterEvent *event)
{
	Q_UNUSED(event);
	m_MouseHover = true;
}

void AnimationTimelineEditor::leaveEvent(QEvent *event)
{
	Q_UNUSED(event);
	m_MouseHover = false;
}

void AnimationTimelineEditor::updateMouseHover(const QPoint &pos)
{
	// Find the hovered keyframe
	QRect rect = rowsRect();
	ptrdiff_t hoverKeyframe = -1;
	for (AnimationTrack *track : m_AnimationTracks)
	{
		QRect trackRect = visualTrackRect(track);
		trackRect.setY(trackRect.y() + rect.y());
		trackRect.setLeft(rect.left());
		trackRect.setRight(rect.right());
		if (trackRect.contains(pos))
		{
			for (QMap<double, AnimationKeyframe>::const_iterator keyframe = track->keyframes().begin(); keyframe != track->keyframes().end(); ++keyframe)
			{
				int keyframeX = timeToX(keyframe.key());
				QRect keyframeRect(keyframeX - 4, trackRect.y() + (trackRect.height() - 8) / 2, 8, 8);
				if (keyframeRect.contains(pos))
				{
					hoverKeyframe = keyframe.value().Id;
					break;
				}
			}
		}
	}
	if (m_HoverKeyframe != hoverKeyframe)
	{
		m_HoverKeyframe = hoverKeyframe;
		update();
	}
}

void AnimationTimelineEditor::mouseReleaseEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	m_TrackMoveStart = QPoint();
	m_SelectionStart = QPoint();
	m_OriginalAnimationTracks.clear();
	updateMouseHover(event->pos());
	update();
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

bool AnimationTimelineEditor::eventFilter(QObject *watched, QEvent *event)
{
	if (!m_SelectionStart.isNull())
	{
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			if (keyEvent->key() == Qt::Key_Control)
			{
				updateMouseSelection(true);
			}
		}
		else if (event->type() == QEvent::KeyRelease)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			if (keyEvent->key() == Qt::Key_Control)
			{
				updateMouseSelection(false);
			}
		}
	}
	return QWidget::eventFilter(watched, event);
}

/*
*
ChatGPT Dump:

- Enhance mouse interactions for a smoother user experience, such as multi-selecting keyframes,
moving multiple keyframes simultaneously, and snap-to-grid functionality.

*/

/* end of file */
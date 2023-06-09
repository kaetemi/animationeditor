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
#include <QPainterPath>
#include <QMenu>
#include <QAction>
#include <QScrollBar>

AnimationContextMenu::AnimationContextMenu(QWidget *parent) : QMenu(parent)
{
}

void AnimationContextMenu::hideEvent(QHideEvent *event)
{
	QMenu::hideEvent(event);
	emit menuClosed();
}

AnimationTimelineEditor::AnimationTimelineEditor(QWidget *parent)
    : QWidget(parent)
    , m_FromTime(0.0)
    , m_ToTime(10.0)
{
	setMouseTracking(true);
	// setFocusPolicy(Qt::StrongFocus);
	qApp->installEventFilter(this);
	createContextMenu();
}

AnimationTimelineEditor::~AnimationTimelineEditor()
{
}

void AnimationTimelineEditor::createContextMenu()
{
	AnimationContextMenu *contextMenu = new AnimationContextMenu(this);
	connect(contextMenu, &AnimationContextMenu::menuClosed, this, &AnimationTimelineEditor::onContextMenuClosed);

	m_ContextMenu = contextMenu;

	m_RemoveTrackAction = new QAction(tr("Remove Track"), this);
	connect(m_RemoveTrackAction, &QAction::triggered, this, &AnimationTimelineEditor::removeTrack);
	m_ContextMenu->addAction(m_RemoveTrackAction);

	m_AddKeyframeAction = new QAction(tr("Add Keyframe"), this);
	connect(m_AddKeyframeAction, &QAction::triggered, this, &AnimationTimelineEditor::addKeyframe);
	m_ContextMenu->addAction(m_AddKeyframeAction);

	m_RemoveKeyframeAction = new QAction(tr("Remove Keyframe"), this);
	connect(m_RemoveKeyframeAction, &QAction::triggered, this, &AnimationTimelineEditor::removeKeyframe);
	m_ContextMenu->addAction(m_RemoveKeyframeAction);
}

void AnimationTimelineEditor::contextMenuEvent(QContextMenuEvent *event)
{
	if (m_SkipContextMenu)
	{
		m_SkipContextMenu = false;
		return;
	}

	if (m_ContextMenuTrack)
	{
		QPoint pos = event->pos();
		QRect trackRect = visualTrackRectInWidgetSpace(m_ContextMenuTrack);
		if (!trackRect.contains(pos))
		{
			return;
		}
		bool ctrlHeld = event->modifiers() & Qt::ControlModifier;
		ptrdiff_t contextKeyframe = keyframeAtPosition(m_ContextMenuTrack, pos);
		if (contextKeyframe != -1)
		{
			if (ctrlHeld)
			{
				// Add the keyframe to the selection
				m_SelectedKeyframes.insert(contextKeyframe);
			}
			else
			{
				// If the keyframe is not already selected, clear the selection and select the new keyframe
				if (!m_SelectedKeyframes.contains(contextKeyframe))
				{
					m_SelectedKeyframes = { contextKeyframe };
				}
			}
			m_HoverKeyframe = -1;
			m_HoverTrack = nullptr;
			emit selectionChanged(m_SelectedKeyframes);
		}
	}

	m_ContextMenuOpen = true;
	m_ContextMousePosition = event->pos();
	m_ContextMenu->exec(event->globalPos());
}

void AnimationTimelineEditor::onContextMenuClosed()
{
	m_ContextMenuOpen = false;
	m_ContextMenuTrack = nullptr;
	update();
}

void AnimationTimelineEditor::removeTrack()
{
	if (m_ContextMenuTrack)
	{
		emit trackRemoved(m_ContextMenuTrack);
	}
}

void AnimationTimelineEditor::addKeyframe()
{
	// Implement your logic to add a keyframe to the current track
}

void AnimationTimelineEditor::removeKeyframe()
{
	// Implement your logic to remove the selected keyframe(s) from the current track
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

int AnimationTimelineEditor::timeToX(double time)
{
	QRect rect = rowsRect();
	return static_cast<int>(round((time - m_FromTime) / (m_ToTime - m_FromTime) * rect.width())) + rect.x();
}

double AnimationTimelineEditor::xToTime(int x)
{
	QRect rect = rowsRect();
	return m_FromTime + (static_cast<double>(x -  rect.x()) / rect.width()) * (m_ToTime - m_FromTime);
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

QRect AnimationTimelineEditor::visualTrackRectInWidgetSpace(AnimationTrack *track)
{
	QRect rect = rowsRect();
	QRect trackRect = visualTrackRect(track);

	if (trackRect.isEmpty())
		return QRect();

	return QRect(rect.x(), rect.y() + trackRect.y(), rect.width(), trackRect.height());
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
				connect(m_TreeWidget->verticalScrollBar(), &QScrollBar::valueChanged, this, static_cast<void (AnimationTimelineEditor::*)()>(&AnimationTimelineEditor::update));
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

void AnimationTimelineEditor::paintKeyframe(QPainter &painter, const QRect &rect, bool selected, bool hover, bool active)
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

void AnimationTimelineEditor::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter painter(this);

	// Draw the background
	paintEditorBackground(painter);

	// Scissor to frame
	QRect rect = rowsRect();
	painter.setClipRect(rect);

	// Draw the track backgrounds
	int lineWidth = (m_TreeWidget ? m_TreeWidget->frameWidth() : 1);
	for (int i = 0; i < m_AnimationTracks.size(); ++i)
	{
		AnimationTrack *track = m_AnimationTracks[i];
		QRect trackRect = visualTrackRectInWidgetSpace(track);
		if (trackRect.isEmpty())
			continue;
		trackRect = QRect(trackRect.x(), trackRect.y() + lineWidth, trackRect.width(), trackRect.height() - (lineWidth * 2));

		// Draw the track background
		QBrush trackBackgroundBrush = palette().brush(QPalette::AlternateBase);
		if (track == m_ContextMenuTrack && (track == m_CurrentHoverTrack || m_ContextMenuOpen))
			trackBackgroundBrush = palette().brush(QPalette::Highlight);
		else if (track == m_HoverTrack)
			trackBackgroundBrush.setColor(trackBackgroundBrush.color().lighter(110));
		else if (i & 1)
			trackBackgroundBrush.setColor(trackBackgroundBrush.color().lighter(105));
		else
			trackBackgroundBrush.setColor(trackBackgroundBrush.color().darker(105));
		painter.fillRect(trackRect, trackBackgroundBrush);

		QPen separatorPen(trackBackgroundBrush.color().lighter(105));
		separatorPen.setWidthF(1.0);
		painter.setPen(separatorPen);
		painter.drawLine(trackRect.left(), trackRect.y() - 1, trackRect.right(), trackRect.y() - 1);

		// Draw keyframes
		for (AnimationTrack::KeyframeMap::const_iterator keyframe = track->keyframes().begin(); keyframe != track->keyframes().end(); ++keyframe)
		{
			QRect keyframeRect = this->keyframeRect(track, keyframe.key());
			bool isSelected = m_SelectedKeyframes.contains(keyframe.value().Id);
			bool isHovered = (keyframe.value().Id == m_HoverKeyframe);
			bool isPressed = (keyframe.value().Id == m_PressedKeyframe) || ((keyframe.value().Id == m_CurrentHoverKeyframe) && keyframe.value().Id == m_RightPressedKeyframe);
			paintKeyframe(painter, keyframeRect, isSelected, isHovered, isPressed);
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
	QRect trackRect = visualTrackRectInWidgetSpace(track);
	int keyframeX = timeToX(time);
	int keyframeWidth = trackRect.height(); // Set the keyframe width to match the track height
	QRect keyframeRect(keyframeX - keyframeWidth / 2, trackRect.y(), keyframeWidth, trackRect.height());
	return keyframeRect;
}

AnimationTrack *AnimationTimelineEditor::trackAtPosition(const QPoint &pos)
{
	for (AnimationTrack *track : m_AnimationTracks)
	{
		QRect trackRect = visualTrackRectInWidgetSpace(track);
		if (trackRect.contains(pos))
		{
			return track;
		}
	}
	return nullptr;
}

ptrdiff_t AnimationTimelineEditor::keyframeAtPosition(AnimationTrack *track, const QPoint &pos)
{
	if (!track)
		return -1;
	const AnimationTrack::KeyframeMap &keyframes = track->keyframes();
	for (AnimationTrack::KeyframeMap::const_iterator it = keyframes.constEnd(); it != keyframes.constBegin();)
	{
		--it;
		QRect keyframeRect = this->keyframeRect(track, it.key());
		if (keyframeRect.contains(pos))
		{
			return it.value().Id;
		}
	}
	return -1;
}

void AnimationTimelineEditor::mousePressEvent(QMouseEvent *event)
{
	m_SkipContextMenu = false;
	m_MouseMovePosition = event->pos();

	if (event->button() == Qt::LeftButton)
	{
		// Backup all tracks before modifying keyframes
		m_BackupAnimationTracks.clear();
		for (const AnimationTrack *track : m_AnimationTracks)
		{
			m_BackupAnimationTracks.append(track->keyframes());
		}

		bool ctrlHeld = event->modifiers() & Qt::ControlModifier;
		bool clickedKeyframe = false;

		// Find the clicked keyframe
		AnimationTrack *clickedTrack = trackAtPosition(event->pos());
		if (clickedTrack)
		{
			if (clickedTrack)
			{
				// Find the clicked keyframe
				ptrdiff_t clickedKeyframeId = keyframeAtPosition(clickedTrack, event->pos());

				if (clickedKeyframeId != -1)
				{
					if (ctrlHeld)
					{
						// If the keyframe is already selected, deselect it
						if (m_SelectedKeyframes.contains(clickedKeyframeId))
						{
							m_SelectedKeyframes.remove(clickedKeyframeId);
						}
						else
						{
							// Add the keyframe to the selection
							m_SelectedKeyframes.insert(clickedKeyframeId);
						}
					}
					else
					{
						// If the keyframe is not already selected, clear the selection and select the new keyframe
						if (!m_SelectedKeyframes.contains(clickedKeyframeId))
						{
							m_SelectedKeyframes = { clickedKeyframeId };
						}
					}
					m_HoverKeyframe = -1;
					m_HoverTrack = nullptr;
					m_PressedKeyframe = clickedKeyframeId;
					clickedKeyframe = true;
					emit selectionChanged(m_SelectedKeyframes);
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
			m_TrackMoveStart = QPoint();
			m_SelectionStart = event->pos();
			m_HoverTrack = nullptr;
			emit selectionChanged(m_SelectedKeyframes);
		}
		else
		{
			m_TrackMoveStart = event->pos();
			m_SelectionStart = QPoint();
		}
	}

	if (event->button() == Qt::RightButton)
	{
		if (m_SelectionStart.isNull() && m_TrackMoveStart.isNull())
		{
			// QRect rect = rowsRect();
			QPoint pos = event->pos();
			AnimationTrack *currentTrack = trackAtPosition(pos);
			ptrdiff_t currentKeyframe = -1;
			if (currentTrack)
			{
				currentKeyframe = keyframeAtPosition(currentTrack, pos);
			}
			m_RightPressedKeyframe = currentKeyframe;
			m_CurrentHoverKeyframe = currentKeyframe;
			m_ContextMenuTrack = currentTrack;
			m_CurrentHoverTrack = currentTrack;
		}

		if (!m_SelectionStart.isNull())
		{
			// Abort rectangle selection on right click
			m_SkipContextMenu = true;
			m_SelectedKeyframes.clear();
			for (int i = 0; i < m_SelectedKeyframesBackup.size(); i++)
				m_SelectedKeyframes.insert(m_SelectedKeyframesBackup[i]);
			emit selectionChanged(m_SelectedKeyframes);
		}
		if (!m_TrackMoveStart.isNull())
		{
			// Abort track move on right click
			m_SkipContextMenu = true;
			for (int i = 0; i < m_AnimationTracks.size(); ++i)
			{
				AnimationTrack *track = m_AnimationTracks[i];
				AnimationTrack::KeyframeMap &originalKeyframes = m_BackupAnimationTracks[i];
				track->setKeyframes(originalKeyframes);
				emit trackChanged(track);
			}
		}
	}

	mouseMoveEvent(event);
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
			QRect trackRect = visualTrackRectInWidgetSpace(track);
			for (AnimationTrack::KeyframeMap::const_iterator keyframe = track->keyframes().begin(); keyframe != track->keyframes().end(); ++keyframe)
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

	if (!m_SelectedKeyframes.isEmpty() && (event->buttons() & Qt::LeftButton) && !m_TrackMoveStart.isNull())
	{
		double timeDelta = xToTime(event->pos().x()) - xToTime(m_TrackMoveStart.x());

		for (int i = 0; i < m_AnimationTracks.size(); ++i)
		{
			AnimationTrack *track = m_AnimationTracks[i];
			AnimationTrack::KeyframeMap &originalKeyframes = m_BackupAnimationTracks[i];

			// Restore the original tracks before modifying keyframes
			track->setKeyframes(originalKeyframes);
			bool changed = false;

			for (AnimationTrack::KeyframeMap::const_iterator keyframe = originalKeyframes.begin(); keyframe != originalKeyframes.end(); ++keyframe)
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
		m_HoverTrack = nullptr;
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
	m_HoverKeyframe = -1;
	m_HoverTrack = nullptr;
	update();
}

void AnimationTimelineEditor::updateMouseHover(const QPoint &pos)
{
	// Find the hovered keyframe
	// QRect rect = rowsRect();
	ptrdiff_t hoverKeyframe = -1;
	AnimationTrack *hoverTrack = trackAtPosition(pos);
	if (hoverTrack)
	{
		hoverKeyframe = keyframeAtPosition(hoverTrack, pos);
	}
	if (m_HoverKeyframe != hoverKeyframe || (m_SelectionStart.isNull() && m_HoverTrack != hoverTrack))
	{
		m_HoverKeyframe = hoverKeyframe;
		m_CurrentHoverKeyframe = hoverKeyframe;
		if (m_SelectionStart.isNull())
		{
			m_HoverTrack = hoverTrack;
			m_CurrentHoverTrack = m_HoverTrack;
		}
		update();
	}
}

void AnimationTimelineEditor::mouseReleaseEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	m_SelectionStart = QPoint();
	m_TrackMoveStart = QPoint();
	m_PressedKeyframe = -1;
	m_RightPressedKeyframe = -1;
	m_BackupAnimationTracks.clear();
	// updateMouseHover(event->pos());
	mouseMoveEvent(event);
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
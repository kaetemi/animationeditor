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
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QStyleOption>

#include "AnimationTimelineEditor.h" // AnimationContextMenu

AnimationCurveEditor::AnimationCurveEditor(QWidget *parent, QTreeWidget *dimensionalReference)
    : QWidget(parent)
    , m_DimensionalReference(dimensionalReference)
{
	setMouseTracking(true);
	// setFocusPolicy(Qt::StrongFocus);
	qApp->installEventFilter(this);
	recalculateGridInverval();
	createContextMenu();
}

AnimationCurveEditor::~AnimationCurveEditor()
{
}

void AnimationCurveEditor::createContextMenu()
{
	AnimationContextMenu *contextMenu = new AnimationContextMenu(this);
	connect(contextMenu, &AnimationContextMenu::menuClosed, this, &AnimationCurveEditor::onContextMenuClosed);

	m_ContextMenu = contextMenu;

	m_RemoveTrackAction = new QAction(tr("Remove Track"), this);
	connect(m_RemoveTrackAction, &QAction::triggered, this, &AnimationCurveEditor::removeTrack);
	m_ContextMenu->addAction(m_RemoveTrackAction);

	m_AddKeyframeAction = new QAction(tr("Add Keyframe"), this);
	connect(m_AddKeyframeAction, &QAction::triggered, this, &AnimationCurveEditor::addKeyframe);
	m_ContextMenu->addAction(m_AddKeyframeAction);

	m_RemoveKeyframeAction = new QAction(tr("Remove Keyframe"), this);
	connect(m_RemoveKeyframeAction, &QAction::triggered, this, &AnimationCurveEditor::removeKeyframe);
	m_ContextMenu->addAction(m_RemoveKeyframeAction);
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
	m_SelectedKeyframes = selection;
	update();
}

QSet<ptrdiff_t> AnimationCurveEditor::keyframeSelection() const
{
	return m_SelectedKeyframes;
}

void AnimationCurveEditor::updateMousePosition(const QPoint &pos, bool ctrlHeld)
{
	m_MouseMovePosition = pos;
	bool wantUpdate = false;

	// Update hover
	AnimationTrack *track = nullptr;
	{
		ptrdiff_t keyframe = keyframeAtPosition(pos, &track);

		if (keyframe != m_HoverKeyframe)
		{
			m_HoverKeyframe = keyframe;
			if (keyframe != -1)
			{
				m_HoverLeftInterpolationHandle = -1;
				m_HoverRightInterpolationHandle = -1;
			}
			wantUpdate = true;
		}

		if (keyframe == -1)
		{
			ptrdiff_t rightHandle = rightHandleAtPosition(pos, &track);

			if (rightHandle != m_HoverRightInterpolationHandle)
			{
				m_HoverRightInterpolationHandle = rightHandle;
				if (rightHandle != -1)
				{
					m_HoverKeyframe = -1;
					m_HoverLeftInterpolationHandle = -1;
					;
				}
				wantUpdate = true;
			}

			if (rightHandle == -1)
			{
				ptrdiff_t leftHandle = leftHandleAtPosition(pos, &track);

				if (leftHandle != m_HoverLeftInterpolationHandle)
				{
					m_HoverLeftInterpolationHandle = leftHandle;
					if (leftHandle != -1)
					{
						m_HoverKeyframe = -1;
						m_HoverRightInterpolationHandle = -1;
					}
					wantUpdate = true;
				}
			}
		}
	}
	if (track != m_HoverTrack)
	{
		m_HoverTrack = track;
		wantUpdate = true;
	}

	if (m_InteractionState != InteractionState::None)
	{
		if (m_InteractionState == InteractionState::MoveOnly || m_InteractionState == InteractionState::SelectMove
		    || m_InteractionState == InteractionState::MoveLeftHandleOnly || m_InteractionState == InteractionState::SelectMoveLeftHandle
		    || m_InteractionState == InteractionState::MoveRightHandleOnly || m_InteractionState == InteractionState::SelectMoveRightHandle)
		{
			if (!m_SelectMoveTresholdPassed && (m_InteractionState == InteractionState::SelectMove || m_InteractionState == InteractionState::SelectMoveLeftHandle || m_InteractionState == InteractionState::SelectMoveRightHandle))
			{
				int dx = pos.x() - m_MouseLeftPressPosition.x();
				int dy = pos.y() - m_MouseLeftPressPosition.y();
				if ((dx * dx) + (dy * dy) > 9)
				{
					m_SelectMoveTresholdPassed = true;
				}
			}

			if (m_SelectMoveTresholdPassed || m_InteractionState == InteractionState::MoveOnly || m_InteractionState == InteractionState::MoveLeftHandleOnly || m_InteractionState == InteractionState::MoveRightHandleOnly)
			{
				// Calculate the delta movement in time and value
				double timeDelta = timeAtX(pos.x()) - timeAtX(m_MouseLeftPressPosition.x());
				double valueDelta = -(pos.y() - m_MouseLeftPressPosition.y()) / m_VerticalPixelPerValue;

				// Move the selected keyframes or handles relative to the backup position
				for (int i = 0; i < m_AnimationTracks.size(); ++i)
				{
					AnimationTrack *track = m_AnimationTracks[i];
					AnimationTrack::KeyframeMap &originalKeyframes = m_BackupAnimationTracks[i];

					// Restore the original tracks before modifying keyframes
					track->m_Keyframes = originalKeyframes;
					bool changed = false;

					for (AnimationTrack::KeyframeMap::const_iterator it = originalKeyframes.begin(); it != originalKeyframes.end(); ++it)
					{
						if (m_InteractionState == InteractionState::MoveOnly || m_InteractionState == InteractionState::SelectMove)
						{
							if (m_SelectedKeyframes.contains(it.value().Id))
							{
								double newTime = it.key() + timeDelta;
								double newValue = it.value().Value + valueDelta;
								AnimationKeyframe keyframe = track->m_Keyframes.value(it.key());
								keyframe.Value = newValue;
								track->m_Keyframes.remove(it.key());
								track->m_Keyframes.insert(newTime, keyframe);
								changed = true;
							}
						}
						else if (m_InteractionState == InteractionState::MoveLeftHandleOnly || m_InteractionState == InteractionState::SelectMoveLeftHandle)
						{
							if (m_SelectedLeftInterpolationHandles.contains(it.value().Id))
							{
								AnimationKeyframe &keyframe = track->m_Keyframes[it.key()];
								keyframe.Interpolation.Bezier.InTangentX += timeDelta;
								keyframe.Interpolation.Bezier.InTangentY += valueDelta;
							}
						}
						else if (m_InteractionState == InteractionState::MoveRightHandleOnly || m_InteractionState == InteractionState::SelectMoveRightHandle)
						{
							if (m_SelectedRightInterpolationHandles.contains(it.value().Id))
							{
								AnimationKeyframe &keyframe = track->m_Keyframes[it.key()];
								keyframe.Interpolation.Bezier.OutTangentX += timeDelta;
								keyframe.Interpolation.Bezier.OutTangentY += valueDelta;
								changed = true;
							}
						}
					}

					if (changed)
					{
						emit trackChanged(track);
					}
				}
			}
		}
		else if (m_InteractionState == InteractionState::MultiSelect)
		{
			// Update the selection rectangle
			updateMouseSelection(ctrlHeld);
		}

		wantUpdate = true;
	}

	if (wantUpdate)
		update();
}

void AnimationCurveEditor::updateMouseSelection(bool ctrlHeld)
{
	QRect rect;
	rect.setTopLeft(m_MouseLeftPressPosition);
	rect.setBottomRight(m_MouseMovePosition);
	m_SelectedKeyframes = keyframesInRect(rect.normalized());
	if (ctrlHeld)
	{
		m_SelectedKeyframes.unite(m_BackupSelectedKeyframes);
	}
}

void AnimationCurveEditor::restoreAnimationTracks()
{
	for (int i = 0; i < m_AnimationTracks.size(); ++i)
	{
		AnimationTrack *track = m_AnimationTracks[i];
		AnimationTrack::KeyframeMap &originalKeyframes = m_BackupAnimationTracks[i];
		track->setKeyframes(originalKeyframes);
		emit trackChanged(track);
	}
}

void AnimationCurveEditor::mousePressEvent(QMouseEvent *event)
{
	m_SkipContextMenu = false;
	QPoint pos = event->pos();
	bool ctrlHeld = event->modifiers() & Qt::ControlModifier;
	updateMousePosition(pos, ctrlHeld);

	if (event->button() == Qt::LeftButton)
	{
		m_MouseLeftPressPosition = pos;
		m_MouseLeftPressTimeValue = timeValueAtXY(pos);
		ptrdiff_t keyframe = m_HoverKeyframe;
		ptrdiff_t leftHandle = m_HoverLeftInterpolationHandle;
		ptrdiff_t rightHandle = m_HoverRightInterpolationHandle;
		m_ActiveKeyframe = keyframe;
		m_ActiveLeftInterpolationHandle = leftHandle;
		m_ActiveRightInterpolationHandle = rightHandle;
		m_ActiveTrack = m_HoverTrack;
		if (keyframe != -1 || leftHandle != -1 || rightHandle != -1)
		{
			// Backup all tracks before modifying keyframes
			m_BackupAnimationTracks.clear();
			for (const AnimationTrack *track : m_AnimationTracks)
			{
				m_BackupAnimationTracks.append(track->keyframes());
			}
		}
		if (keyframe != -1)
		{
			if (m_SelectedKeyframes.contains(keyframe))
			{
				m_InteractionState = InteractionState::MoveOnly;
			}
			else
			{
				m_BackupSelectedKeyframes = m_SelectedKeyframes;
				m_BackupSelectedLeftInterpolationHandles = m_SelectedLeftInterpolationHandles;
				m_BackupSelectedRightInterpolationHandles = m_SelectedRightInterpolationHandles;
				if (!ctrlHeld)
				{
					m_SelectedKeyframes.clear();
					m_SelectedLeftInterpolationHandles.clear();
					m_SelectedRightInterpolationHandles.clear();
				}
				m_SelectedKeyframes.insert(keyframe);
				m_InteractionState = InteractionState::SelectMove;
				m_SelectMoveTresholdPassed = false;
			}
		}
		else if (leftHandle != -1)
		{
			if (m_SelectedLeftInterpolationHandles.contains(leftHandle))
			{
				m_InteractionState = InteractionState::MoveLeftHandleOnly;
			}
			else
			{
				m_BackupSelectedLeftInterpolationHandles = m_SelectedLeftInterpolationHandles;
				m_BackupSelectedRightInterpolationHandles = m_SelectedRightInterpolationHandles;
				if (!ctrlHeld)
				{
					m_SelectedLeftInterpolationHandles.clear();
					m_SelectedRightInterpolationHandles.clear();
				}
				m_SelectedLeftInterpolationHandles.insert(leftHandle);
				m_InteractionState = InteractionState::SelectMoveLeftHandle;
				m_SelectMoveTresholdPassed = false;
			}
		}
		else if (rightHandle != -1)
		{
			if (m_SelectedRightInterpolationHandles.contains(rightHandle))
			{
				m_InteractionState = InteractionState::MoveRightHandleOnly;
			}
			else
			{
				m_BackupSelectedLeftInterpolationHandles = m_SelectedLeftInterpolationHandles;
				m_BackupSelectedRightInterpolationHandles = m_SelectedRightInterpolationHandles;
				if (!ctrlHeld)
				{
					m_SelectedLeftInterpolationHandles.clear();
					m_SelectedRightInterpolationHandles.clear();
				}
				m_SelectedRightInterpolationHandles.insert(rightHandle);
				m_InteractionState = InteractionState::SelectMoveRightHandle;
				m_SelectMoveTresholdPassed = false;
			}
		}
		else
		{
			m_BackupSelectedKeyframes = m_SelectedKeyframes;
			m_BackupSelectedLeftInterpolationHandles = m_SelectedLeftInterpolationHandles;
			m_BackupSelectedRightInterpolationHandles = m_SelectedRightInterpolationHandles;
			m_SelectedKeyframes.clear();
			m_SelectedLeftInterpolationHandles.clear();
			m_SelectedRightInterpolationHandles.clear();
			m_InteractionState = InteractionState::MultiSelect;
			updateMouseSelection(ctrlHeld);
		}
	}

	if (event->button() == Qt::RightButton)
	{
		m_MouseRightPressPosition = pos;
		m_MouseRightPressTimeValue = timeValueAtXY(pos);
		ptrdiff_t keyframe = m_HoverKeyframe;
		switch (m_InteractionState)
		{
		case InteractionState::MoveOnly: {
			restoreAnimationTracks();
			m_SkipContextMenu = true;
			m_InteractionState = InteractionState::None;
			break;
		}
		case InteractionState::SelectMove: {
			if (m_SelectMoveTresholdPassed)
			{
				restoreAnimationTracks();
			}
			else
			{
				// Only abort selection if we didn't move, the abort is just to abort the movement!
				m_SelectedKeyframes = m_BackupSelectedKeyframes;
				m_SelectedLeftInterpolationHandles = m_BackupSelectedLeftInterpolationHandles;
				m_SelectedRightInterpolationHandles = m_BackupSelectedRightInterpolationHandles;
			}
			m_BackupSelectedKeyframes.clear();
			m_BackupSelectedLeftInterpolationHandles.clear();
			m_BackupSelectedRightInterpolationHandles.clear();
			m_SkipContextMenu = true;
			m_InteractionState = InteractionState::None;
			break;
		}
		case InteractionState::MoveLeftHandleOnly: {
			restoreAnimationTracks();
			m_SkipContextMenu = true;
			m_InteractionState = InteractionState::None;
			break;
		}
		case InteractionState::SelectMoveLeftHandle: {
			if (m_SelectMoveTresholdPassed)
			{
				restoreAnimationTracks();
			}
			else
			{
				m_SelectedLeftInterpolationHandles = m_BackupSelectedLeftInterpolationHandles;
				m_SelectedRightInterpolationHandles = m_BackupSelectedRightInterpolationHandles;
			}
			m_BackupSelectedLeftInterpolationHandles.clear();
			m_BackupSelectedRightInterpolationHandles.clear();
			m_SkipContextMenu = true;
			m_InteractionState = InteractionState::None;
			break;
		}
		case InteractionState::MoveRightHandleOnly: {
			restoreAnimationTracks();
			m_SkipContextMenu = true;
			m_InteractionState = InteractionState::None;
			break;
		}
		case InteractionState::SelectMoveRightHandle: {
			if (m_SelectMoveTresholdPassed)
			{
				restoreAnimationTracks();
			}
			else
			{
				m_SelectedLeftInterpolationHandles = m_BackupSelectedLeftInterpolationHandles;
				m_SelectedRightInterpolationHandles = m_BackupSelectedRightInterpolationHandles;
			}
			m_BackupSelectedLeftInterpolationHandles.clear();
			m_BackupSelectedRightInterpolationHandles.clear();
			m_SkipContextMenu = true;
			m_InteractionState = InteractionState::None;
			break;
		}
		case InteractionState::MultiSelect: {
			m_SelectedKeyframes = m_BackupSelectedKeyframes;
			m_SelectedLeftInterpolationHandles = m_BackupSelectedLeftInterpolationHandles;
			m_SelectedRightInterpolationHandles = m_BackupSelectedRightInterpolationHandles;
			m_BackupSelectedKeyframes.clear();
			m_BackupSelectedLeftInterpolationHandles.clear();
			m_BackupSelectedRightInterpolationHandles.clear();
			m_SkipContextMenu = true;
			m_InteractionState = InteractionState::None;
			break;
		}
		default: {
			m_ActiveKeyframe = m_HoverKeyframe;
			m_ActiveTrack = m_HoverTrack;
			break;
		}
		}
	}

	update();
}

void AnimationCurveEditor::mouseMoveEvent(QMouseEvent *event)
{
	updateMousePosition(event->pos(), event->modifiers() & Qt::ControlModifier);
}

void AnimationCurveEditor::mouseReleaseEvent(QMouseEvent *event)
{
	QPoint pos = event->pos();
	updateMousePosition(pos, event->modifiers() & Qt::ControlModifier);

	if (event->button() == Qt::LeftButton)
	{
		if (m_InteractionState == InteractionState::MoveOnly || m_InteractionState == InteractionState::SelectMove)
		{
			// Apply the move operation to selected keyframes
			m_BackupAnimationTracks.clear();
		}
		else if (m_InteractionState == InteractionState::MultiSelect)
		{
			// Finalize the multi-selection
			m_BackupSelectedKeyframes.clear();
			m_BackupSelectedLeftInterpolationHandles.clear();
			m_BackupSelectedRightInterpolationHandles.clear();
		}

		m_InteractionState = InteractionState::None;
		m_MouseLeftPressPosition = QPoint();
		m_MouseLeftPressTimeValue = QPointF();
		m_ActiveKeyframe = -1;
		m_ActiveTrack = nullptr;
	}

	if (event->button() == Qt::RightButton)
	{
		m_InteractionState = InteractionState::None;
		m_MouseRightPressPosition = QPoint();
		m_MouseRightPressTimeValue = QPointF();
		m_ActiveKeyframe = -1;
		m_ActiveTrack = nullptr;
	}

	update();
}

void AnimationCurveEditor::contextMenuEvent(QContextMenuEvent *event)
{
	if (m_SkipContextMenu)
	{
		m_SkipContextMenu = false;
		return;
	}

	// Show the context menu.
	// ...
}

void AnimationCurveEditor::wheelEvent(QWheelEvent *event)
{
	bool needRecalculate = false;

	double scrollAmount = event->angleDelta().y() / 8.0;
	double scaleFactor = 1 + scrollAmount / 120.0;

	// Change m_VerticalPixelPerValue when there are no key modifiers, or Control is held
	if (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::ControlModifier)
	{
		// Calculate the value at the mouse position
		double mouseY = event->position().y();
		double mouseValue = timeValueAtXY(QPoint(0, mouseY)).y();

		// Scale the vertical pixel per value around the mouse position
		double prevVerticalPixelPerValue = m_VerticalPixelPerValue;
		m_VerticalPixelPerValue *= scaleFactor;

		double newMouseY = keyframePointF(0, mouseValue).y();
		double yOffset = newMouseY - mouseY;

		m_VerticalCenterValue -= yOffset / m_VerticalPixelPerValue;

		needRecalculate = true;
	}
	// Change the from/to time range when the Shift key is held, or Control is held
	if (event->modifiers() == Qt::ShiftModifier || event->modifiers() == Qt::ControlModifier)
	{
		double timeRange = m_ToTime - m_FromTime;

		// Calculate the time at the mouse position
		double mouseTime = timeAtX(event->position().x());

		// Scale the time range around the mouse position
		double newTimeRange = timeRange / scaleFactor;
		double timeDiff = newTimeRange - timeRange;
		double timeDiffRatio = (mouseTime - m_FromTime) / timeRange;

		m_FromTime -= timeDiff * timeDiffRatio;
		m_ToTime += timeDiff * (1.0 - timeDiffRatio);
		emit rangeChanged(m_FromTime, m_ToTime);

		needRecalculate = true;
	}
	if (needRecalculate)
	{
		recalculateGridInverval(); // Redraw the widget
		if (!m_MouseLeftPressPosition.isNull() || !m_MouseLeftPressTimeValue.isNull())
		{
			m_MouseLeftPressPosition = keyframePoint(m_MouseLeftPressTimeValue.x(), m_MouseLeftPressTimeValue.y());
		}
		if (!m_MouseRightPressPosition.isNull() || !m_MouseRightPressTimeValue.isNull())
		{
			m_MouseRightPressPosition = keyframePoint(m_MouseRightPressTimeValue.x(), m_MouseRightPressTimeValue.y());
		}
		updateMousePosition(event->position().toPoint(), event->modifiers() & Qt::ControlModifier);
	}
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

QPointF AnimationCurveEditor::keyframePointF(double time, double value) const
{
	// Get the grid rectangle
	QRect grid = gridRect();

	// Calculate the horizontal position (x) within the grid rectangle
	double normalizedTime = (time - m_FromTime) / (m_ToTime - m_FromTime);
	double x = grid.left() + normalizedTime * grid.width();

	// Calculate the vertical position (y) within the grid rectangle
	double normalizedValue = (value - m_VerticalCenterValue) * m_VerticalPixelPerValue;
	double y = grid.center().y() - normalizedValue;

	return QPointF(x, y);
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

QPoint AnimationCurveEditor::keyframePointOffset(double time, double value) const
{
	// Get the grid rectangle
	QRect grid = gridRect();

	// Calculate the horizontal position (x) offset
	double normalizedTime = time / (m_ToTime - m_FromTime);
	double x = normalizedTime * grid.width();

	// Calculate the vertical position (y) offset
	double y = value * m_VerticalPixelPerValue;

	return QPoint(round(x), round(y));
}

double AnimationCurveEditor::timeAtX(double x) const
{
	QRect grid = gridRect();
	double xNormalized = (x - grid.left()) / static_cast<double>(grid.width());
	double time = m_FromTime + xNormalized * (m_ToTime - m_FromTime);
	return time;
}

double AnimationCurveEditor::timeAtX(int x) const
{
	QRect grid = gridRect();
	double xNormalized = (x - grid.left()) / static_cast<double>(grid.width());
	double time = m_FromTime + xNormalized * (m_ToTime - m_FromTime);
	return time;
}

QPointF AnimationCurveEditor::timeValueAtXY(const QPoint &pos) const
{
	QRect grid = gridRect();

	double xNormalized = (pos.x() - grid.left()) / static_cast<double>(grid.width());
	double time = m_FromTime + xNormalized * (m_ToTime - m_FromTime);

	double normalizedValue = grid.center().y() - pos.y();
	double value = (normalizedValue / m_VerticalPixelPerValue) + m_VerticalCenterValue;

	return QPointF(time, value);
}

ptrdiff_t AnimationCurveEditor::keyframeAtPosition(const QPoint &pos, AnimationTrack **trackRes) const
{
	if (pos.isNull())
		return -1;

	int keyframeHalfSize = 6;
	for (QList<AnimationTrack *>::const_iterator trackIt = m_AnimationTracks.end(); trackIt != m_AnimationTracks.begin();)
	{
		--trackIt;
		AnimationTrack *track = *trackIt;
		const AnimationTrack::KeyframeMap &keyframes = track->keyframes();
		if (keyframes.begin() != keyframes.end())
		{
			// Check if the pos time is between the first and last keyframe
			double firstTime = keyframes.begin().key();
			double lastTime = std::prev(keyframes.end()).key();
			if (timeAtX(pos.x() + keyframeHalfSize) >= firstTime && timeAtX(pos.x() - keyframeHalfSize) <= lastTime)
			{
				// Find the keyframe at the given position
				for (AnimationTrack::KeyframeMap::const_iterator it = keyframes.constEnd(); it != keyframes.constBegin();)
				{
					--it;
					QPoint keyframePos = keyframePoint(it.key(), it.value().Value);
					if (abs(keyframePos.x() - pos.x()) <= keyframeHalfSize && abs(keyframePos.y() - pos.y()) <= keyframeHalfSize)
					{
						if (trackRes)
							*trackRes = track;
						return it.value().Id;
					}
				}
			}
		}
	}
	if (trackRes)
		*trackRes = nullptr;
	return -1;
}

ptrdiff_t AnimationCurveEditor::leftHandleAtPosition(const QPoint &pos, AnimationTrack **trackRes) const
{
	if (pos.isNull())
		return -1;

	int handleHalfSize = 3;
	for (QList<AnimationTrack *>::const_iterator trackIt = m_AnimationTracks.end(); trackIt != m_AnimationTracks.begin();)
	{
		--trackIt;
		AnimationTrack *track = *trackIt;
		if (track->interpolationMethod() == AnimationInterpolation::Bezier)
		{
			const AnimationTrack::KeyframeMap &keyframes = track->keyframes();
			if (keyframes.begin() != keyframes.end())
			{
				// Check if the pos time is between the first and last keyframe (not entirely accurate on the rounding)
				// double firstTime = keyframes.begin().key() - keyframes.begin().value().Interpolation.Bezier.InTangentX;
				// double lastTime = std::prev(keyframes.end()).key() - std::prev(keyframes.end()).value().Interpolation.Bezier.OutTangentX;
				// if (timeAtX(pos.x() + handleHalfSize) >= firstTime && timeAtX(pos.x() - handleHalfSize) <= lastTime)
				{
					// Find the keyframe at the given position
					for (AnimationTrack::KeyframeMap::const_iterator it = keyframes.constEnd(); it != keyframes.constBegin();)
					{
						--it;
						QPoint keyframePos = keyframePoint(it.key(), it.value().Value);
						const double scale = 1.0;
						QPoint handleOffset = keyframePointOffset(
						    it.value().Interpolation.Bezier.InTangentX * scale,
						    -it.value().Interpolation.Bezier.InTangentY * scale);
						QPoint handlePoint = keyframePos + handleOffset;
						if (abs(handlePoint.x() - pos.x()) <= handleHalfSize && abs(handlePoint.y() - pos.y()) <= handleHalfSize)
						{
							if (trackRes)
								*trackRes = track;
							return it.value().Id;
						}
					}
				}
			}
		}
	}
	if (trackRes)
		*trackRes = nullptr;
	return -1;
}

ptrdiff_t AnimationCurveEditor::rightHandleAtPosition(const QPoint &pos, AnimationTrack **trackRes) const
{
	if (pos.isNull())
		return -1;

	int handleHalfSize = 3;
	for (QList<AnimationTrack *>::const_iterator trackIt = m_AnimationTracks.end(); trackIt != m_AnimationTracks.begin();)
	{
		--trackIt;
		AnimationTrack *track = *trackIt;
		if (track->interpolationMethod() == AnimationInterpolation::Bezier)
		{
			const AnimationTrack::KeyframeMap &keyframes = track->keyframes();
			if (keyframes.begin() != keyframes.end())
			{
				// Check if the pos time is between the first and last keyframe (not entirely accurate on the rounding)
				// double firstTime = keyframes.begin().key() - keyframes.begin().value().Interpolation.Bezier.InTangentX;
				// double lastTime = std::prev(keyframes.end()).key() + std::prev(keyframes.end()).value().Interpolation.Bezier.OutTangentX;
				// if (timeAtX(pos.x() + handleHalfSize) >= firstTime && timeAtX(pos.x() - handleHalfSize) <= lastTime)
				{
					// Find the keyframe at the given position
					for (AnimationTrack::KeyframeMap::const_iterator it = keyframes.constEnd(); it != keyframes.constBegin();)
					{
						--it;
						QPoint keyframePos = keyframePoint(it.key(), it.value().Value);
						const double scale = 1.0;
						QPoint handleOffset = keyframePointOffset(
						    it.value().Interpolation.Bezier.OutTangentX * scale,
						    -it.value().Interpolation.Bezier.OutTangentY * scale);
						QPoint handlePoint = keyframePos + handleOffset;
						if (abs(handlePoint.x() - pos.x()) <= handleHalfSize && abs(handlePoint.y() - pos.y()) <= handleHalfSize)
						{
							if (trackRes)
								*trackRes = track;
							return it.value().Id;
						}
					}
				}
			}
		}
	}
	if (trackRes)
		*trackRes = nullptr;
	return -1;
}

QSet<ptrdiff_t> AnimationCurveEditor::keyframesAtPosition(const QPoint &pos) const
{
	int keyframeHalfSize = 6;
	QSet<ptrdiff_t> res;
	for (AnimationTrack *track : m_AnimationTracks)
	{
		const AnimationTrack::KeyframeMap &keyframes = track->keyframes();
		if (keyframes.begin() != keyframes.end())
		{
			// Check if the pos time is between the first and last keyframe
			double firstTime = keyframes.begin().key();
			double lastTime = std::prev(keyframes.end()).key();
			if (timeAtX(pos.x() + keyframeHalfSize) >= firstTime && timeAtX(pos.x() - keyframeHalfSize) <= lastTime)
			{
				// Find the keyframe at the given position
				for (auto it = keyframes.begin(); it != keyframes.end(); ++it)
				{
					QPoint keyframePos = keyframePoint(it.key(), it.value().Value);
					if (abs(keyframePos.x() - pos.x()) <= keyframeHalfSize && abs(keyframePos.y() - pos.y()) <= keyframeHalfSize)
					{
						res.insert(it.value().Id);
					}
				}
			}
		}
	}
	return res;
}

QSet<ptrdiff_t> AnimationCurveEditor::keyframesInRect(const QRect &rect) const
{
	int keyframeHalfSize = 6;
	QRect expandedRect = rect.adjusted(-keyframeHalfSize, -keyframeHalfSize, keyframeHalfSize, keyframeHalfSize);
	QSet<ptrdiff_t> res;

	for (AnimationTrack *track : m_AnimationTracks)
	{
		const AnimationTrack::KeyframeMap &keyframes = track->keyframes();
		if (!keyframes.isEmpty())
		{
			double firstTime = keyframes.begin().key();
			double lastTime = std::prev(keyframes.end()).key();
			int firstTimeX = keyframePoint(firstTime, 0).x();
			int lastTimeX = keyframePoint(lastTime, 0).x();

			QRect trackTimeRect(firstTimeX, expandedRect.top(), lastTimeX - firstTimeX, expandedRect.height());

			if (expandedRect.intersects(trackTimeRect))
			{
				for (auto it = keyframes.begin(); it != keyframes.end(); ++it)
				{
					QPoint keyframePos = keyframePoint(it.key(), it.value().Value);
					if (expandedRect.contains(keyframePos))
					{
						res.insert(it.value().Id);
					}
				}
			}
		}
	}
	return res;
}

void AnimationCurveEditor::recalculateGridInverval()
{
	QRect grid = gridRect();

	double targetPixelsPerPrimary = 100.0; // Target pixels per primary
	double pixelsPerValue = m_VerticalPixelPerValue;
	double pixelsPerTime = grid.width() / (m_ToTime - m_FromTime);

	double primaryValueInterval = targetPixelsPerPrimary / pixelsPerValue;
	primaryValueInterval = pow(10, round(log10(primaryValueInterval)));
	if (abs(pixelsPerValue * primaryValueInterval - targetPixelsPerPrimary) > abs(pixelsPerValue * primaryValueInterval * 2.0 - targetPixelsPerPrimary))
		primaryValueInterval *= 2.0;
	else if (abs(pixelsPerValue * primaryValueInterval - targetPixelsPerPrimary) > abs(pixelsPerValue * primaryValueInterval * 0.5 - targetPixelsPerPrimary))
		primaryValueInterval *= 0.5;
	m_VerticalPrimaryValueInterval = primaryValueInterval;
	m_VerticalSecondaryValueInterval = primaryValueInterval * 0.1;

	double pixelsPerSecond = pixelsPerTime;
	double pixelsPerMinute = pixelsPerSecond * 60.0;
	double pixelsPerHour = pixelsPerSecond * 3600.0;
	double pixelsPerDay = pixelsPerSecond * 86400.0;

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

void AnimationCurveEditor::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	recalculateGridInverval();
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

void AnimationCurveEditor::paintInterpolationHandle(QPainter &painter, const QRect &rect, bool selected, bool hover, bool active)
{
	// Adjust the keyframe size
	QRect adjustedRect = rect.adjusted(1, 1, -1, -1);
	QRect shadowRect = adjustedRect.adjusted(-2, -2, 2, 2);
	QPointF top = QPointF((adjustedRect.left() + adjustedRect.right()) / 2.0, adjustedRect.top());
	QPointF bottom = QPointF((adjustedRect.left() + adjustedRect.right()) / 2.0, adjustedRect.bottom());

	QColor shadow = palette().color(QPalette::Shadow).darker(300);
	shadow.setAlpha(128);

	QBrush shadowBrush(shadow);

	QPainterPath shadowPath;
	shadowPath.addEllipse(shadowRect);

	// Create the gradient brush
	QLinearGradient gradient(top, bottom);

	QColor baseColor;
	QColor lightColor;
	QColor darkColor;
	if (selected)
	{
		baseColor = palette().color(QPalette::Highlight);
		baseColor.setHsl((baseColor.hue() + 180) % 360, baseColor.saturation(), baseColor.lightness());
		lightColor = baseColor.lighter(150);
		darkColor = baseColor.darker(125);
	}
	else
	{
		baseColor = lerp(palette().color(QPalette::Button), palette().color(QPalette::ButtonText), 0.3);
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
	path.addEllipse(adjustedRect);

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
	QRect grid = gridRect();

	QColor baseColor = palette().color(QPalette::Base);
	QPen secondaryLightPen = QPen(baseColor.lighter(110));
	QPen secondaryDarkPen = QPen(baseColor.darker(110));
	QPen primaryLightPen = QPen(baseColor.lighter(150));
	QPen primaryDarkPen = QPen(baseColor.darker(150));

	// Draw horizontal secondary grid lines
	double minValue = m_VerticalCenterValue - ((grid.height() / 2.0) / m_VerticalPixelPerValue);
	double maxValue = m_VerticalCenterValue + ((grid.height() / 2.0) / m_VerticalPixelPerValue);
	minValue = floor(minValue / m_VerticalPrimaryValueInterval) * m_VerticalPrimaryValueInterval;
	maxValue = ceil(maxValue / m_VerticalPrimaryValueInterval) * m_VerticalPrimaryValueInterval;

	for (double value = minValue; value <= maxValue; value += m_VerticalPrimaryValueInterval)
	{
		for (double secondaryValue = value + m_VerticalSecondaryValueInterval; secondaryValue <= (value + m_VerticalPrimaryValueInterval - (m_VerticalSecondaryValueInterval / 2.0)); secondaryValue += m_VerticalSecondaryValueInterval)
		{
			QPoint startPoint = keyframePoint(m_FromTime, secondaryValue);
			QPoint endPoint = keyframePoint(m_ToTime, secondaryValue);
			QPoint offset = QPoint(0, -1);
			painter.setPen(secondaryDarkPen);
			painter.drawLine(startPoint + offset, endPoint + offset);
			painter.setPen(secondaryLightPen);
			painter.drawLine(startPoint, endPoint);
		}
	}

	// Draw vertical secondary grid lines
	double startTime = floor(m_FromTime / m_HorizontalPrimaryTimeInterval) * m_HorizontalPrimaryTimeInterval;
	double endTime = ceil(m_ToTime / m_HorizontalPrimaryTimeInterval) * m_HorizontalPrimaryTimeInterval;

	for (double time = startTime; time <= endTime; time += m_HorizontalPrimaryTimeInterval)
	{
		for (double secondaryTime = time + m_HorizontalSecondaryTimeInterval; secondaryTime <= (time + m_HorizontalPrimaryTimeInterval - (m_HorizontalSecondaryTimeInterval / 2.0)); secondaryTime += m_HorizontalSecondaryTimeInterval)
		{
			QPoint startPoint = keyframePoint(secondaryTime, minValue);
			QPoint endPoint = keyframePoint(secondaryTime, maxValue);
			QPoint offset = QPoint(-1, 0);
			painter.setPen(secondaryDarkPen);
			painter.drawLine(startPoint + offset, endPoint + offset);
			painter.setPen(secondaryLightPen);
			painter.drawLine(startPoint, endPoint);
		}
	}

	// Draw horizontal primary grid lines
	for (double value = minValue; value <= maxValue; value += m_VerticalPrimaryValueInterval)
	{
		QPoint startPoint = keyframePoint(m_FromTime, value);
		QPoint endPoint = keyframePoint(m_ToTime, value);
		QPoint offset = QPoint(0, -1);
		painter.setPen(primaryDarkPen);
		painter.drawLine(startPoint + offset, endPoint + offset);
		painter.setPen(primaryLightPen);
		painter.drawLine(startPoint, endPoint);
	}

	// Draw vertical primary grid lines
	for (double time = startTime; time <= endTime; time += m_HorizontalPrimaryTimeInterval)
	{
		QPoint startPoint = keyframePoint(time, minValue);
		QPoint endPoint = keyframePoint(time, maxValue);
		QPoint offset = QPoint(-1, 0);
		painter.setPen(primaryDarkPen);
		painter.drawLine(startPoint + offset, endPoint + offset);
		painter.setPen(primaryLightPen);
		painter.drawLine(startPoint, endPoint);
	}
}

void AnimationCurveEditor::paintValueRuler(QPainter &painter)
{
	QRect grid = gridRect();

	// Calculate the range of values to be displayed
	double minValue = m_VerticalCenterValue - ((grid.height() / 2.0) / m_VerticalPixelPerValue);
	double maxValue = m_VerticalCenterValue + ((grid.height() / 2.0) / m_VerticalPixelPerValue);
	minValue = floor(minValue / m_VerticalPrimaryValueInterval) * m_VerticalPrimaryValueInterval;
	maxValue = ceil(maxValue / m_VerticalPrimaryValueInterval) * m_VerticalPrimaryValueInterval;

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
	QPen curvePen = QPen(curveColor);
	curvePen.setWidthF(1.5);
	painter.setPen(curvePen);

	const double pxStep = 2;
	const double fromX = keyframePointF(m_FromTime, 0).x(); // TODO: Adjust to 0 time aligned with framerate or pxStep
	const double toX = keyframePointF(m_ToTime, 0).x(); // TODO: Adjust to 0 time aligned with framerate or pxStep

	const AnimationTrack::KeyframeMap &keyframes = track->keyframes();
	AnimationTrack::KeyframeMap::const_iterator it = keyframes.begin();

	// Check if the track has at least two keyframes
	QPainterPath path;
	if (it != keyframes.end() && std::next(it) != keyframes.end())
	{
		QPointF lastPoint = keyframePointF(it.key(), it.value().Value);
		path.moveTo(lastPoint);

		bool withinTimeRange = false;

		for (; std::next(it) != keyframes.end(); ++it)
		{
			double time1 = it.key();
			double time2 = std::next(it).key();

			if (!withinTimeRange && keyframePointF(time2, 0).x() >= fromX)
			{
				// We are now within the time range
				withinTimeRange = true;
			}

			if (withinTimeRange)
			{
				if (track->interpolationMethod() == AnimationInterpolation::Linear)
				{
					// If time1 is beyond the m_ToTime, we can exit the loop early
					if (time1 > m_ToTime)
						break;

					QPointF nextPoint = keyframePointF(time2, std::next(it).value().Value);
					path.lineTo(nextPoint);
					lastPoint = nextPoint;
				}
				else
				{
					double x = lastPoint.x();
					x += pxStep;
					double timeX = timeAtX(x);
					while (timeX <= time2)
					{
						// If lastPoint is beyond the toX, we can exit the loop early
						if (lastPoint.x() > toX)
							break;

						QPointF nextPoint = keyframePointF(timeX, track->valueAtTime(it, std::next(it), timeX));
						path.lineTo(nextPoint);
						lastPoint = nextPoint;
						x += pxStep;
						timeX = timeAtX(x);
					}
					if (lastPoint.x() > toX)
						break;
				}
			}
		}
	}
	painter.drawPath(path);
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

	// Handle pen
	QPen handlePen = QPen(palette().color(QPalette::ButtonText));
	handlePen.setWidthF(0.75);

	// Paint the curves
	for (AnimationTrack *track : m_AnimationTracks)
	{
		QColor curveColor = track->color();
		painter.setRenderHint(QPainter::Antialiasing, true);
		paintCurve(painter, track, curveColor);

		const AnimationTrack::KeyframeMap &keyframes = track->keyframes();
		painter.setRenderHint(QPainter::Antialiasing, false);
		for (AnimationTrack::KeyframeMap::const_iterator it = keyframes.begin(); it != keyframes.end(); ++it)
		{
			QPoint point = keyframePoint(it.key(), it.value().Value);
			bool selected = m_SelectedKeyframes.contains(it.value().Id);
			if (track->interpolationMethod() == AnimationInterpolation::Bezier)
			{
				bool leftSelected = m_SelectedLeftInterpolationHandles.contains(it.value().Id);
				bool rightSelected = m_SelectedRightInterpolationHandles.contains(it.value().Id);
				if (selected || leftSelected || rightSelected)
				{
					const double scale = 1.0;
					const int handleHalfSize = 3;
					QPoint leftOffset = keyframePointOffset(
					    it.value().Interpolation.Bezier.InTangentX * scale,
					    -it.value().Interpolation.Bezier.InTangentY * scale);
					QPoint leftPoint = point + leftOffset;
					QPoint rightOffset = keyframePointOffset(
					    it.value().Interpolation.Bezier.OutTangentX * scale,
					    -it.value().Interpolation.Bezier.OutTangentY * scale);
					QPoint rightPoint = point + rightOffset;
					QRect leftHandleRect = QRect(leftPoint.x() - handleHalfSize, leftPoint.y() - handleHalfSize, handleHalfSize * 2, handleHalfSize * 2);
					QRect rightHandleRect = QRect(rightPoint.x() - handleHalfSize, rightPoint.y() - handleHalfSize, handleHalfSize * 2, handleHalfSize * 2);
					painter.setRenderHint(QPainter::Antialiasing, true);
					painter.setPen(handlePen);
					painter.drawLine(point, leftPoint);
					painter.drawLine(point, rightPoint);
					painter.setRenderHint(QPainter::Antialiasing, false);
					bool leftHover = m_HoverLeftInterpolationHandle == it.value().Id;
					bool leftActive = (m_ActiveLeftInterpolationHandle == it.value().Id) && leftHover;
					paintInterpolationHandle(painter, leftHandleRect, leftSelected, leftHover, leftActive);
					bool rightHover = m_HoverRightInterpolationHandle == it.value().Id;
					bool rightActive = (m_ActiveRightInterpolationHandle == it.value().Id) && rightHover;
					paintInterpolationHandle(painter, rightHandleRect, rightSelected, rightHover, rightActive);
				}
			}
			bool hover = m_HoverKeyframe == it.value().Id;
			bool active = (m_ActiveKeyframe == it.value().Id) && hover;
			QRect keyframeRect = QRect(point.x() - 6, point.y() - 6, 12, 12);
			paintKeyframe(painter, keyframeRect, selected, hover, active);
		}
	}

	// Paint selection
	if (m_InteractionState == InteractionState::MultiSelect)
	{
		QRect selectionRect = QRect(m_MouseLeftPressPosition, m_MouseMovePosition).normalized();
		QStyleOptionRubberBand rubberBandOption;
		rubberBandOption.initFrom(this);
		rubberBandOption.rect = selectionRect;
		style()->drawControl(QStyle::CE_RubberBand, &rubberBandOption, &painter, this);
	}
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
	// ...
}

void AnimationCurveEditor::enterEvent(QEnterEvent *event)
{
	// ...
}

void AnimationCurveEditor::leaveEvent(QEvent *event)
{
	updateMousePosition(QPoint(), false);
}

bool AnimationCurveEditor::eventFilter(QObject *watched, QEvent *event)
{
	if (m_InteractionState == InteractionState::MultiSelect)
	{
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			if (keyEvent->key() == Qt::Key_Control)
			{
				updateMouseSelection(true);
				update();
			}
		}
		else if (event->type() == QEvent::KeyRelease)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			if (keyEvent->key() == Qt::Key_Control)
			{
				updateMouseSelection(false);
				update();
			}
		}
	}
	return QWidget::eventFilter(watched, event);
}

/* end of file */
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

#pragma once
#ifndef ANIMATION_EDITOR__H
#define ANIMATION_EDITOR__H

#include "AnimationEditorGlobal.h"

#include <QWidget>

#include "AnimationTrack.h"

class QToolBar;
class QTreeWidget;
class QTreeWidgetItem;

class AnimationTimelineEditor;
class AnimationCurveEditor;
class AnimationTimeScrubber;
class AnimationEditor;

struct AnimationNode
{
private:
	friend AnimationEditor;
	QList<AnimationNode *> Nodes;
	QList<AnimationTrack *> Tracks;
	QTreeWidgetItem *TreeWidgetItem;
};

class ANIMATIONEDITOR_EXPORT AnimationEditor : public QWidget
{
	Q_OBJECT

public:
	AnimationEditor(QWidget *parent);
	virtual ~AnimationEditor();

	// Functions to add and remove nodes and tracks
	AnimationNode *addNode(AnimationNode *parentNode = nullptr);
	void removeNode(AnimationNode *node);
	AnimationTrack *addTrack(AnimationNode *node = nullptr);
	void removeTrack(AnimationTrack *track);

private:
	QToolBar *m_ToolBar;
	QToolBar *m_TrackTreeToolBar;
	QTreeWidget *m_TrackTreeWidget;
	AnimationTimelineEditor *m_TimelineEditor;
	AnimationCurveEditor *m_CurveEditor;
	AnimationTimeScrubber *m_TimeScrubber;

	AnimationNode m_RootNode;

private:
	// Helper functions
	AnimationNode *findParentNode(AnimationNode *rootNode, AnimationNode *targetNode);
	AnimationNode *findParentNodeWithTrack(AnimationNode *startNode, AnimationTrack *track);
	// void removeTrackFromNode(AnimationNode *node, AnimationTrack *track);
	void deleteNodeAndChildren(AnimationNode *node);
	void updateTimelineTracks();
	void listTimelineTracks(QList<AnimationTrack *> &tracks, AnimationNode *node);

}; /* class AnimationEditor */

#endif /* ANIMATION_EDITOR__H */

/* end of file */

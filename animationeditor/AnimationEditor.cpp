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

#include "AnimationEditor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>

#include <QToolBar>
#include <QTreeWidget>

#include "AnimationTimelineEditor.h"
#include "AnimationCurveEditor.h"
#include "AnimationTimeScrubber.h"

/*

ChatGPT Dump:

- You may want to add icons to the toolbar actions for better visual cues for the users.
To do this, you can use QIcon and set it to the actions with QAction::setIcon().

- In the right layout, you've set both the timeline editor and curve editor to be visible at the same time.
You may want to consider adding a mechanism to switch between the timeline editor
and curve editor visibility based on user actions or preferences,
such as using a button or menu action that toggles their visibility.

- If you plan to make the left side of the splitter resizable,
consider using QSplitter::setCollapsible() to prevent the left side from being completely collapsed.

- Since you're setting the layout for AnimationEditor,
which inherits from QWidget, you don't need to call setLayout(mainLayout) explicitly.
Instead, pass this to the QVBoxLayout constructor: QVBoxLayout *mainLayout = new QVBoxLayout(this).
The layout will automatically be set for the AnimationEditor widget.

*/

AnimationEditor::AnimationEditor(QWidget *parent)
    : QWidget(parent)
    , m_ToolBar(new QToolBar(this))
    , m_TrackTreeToolBar(new QToolBar(this))
    , m_TrackTreeWidget(new QTreeWidget(this))
    , m_TimelineEditor(new AnimationTimelineEditor(this))
    , m_CurveEditor(new AnimationCurveEditor(this))
    , m_TimeScrubber(new AnimationTimeScrubber(this))
{
	// Set up the toolbar
	m_ToolBar->addAction("Action 1");
	m_ToolBar->addAction("Action 2");

	m_TrackTreeToolBar->addAction("Action 1");
	m_TrackTreeToolBar->addAction("Action 2");

	// Layout, first row for toolbar, second row a splitter
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	// mainLayout->setMargin(0);
	// mainLayout->setSpacing(0);
	mainLayout->addWidget(m_ToolBar);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
	// splitter->setHandleWidth(2);
	// splitter->setContentsMargins(0, 0, 0, 0);

	// In the splitter, on both sides a vertical layout
	// On the left side, first an extra toolbar that matches the time scrubber height, then the tree view
	QWidget *leftWidget = new QWidget();
	// leftWidget->setContentsMargins(0, 0, 0, 0);
	QVBoxLayout *leftLayout = new QVBoxLayout();
	leftLayout->setContentsMargins(0, 0, 0, 0);
	m_TrackTreeToolBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_TrackTreeToolBar->setFixedHeight(30);
	// leftToolbar->setContentsMargins(0, 0, 0, 0);
	// leftLayout->setMargin(0);
	// leftLayout->setSpacing(0);
	leftLayout->addWidget(m_TrackTreeToolBar);
	leftLayout->addWidget(m_TrackTreeWidget);
	leftWidget->setLayout(leftLayout);
	splitter->addWidget(leftWidget);

	// On the right side, first the time scrubber, then either the curve editor or the timeline editor (we can switch visibility between at runtime)
	QWidget *rightWidget = new QWidget();
	QVBoxLayout *rightLayout = new QVBoxLayout();
	rightLayout->setContentsMargins(0, 0, 0, 0);
	m_TimeScrubber->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_TimeScrubber->setFixedHeight(30);
	// m_TimeScrubber->setContentsMargins(0, 0, 0, 0);
	// rightLayout->setMargin(0);
	// rightLayout->setSpacing(0);
	rightLayout->addWidget(m_TimeScrubber);
	rightLayout->addWidget(m_TimelineEditor);
	rightLayout->addWidget(m_CurveEditor);
	m_CurveEditor->setVisible(false);
	rightWidget->setLayout(rightLayout);
	splitter->addWidget(rightWidget);

	// Left side of splitter should have an initial fixed size, resizing the window should just resize the right size
	splitter->setStretchFactor(0, 0);
	splitter->setStretchFactor(1, 1);

	// Add the splitter to the main layout
	mainLayout->addWidget(splitter);

	// Set up the main layout
	setLayout(mainLayout);

	// Set up the tree widget
	m_RootNode.TreeWidgetItem = m_TrackTreeWidget->invisibleRootItem();

	// Hide the header in the tree widget
	m_TrackTreeWidget->setHeaderHidden(true);
}

AnimationEditor::~AnimationEditor()
{
	// Clean up the root node and its children
	deleteNodeAndChildren(&m_RootNode);
}

AnimationNode *AnimationEditor::addNode(AnimationNode *parentNode)
{
	AnimationNode *newNode = new AnimationNode();
	newNode->TreeWidgetItem = new QTreeWidgetItem(m_TrackTreeWidget);
	newNode->TreeWidgetItem->setText(0, "Node Name");

	if (parentNode == nullptr)
		parentNode = &m_RootNode;

	parentNode->Nodes.append(newNode);
	parentNode->TreeWidgetItem->addChild(newNode->TreeWidgetItem);

	m_TrackTreeWidget->expandItem(parentNode->TreeWidgetItem);

	return newNode;
}

void AnimationEditor::removeNode(AnimationNode *node)
{
	AnimationNode *parentNode = findParentNode(&m_RootNode, node);

	if (parentNode != nullptr)
	{
		parentNode->Nodes.removeOne(node);
		parentNode->TreeWidgetItem->removeChild(node->TreeWidgetItem);
	}

	updateTimelineTracks();

	deleteNodeAndChildren(node);
}

AnimationTrack *AnimationEditor::addTrack(AnimationNode *node)
{
	AnimationTrack *newTrack = new AnimationTrack(this);
	newTrack->m_TreeWidgetItem = new QTreeWidgetItem(m_TrackTreeWidget);
	newTrack->m_TreeWidgetItem->setText(0, "Track Name");

	if (node == nullptr)
		node = &m_RootNode;

	node->Tracks.append(newTrack);
	node->TreeWidgetItem->addChild(newTrack->m_TreeWidgetItem);

	m_TrackTreeWidget->expandItem(node->TreeWidgetItem);
	
	updateTimelineTracks();

	return newTrack;
}

void AnimationEditor::removeTrack(AnimationTrack *track)
{
	AnimationNode *parentNode = findParentNodeWithTrack(&m_RootNode, track);

	if (parentNode != nullptr)
	{
		parentNode->Tracks.removeOne(track);
		parentNode->TreeWidgetItem->removeChild(track->m_TreeWidgetItem);
	}

	updateTimelineTracks();

	delete track->m_TreeWidgetItem;
	delete track;
}

AnimationNode *AnimationEditor::findParentNode(AnimationNode *rootNode, AnimationNode *targetNode)
{
	for (AnimationNode *childNode : rootNode->Nodes)
	{
		if (childNode == targetNode) return rootNode;
		AnimationNode *parentNode = findParentNode(childNode, targetNode);
		if (parentNode != nullptr) return parentNode;
	}
	return nullptr;
}

AnimationNode *AnimationEditor::findParentNodeWithTrack(AnimationNode *startNode, AnimationTrack *track)
{
	if (startNode->Tracks.contains(track))
	{
		return startNode;
	}

	for (AnimationNode *childNode : startNode->Nodes)
	{
		AnimationNode *parentNode = findParentNodeWithTrack(childNode, track);
		if (parentNode != nullptr)
		{
			return parentNode;
		}
	}

	return nullptr;
}

/*
void AnimationEditor::removeTrackFromNode(AnimationNode *node, AnimationTrack *track)
{
    if (node->Tracks.removeOne(track))
    {
        delete track;
        return;
    }

    for (AnimationNode *childNode : node->Nodes)
    {
        removeTrackFromNode(childNode, track);
    }
}
*/

void AnimationEditor::deleteNodeAndChildren(AnimationNode *node)
{
	// Recursively delete child nodes
	while (!node->Nodes.isEmpty())
	{
		AnimationNode *childNode = node->Nodes.takeLast();
		deleteNodeAndChildren(childNode);
	}

	// Delete tracks
	qDeleteAll(node->Tracks);
	node->Tracks.clear();

	// Delete the node itself
	if (node != &m_RootNode)
		delete node;
}

void AnimationEditor::updateTimelineTracks()
{
	QList<AnimationTrack *> tracks;
	listTimelineTracks(tracks, &m_RootNode);
	m_TimelineEditor->setAnimationTracks(tracks);
}

void AnimationEditor::listTimelineTracks(QList<AnimationTrack *> &tracks, AnimationNode *node)
{
	// Iterate through nodes
	for (AnimationNode *childNode : node->Nodes)
	{
		listTimelineTracks(tracks, childNode);
	}

	// Iterate through tracks
	for (AnimationTrack *track : node->Tracks)
	{
		tracks.append(track);
	}
}

/*

ChatGPT Dump:

The current implementation allows developers to add and remove nodes and tracks,
as well as modify keyframe properties using different interpolation methods.
However, there are some areas that may require further improvements or enhancements:

- User Interface: The UI design can be refined and polished to improve the
user experience and to better align with the overall look and feel of the target application.

- Undo/Redo functionality: To provide a more robust editing experience,
implementing an undo/redo system would be beneficial.

- Integration with external data formats: Support for importing and exporting
animation data to and from various file formats could be added to facilitate
integration with other tools.

- Performance optimizations: Depending on the use case, performance optimizations
may be required to handle large animation datasets more efficiently.

*/

/* end of file */
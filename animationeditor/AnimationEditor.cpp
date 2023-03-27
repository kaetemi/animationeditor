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
    , m_TrackTreeView(new QTreeView(this))
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
	leftLayout->addWidget(m_TrackTreeView);
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
}

AnimationEditor::~AnimationEditor()
{
}

AnimationNode *AnimationEditor::addNode(AnimationNode *parentNode)
{
	AnimationNode *newNode = new AnimationNode;
	if (parentNode == nullptr) parentNode = &m_RootNode;
	parentNode->Nodes.append(newNode);
	// Update the UI, e.g., TreeView
	return newNode;
}

AnimationTrack *AnimationEditor::addTrack(AnimationNode *node)
{
	AnimationTrack *newTrack = new AnimationTrack(this);
	if (node == nullptr) node = &m_RootNode;
	node->Tracks.append(newTrack);
	// Update the UI, e.g., TreeView
	return newTrack;
}

void AnimationEditor::removeNode(AnimationNode* node)
{
	// Find the parent node
	AnimationNode* parentNode = findParentNode(&m_RootNode, node);
	if (parentNode != nullptr) parentNode->Nodes.removeOne(node);

	// Recursively delete child nodes and tracks
	deleteNodeAndChildren(node);

	// Update the UI, e.g., TreeView
}

void AnimationEditor::removeTrack(AnimationTrack *track)
{
	removeTrackFromNode(&m_RootNode, track);
	// Update the UI, e.g., TreeView
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

void AnimationEditor::deleteNodeAndChildren(AnimationNode* node)
{
	// Recursively delete child nodes
	while (!node->Nodes.isEmpty())
	{
		AnimationNode* childNode = node->Nodes.takeLast();
		deleteNodeAndChildren(childNode);
	}

	// Delete tracks
	qDeleteAll(node->Tracks);
	node->Tracks.clear();

	// Delete the node itself
	delete node;
}

/* end of file */
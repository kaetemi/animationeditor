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

AnimationEditor::AnimationEditor(QWidget *parent)
    : QWidget(parent)
    , m_Toolbar(new QToolBar(this))
    , m_TrackTreeView(new QTreeView(this))
    , m_TimelineEditor(new AnimationTimelineEditor(this))
    , m_CurveEditor(new AnimationCurveEditor(this))
    , m_TimeScrubber(new AnimationTimeScrubber(this))
{
	// Set up the toolbar
	m_Toolbar->addAction("Action 1");
	m_Toolbar->addAction("Action 2");

	// Layout, first row for toolbar, second row a splitter
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	// mainLayout->setMargin(0);
	// mainLayout->setSpacing(0);
	mainLayout->addWidget(m_Toolbar);
	QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
	// splitter->setHandleWidth(2);

	// In the splitter, on both sides a vertical layout
	// On the left side, first an extra toolbar that matches the time scrubber height, then the tree view
	QWidget *leftWidget = new QWidget();
	QVBoxLayout *leftLayout = new QVBoxLayout();
	QToolBar *leftToolbar = new QToolBar(this);
	leftToolbar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	leftToolbar->setFixedHeight(30);
	// leftToolbar->setContentsMargins(0, 0, 0, 0);
	// leftLayout->setMargin(0);
	// leftLayout->setSpacing(0);
	leftLayout->addWidget(leftToolbar);
	leftLayout->addWidget(m_TrackTreeView);
	leftWidget->setLayout(leftLayout);
	splitter->addWidget(leftWidget);

	// On the right side, first the time scrubber, then either the curve editor or the timeline editor (we can switch visibility between at runtime)
	QWidget *rightWidget = new QWidget();
	QVBoxLayout *rightLayout = new QVBoxLayout();
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

	// Add the splitter to the main layout
	mainLayout->addWidget(splitter);

	// Set up the main layout
	setLayout(mainLayout);
}

AnimationEditor::~AnimationEditor()
{
}

/* end of file */
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

*/

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QStyleFactory>

#include "AnimationEditor.h" // Include the header file for your AnimationEditor widget

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	QApplication::setStyle(QStyleFactory::create("Fusion"));
	QPalette palette = qApp->palette();
	palette.setColor(QPalette::Window, QColor(64, 64, 64));
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Base, QColor(48, 48, 48));
	palette.setColor(QPalette::AlternateBase, QColor(64, 64, 64));
	palette.setColor(QPalette::ToolTipBase, Qt::white);
	palette.setColor(QPalette::ToolTipText, Qt::white);
	palette.setColor(QPalette::Text, Qt::white);
	palette.setColor(QPalette::Button, QColor(64, 64, 64));
	palette.setColor(QPalette::ButtonText, Qt::white);
	palette.setColor(QPalette::BrightText, Qt::red);
	palette.setColor(QPalette::Highlight, QColor(64, 128, 96));
	palette.setColor(QPalette::HighlightedText, Qt::white);
	qApp->setPalette(palette);

	QMainWindow mainWindow;
	mainWindow.setWindowTitle("Animation Editor Preview");

	QWidget *centralWidget = new QWidget(&mainWindow);
	QVBoxLayout *layout = new QVBoxLayout(centralWidget);

	AnimationEditor *animationEditor = new AnimationEditor(centralWidget);
	layout->addWidget(animationEditor);

	centralWidget->setLayout(layout);
	mainWindow.setCentralWidget(centralWidget);

	// Add sample tracks and keyframes for testing purposes
	AnimationTrack *track1 = animationEditor->addTrack();
	track1->upsertKeyframe(0.0, 1.0);
	track1->upsertKeyframe(1.0, 2.0);
	track1->upsertKeyframe(2.0, 1.0);

	for (int i = 0; i < 10; ++i)
	{
		AnimationTrack *track2 = animationEditor->addTrack();
		track2->upsertKeyframe(0.5 + i, 2.0);
		track2->upsertKeyframe(1.5 + i, 3.0);
		track2->upsertKeyframe(2.5 + i, 2.0);
	}

	mainWindow.resize(900, 400);
	mainWindow.show();

	return app.exec();
}

/* end of file */

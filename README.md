# AnimationEditor Widget

The AnimationEditor is a QWidget for Qt, which allows developers to create, edit, and manage animations in a user-friendly interface. It is designed for use in various applications, such as game development, multimedia applications, and animation tools.

![Animation Editor](https://raw.githubusercontent.com/kaetemi/animationeditor/master/doc/screenshot.png "Screenshot")

The main components of the AnimationEditor include:
1. `AnimationEditor`: The main QWidget that combines and manages all other sub-widgets.
2. `AnimationTrack`: A QObject representing a track with its keyframes and associated data.
3. `AnimationTimelineEditor`: A QWidget for visualizing and editing keyframes in a timeline view, showing rows for all the tracks with handles for keyframes.
4. `AnimationCurveEditor`: A QWidget for visualizing and editing keyframes as curves, displaying only the curves of the currently selected tracks.
5. `AnimationTimeScrubber`: A QWidget displaying a time ruler and scrubber handle for navigating the timeline and previewing animations at specified times.

The layout of the AnimationEditor includes a toolbar at the top, a time ruler with a scrubber handle below the toolbar, a tree view of nodes with their animation tracks on the left side, and either the timeline or curve editor on the right side.

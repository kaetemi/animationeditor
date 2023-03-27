# AnimationEditor Widget

The AnimationEditor is a Qt-based QWidget designed to edit, visualize, and manage animations in a user-friendly and flexible interface. It allows users to create and manipulate animation tracks and keyframes, providing separate views for a timeline and a curve editor. The timeline view displays keyframes as points on a track, while the curve editor represents keyframes as curves, allowing for fine-grained control of animation values. It's suitable for a range of use cases, including game development, multimedia applications, and animation tools.

![Animation Editor](https://raw.githubusercontent.com/kaetemi/animationeditor/master/doc/screenshot.png "Screenshot")

The main components of the AnimationEditor include:
1. `AnimationEditor`: The main QWidget that combines and manages all other sub-widgets.
2. `AnimationTrack`: A QObject representing a track with its keyframes and associated data.
3. `AnimationTimelineEditor`: A QWidget for visualizing and editing keyframes in a timeline view, showing rows for all the tracks with handles for keyframes.
4. `AnimationCurveEditor`: A QWidget for visualizing and editing keyframes as curves, displaying only the curves of the currently selected tracks.
5. `AnimationTimeScrubber`: A QWidget displaying a time ruler and scrubber handle for navigating the timeline and previewing animations at specified times.

The AnimationEditor is organized with a toolbar at the top, a time ruler below the toolbar, a tree view of nodes with their animation tracks on the left side, and either the timeline or curve editor on the right side.

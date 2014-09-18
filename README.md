# libwheel - A library to avoid reinventing the wheel

[![Build Status](https://drone.io/github.com/aperezdc/wheel/status.png)](https://drone.io/github.com/aperezdc/wheel/latest)

The “wheel” library aims to provide basic (and some not-that-basic)
facilities that one would expect to have in C. For the moment it contains:

- Memory handling routines.
- In-memory buffers.
- Support for loading and saving “configuration” files. Actually, This
	module can be (ab)used to save and load arbitrary data.
- Hash-based dictionaries (being dictionaries means that keys are always
	strings).
- String handling functions.
- Command line parsing.
- Utilities for building simple parsers.
- Generic input/output streams.


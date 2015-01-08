# libwheel - A library to avoid reinventing the wheel

[![Build Status](https://img.shields.io/travis/aperezdc/wheel.svg?style=flat)](https://travis-ci.org/aperezdc/wheel)
[![Code Coverage](https://img.shields.io/coveralls/aperezdc/wheel/master.svg?style=flat)](https://coveralls.io/r/aperezdc/wheel?branch=master)
[![Documentation Status](https://readthedocs.org/projects/libwheel/badge/?version=latest)](https://libwheel.readthedocs.org/en/latest)

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


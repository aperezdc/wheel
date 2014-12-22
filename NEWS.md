What's new in libwheel
======================

v0.2.0 (unreleased)
-------------------

* New API for lighweight coroutines (tasks) inspired by Plan 9's `libtask`.

* New `w_str_ncasecmp()` function, to perform case-insensitive ASCII string
  comparison.

* New `w_str_time_period()` and `w_str_size_bytes()` conversion functions,
  which can be used alongside `wopt`.

* New `wmeta` macros, which can be used to generate metadata about `struct`
  layouts. This can be used to implement (de)serialization routines which
  read and write values directly to/from structures, without needing to use
  `w_variant_t` objects.

* New macros `w_lmem` and `w_lobj` which can be used to mark stack-allocated
  variables to be released automatically. Using `w_lmem` marks a pointer to be
  freed using `w_free()` when control flow exits the declaration scope; and
  `w_lobj` marks a pointer as a reference to an object and `w_obj_unref()`
  will be called on it when control flow exits the declaration scope. Using
  those macros requires using GCC or Clang. The macros are not used in the
  code of `libwheel` itself, so the library will still be useable with other
  compilers.

* Functions `w_free()` and `w_obj_unref()` now accept being passed a `NULL`,
  in which case they are no-ops.

* Functions which report sizes of containers and other structures now all have
  a `_size` suffix. This makes the overall API of the library more consistent.

* Lists and dictionaries now support keeping references to contained objects.
  This is useful when it is known that a list or dictionary is going to always
  contain objects; the container will call `w_obj_ref()` on objects added to
  the container and `w_obj_unref()` when removing them from the container.
  This helps in keeping calls for refing/unrefing objects balanced.

* The I/O system now uses `w_io_result_t` as return type for most functions.
  All the calls to I/O functions must be changed to handle the new return
  type. This change brings in better type checking, easier error handling,
  and a more consistent API. Last, but not least, a number of long-standing
  minor bugs (most of which have gone unnoticed until now) have been fixed
  thanks to this change and the added unit tests.

* As a result of the improved I/O API, certain functions which previously did
  not track the amount of data bytes read/written now are able to report the
  amount of data read/written.

* A compiler that implements C99 is now required to build and use `libwheel`.
  Most current compilers will work without problems. GCC and Clang, the main
  targets of `libwheel` have supported C99 for a long while.

* On the light of the new implementation of [wcfg for
  Python](https://github.com/aperezdc/python-wcfg), the implementation in
  `libwheel` has been updated to allow the same syntax and data types as the
  Python implementation. The main changes are:

  - Lists are now allowed as values.
  - Optional colon (`:`) between keys and values.

* Fix reading floating point numbers with embedded dots.

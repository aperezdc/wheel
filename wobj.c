/*
 * wobj.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * Objects
 * =======
 *
 * The object system allows to do object oriented programming in C99. The
 * object model has the following features:
 *
 * - Simple inheritance. All object types must “inherit” from :type:`w_obj_t`.
 *   Of course, composition of objects is also possible.
 *
 * - Objects are typically allocated in the heap, but it is possible to
 *   allocate them statically, or in the stack with the aid of the
 *   :macro:`W_OBJ_STATIC` macro.
 *
 * - Objects keep a reference counter, which can be manipulated using
 *   :func:`w_obj_ref()`, and :func:`w_obj_unref()`. Objects are
 *   deallocated when their reference counter drops to zero.
 *
 * - It is possible to assign a “destructor function” to any object using
 *   :func:`w_obj_dtor()`.
 *
 * - Minimal overhead: objects do not have a *vtable* by default, and dynamic
 *   method dispatching is not done unless explicitly added by the user.
 *
 * - Uses only C99 constructs, and it does not require any special compiler
 *   support.
 *
 * - Optionally, when using GCC or Clang, the reference count for an object
 *   can be automatically decreased when a pointer to it goes out of scope,
 *   by marking it with the :macro:`w_lobj` macro.
 *
 *
 * Usage
 * -----
 *
 * This example shows how to define a base “shape” object type: ``shape_t``;
 * and two derived types for squares (``square_t``) and rectangles
 * (``rectangle_t``).
 *
 * In order to have methods which work on any object derived from the shape
 * type, a “vtable” is added manually to perform dynamic dispatch using a
 * shared ``struct`` which contains function pointers to the actual
 * implementations for each shape. Another valid approach would be to add
 * the function pointers directly in ``shape_t`` to avoid the extra
 * indirection. This second approach would be better if the function pointers
 * to method implementations could change at runtime, at the cost of each
 * instance of a shape occupying some extra bytes of memory.
 *
 * Header:
 *
 * .. code-block:: c
 *
 *
 *      // Objects have no vtable by default, so one is defined manually.
 *      typedef struct {
 *          double (*calc_area)      (void*);
 *          double (*calc_perimeter) (void*);
 *      } shape_vtable_t;
 *
 *      // Base object type for shapes.
 *      W_OBJ (shape_t) {
 *          w_obj_t         parent;  // Base object type.
 *          shape_vtable_t *vtable;  // Pointer to vtable.
 *      };
 *
 *      // A square shape.
 *      W_OBJ (square_t) {
 *          shape_t parent;  // Inherits both base object and vtable.
 *          double  side_length;
 *      };
 *
 *      // A rectangular shape.
 *      W_OBJ (rectangle_t) {
 *          shape_t parent;  // Inherits both base object and vtable.
 *          double  width;
 *          double  height;
 *      };
 *
 *      // Functions used to create new objects.
 *      extern shape_t* square_new (double side_length);
 *      extern shape_t* rectangle_new (double width, double height);
 *
 *      // Convenience functions to avoid having to manually make the
 *      // dynamic dispatch through the vtable manually in client code.
 *      static inline double shape_calc_area (shape_t *shape) {
 *          return (*shape->vtable->calc_area) (shape);
 *      }
 *      static inline double shape_calc_perimeter (shape_t *shape) {
 *          return (*shape->vtable->calc_perimeter) (shape);
 *      }
 *
 *
 * Implementation:
 *
 * .. code-block:: c
 *
 *      // Methods and vtable for squares.
 *      static double square_calc_area (void *obj) {
 *          double side_length = ((square_t*) obj)->side_length;
 *          return side_length * side_length;
 *      }
 *      static double square_calc_perimeter (void *obj) {
 *          return 4 * ((square_t*) obj)->side_length;
 *      }
 *      static const shape_vtable_t square_vtable = {
 *          .calc_area      = square_calc_area,
 *          .calc_perimeter = square_calc_perimeter,
 *      };
 *
 *      shape_t* square_new (double side_length) {
 *          square_t *square = w_obj_new (square_t);  // Make object.
 *          square->parent.vtable = &square_vtable;   // Set vtable.
 *          square->side_length = side_length;
 *          return (shape_t*) square;
 *      }
 *
 *      // Methods and vtable for rectangles.
 *      static double rectangle_calc_area (void *obj) {
 *          rectangle_t *rect = (rectangle_t*) obj;
 *          return rect->width * rect->height;
 *      }
 *      static double rectangle_calc_perimeter (void *obj) {
 *          rectangle_t *rect = (rectangle_t*) obj;
 *          return 2 * (rect->width + rect->height);
 *      }
 *      static const shape_vtable_t rectangle_vtable = {
 *          .calc_area      = rectangle_calc_area,
 *          .calc_perimeter = rectangle_calc_perimeter,
 *      };
 *
 *      shape_t*
 *      rectangle_new (double width, double height) {
 *          rectangle_t *rect = w_obj_new (rectangle_t);  // Make object.
 *          rect->parent.vtable = &rectangle_vtable;      // Set vtable.
 *          rect->width = width;
 *          rect->height = height;
 *          return (shape_t*) rect;
 *      }
 *
 *
 * Using shapes:
 *
 * .. code-block:: c
 *
 *      // Uses the generic shape_* functions.
 *      static void print_shape_infos (shape_t *shape) {
 *          w_print ("Shape area: $F\n", shape_calc_area (shape));
 *          w_print ("Shape perimeter: $F\n", shape_calc_perimeter (shape));
 *      }
 *
 *      int main (void) {
 *          w_lobj shape_t *s = square_new (10);
 *          w_lobj shape_t *r = rectangle_new (10, 20);
 *          print_shape_infos (s);  // Works on any object derived from shape_t.
 *          print_shape_infos (r);  // Ditto.
 *          return 0;
 *      }
 */

/**
 * Types
 * -----
 */

/*~t w_obj_t
 *
 * Base type for objects.
 *
 * All other object types must “derive” from this type for the objects system
 * to work properly. This is achieved by having a member of this type as first
 * member of object types — either explicitly or by “inheriting” it from
 * another object type:
 *
 * .. code-block:: c
 *
 *      W_OBJ (my_type) {
 *          // Explicitly make the first member be an "w_obj_t"
 *          w_obj_t parent;
 *      };
 *
 *      W_OBJ (my_subtype) {
 *          // The first member itself has an "w_obj_t" as first member.
 *          my_type parent;
 *      };
 */

/**
 * Macros
 * ------
 */

/*~M W_OBJ_DECL(type)
 *
 * Makes a forward declaration of a object class of a certain `type`.
 *
 * See also :macro:`W_OBJ_DEF`.
 */

/*~M W_OBJ_DEF(type)
 *
 * Defines the structure for an object class of a certain `type`.
 *
 * This macro should be used after the `type` has been declared using the
 * :macro:`W_OBJ_DECL` macro.
 *
 * Typical usage involves declaring the `type` in a header, and the actual
 * layout of it in an implementation file, to make the internals opaque to
 * third party code:
 *
 * .. code-block:: c
 *
 *      // In "my_type.h"
 *      W_OBJ_DECL (my_type);
 *
 *      // In "my_type.c"
 *      W_OBJ_DEF (my_type) {
 *          w_obj_t parent;
 *          int     value;
 *          // ...
 *      };
 */

/*~M W_OBJ(type)
 *
 * Declares *and* defines the structure for an object class of a certain
 * `type`. This is equivalent to using :macro:`W_OBJ_DECL` immediately
 * followed by :macro:`W_OBJ_DEF`.
 *
 * For example:
 *
 * .. code-block:: c
 *
 *      W_OBJ (my_type) {
 *          w_obj_t parent;
 *          int     value;
 *          // ...
 *      };
 *
 * This is used instead of a combination of :macro:`W_OBJ_DECL` and
 * :macro:`W_OBJ_DEF` when a forward declaration is not needed, and it does
 * not matter that the internals of how an object class is implemented are
 * visible in headers:
 */

/*~M W_OBJ_STATIC(destructor)
 *
 * Initializes a statically-allocated object, and sets `destructor` to be
 * called before the object is deallocated by :func:`w_obj_destroy()`.
 *
 * Similarly to :func:`w_obj_mark_static()`, this macro allows to initialize
 * objects for which the memory they occupy will not be deallocated.
 *
 * Typical usage involves initializing static global objects, or objects
 * allocated in the stack, e.g.:
 *
 * .. code-block:: c
 *
 *      W_OBJ (my_type) {
 *          w_obj_t parent;
 *          int     value;
 *      };
 *
 *      static my_type static_object = {
 *          .parent = W_OBJ_STATIC (NULL),
 *          .value  = 42,
 *      };
 *
 *      void do_foo (void) {
 *          my_type stack_object = {
 *              .parent = W_OBJ_STATIC (NULL),
 *              .value  = 32,
 *          };
 *
 *          use_object (&stack_object);
 *      }
 */

/**
 * Functions
 * ---------
 */

#include "wheel.h"


/*~f void* w_obj_ref (void *object)
 *
 * Increases the reference counter of an `object`.
 *
 * The `object` itself is returned, to allow easy chaining of other
 * function calls.
 */
void*
w_obj_ref (void *obj)
{
    if (w_likely (obj != NULL))
        if (w_likely (((w_obj_t*) obj)->__refs != (size_t) -1))
            ((w_obj_t*) obj)->__refs++;
    return obj;
}


/*~f void* w_obj_unref (void *object)
 *
 * Decreases the reference counter of an `object`.
 *
 * Once the reference count for an object reaches zero, it is destroyed
 * using :func:`w_obj_destroy()`.
 *
 * The `object` itself is returned, to allow easy chaining of other
 * function calls.
 */
void*
w_obj_unref (void *obj)
{
    if (w_likely (obj != NULL)) {
        if (w_unlikely (((w_obj_t*) obj)->__refs == (size_t) -1)) {
            w_obj_destroy (obj);
            return NULL;
        }

        if (--((w_obj_t*) obj)->__refs == 0) {
            w_obj_destroy (obj);
            return NULL;
        }
    }
    return obj;
}


/*~f void w_obj_destroy (void *object)
 *
 * Destroys an `object`.
 *
 * If a destructor function was set for the `object` using
 * :func:`w_obj_dtor()`, then it will be called before the
 * memory used by the object being freed.
 */
void
w_obj_destroy (void *obj)
{
    w_assert (obj);

    w_obj_t *o = (w_obj_t*) obj;

    if (o->__dtor) {
        (*o->__dtor) (obj);
        o->__dtor = NULL;
    }

    if (w_likely (o->__refs != (size_t) -1)) {
        w_free (o);
    }
}


/*~f void* w_obj_dtor (void *object, void (*destructor)(void*))
 *
 * Registers a `destructor` function to be called when an `object` is
 * destroyed using :func:`w_obj_destroy()`.
 *
 * The `object` itself is returned, to allow easy chaining of other
 * function calls.
 */
void*
w_obj_dtor (void *obj, void (*dtor) (void*))
{
    w_assert (obj);

    ((w_obj_t*) obj)->__dtor = dtor;
    return obj;
}


/*~f void w_obj_mark_static (void *object)
 *
 * Marks an `object` as being statically allocated.
 *
 * When the last reference to an object marked as static is lost, its destructor
 * will be called, but the area of memory occupied by the object **will not** be
 * freed. This is the same behaviour as for objects initialized with the
 * :macro:`W_OBJ_STATIC` macro. The typical use-case for this function to mark
 * objects that are allocated as part of others, and the function is called during
 * their initialization, like in the following example:
 *
 * .. code-block:: c
 *
 *      W_OBJ (my_type) {
 *          w_obj_t     parent;
 *          w_io_unix_t unix_io;
 *      };
 *
 *      void my_type_free (void *objptr) {
 *          w_obj_destroy (&self->unix_io);
 *      }
 *
 *      my_type* my_type_new (void) {
 *          my_type *self = w_obj_new (my_type);
 *          w_io_unix_init_fd (&self->unix_io, 0);
 *          w_obj_mark_static (&self->unix_io);
 *          return w_obj_dtor (self, _my_type_free);
 *      }
 */
void
w_obj_mark_static (void *obj)
{
    w_assert (obj);
    ((w_obj_t*) obj)->__refs = (size_t) -1;
}


/*~f type* w_obj_new (type)
 *
 * Creates a new instance of an object of a given `type`.
 *
 * Freshly created objects always have a reference count of ``1``.
 */

/*~f type* w_obj_new_with_priv_sized (type, size_t size)
 *
 * Creates a new instance of an object of a given `type`, with additional
 * space of `size` bytes to be used as instance private data.
 *
 * A pointer to the private data of an object can be obtained using
 * :func:`w_obj_priv()`.
 */

/*~f type* w_obj_new_with_priv(type)
 *
 * Creates a new instance of an object of a given `type`, with additional
 * space to be used as instance private data. The size of the private data
 * will be that of a type named after the gicen `type` with a ``_p`` suffix
 * added to it.
 *
 * A pointer to the private data of an object can be obtained using
 * :func:`w_obj_priv()`.
 *
 * Typical usage:
 *
 * .. code-block:: c
 *
 *      // In "my_type.h"
 *      W_OBJ (my_type) {
 *          w_obj_t parent;
 *      };
 *
 *      extern my_type* my_type_new ();
 *
 *
 *      // In "my_type.c"
 *      typedef struct {
 *          int private_value;
 *      } my_type_p;
 *
 *
 *      my_type* my_type_new (void) {
 *          my_type *obj = w_obj_new_with_priv (my_type);
 *          my_type_p *p = w_obj_priv (obj, my_type);
 *          p->private_value = 42;
 *          return obj;
 *      }
 */

/*~f void* w_obj_priv(void *object, type)
 *
 * Obtains a pointer to the private instance data area of an `object` of a
 * given `type`.
 *
 * Note that only objects created using :func:`w_obj_new_with_priv_sized()` or
 * :func:`w_obj_new_with_priv()` have a private data area. The results of
 * using this function on objects which do not have a private data area is
 * undefined.
 */

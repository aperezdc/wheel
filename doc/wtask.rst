
Tasks
=====


Types
-----

.. c:type:: w_task_t

   Type of a task.

.. c:type:: w_task_func_t

   Type of task functions.


Functions
---------

.. c:function:: w_task_t* w_task_prepare (w_task_func_t function, void *data, size_t stack_size)

   Creates a task with a given `stack_size` and prepares it for running a
   `function`, passing a `data` pointer to the function.  The task will be
   in paused state upon creation.

   To get tasks running, the scheduler must be running, see
   :func:`w_task_run_scheduler()`.

   The `stack_size` is  always rounded up to the size of a memory page. It
   is possible to pass zero to get the smallest possible stack size (usually
   4 kB).

.. c:function:: w_task_t* w_task_current ()

   Obtains the task currently running.

   .. warning:: This function **must** be called from inside a task, once the
      task scheduler has been started. Otherwise, calling this function is an
      error and the execution of the program will be aborted.


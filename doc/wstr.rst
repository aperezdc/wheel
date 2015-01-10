.. c:function:: char* w_cstr_format (const char *format, ...)

   Create a C (``\0``-terminated) string with a given `format`, consuming
   additional arguments as needed by the `format`.

   The returned strings must be freed by the caller.

   See :ref:`formatted-output` for the available formatting options.

.. c:function:: char* w_cstr_formatv (const char *format, va_list arguments)

   Create a C (``\0``-terminated) string with a given `format`, consuming
   additional `arguments` as needed by the `format`.

   The returned strings must be freed by the caller.

   See :ref:`formatted-output` for the available formatting options.


/*
 * wdict.h
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __wdict_h__
#define __wdict_h__

#include "wdef.h"
#include <stdlib.h>

typedef struct w_dict w_dict_t;



W_EXPORT w_dict_t* w_dict_new(void);

W_EXPORT void w_dict_free(w_dict_t *d);
W_EXPORT void w_dict_clear(w_dict_t *d);

W_EXPORT unsigned w_dict_count(const w_dict_t *d);

W_EXPORT void* w_dict_getn(const w_dict_t *d, const char *key, size_t keylen);
W_EXPORT void  w_dict_setn(w_dict_t *d, const char *key, size_t keylen, void *data);

W_EXPORT void  w_dict_del(w_dict_t *d, const char *key);
W_EXPORT void  w_dict_set(w_dict_t *d, const char *key, void *data);
W_EXPORT void* w_dict_get(const w_dict_t *d, const char *key);

W_EXPORT void  w_dict_update(w_dict_t *d, const w_dict_t *o);

W_EXPORT void w_dict_traverse(w_dict_t *d, w_traverse_fun_t f, void *ctx);
W_EXPORT void w_dict_traverse_keys(w_dict_t *d, w_traverse_fun_t f, void *ctx);
W_EXPORT void w_dict_traverse_values(w_dict_t *d, w_traverse_fun_t f, void *ctx);

W_EXPORT w_iterator_t w_dict_first(const w_dict_t *d);
W_EXPORT w_iterator_t w_dict_next(const w_dict_t *d, w_iterator_t i);
W_EXPORT const char const* w_dict_iterator_get_key(w_iterator_t i);



/*
 * XXX: Never, NEVER change the layout of this structure. This **MUST**
 *      follow w_dict_node_t defined in wopt.c.
 * XXX: They key is totally unmodifiable, as it is insane to change it while
 *      traversing the
 */
struct w_dict_item
{
	void *val;
	const char const* key;
};
typedef struct w_dict_item w_dict_item_t;


#define w_dict_item_first(_d) \
	((w_dict_item_t*) w_dict_first(_d))

#define w_dict_item_next(_d, _i) \
	((w_dict_item_t*) w_dict_next((_d), (w_iterator_t)(_i)))

#endif /* !__wdict_h__ */


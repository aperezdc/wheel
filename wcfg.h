/*
 * wcfg.h
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __wcfg_h__
#define __wcfg_h__

#include "wdef.h"
#include "wdict.h"
#include <stdio.h>

typedef w_dict_t w_cfg_t;

enum w_cfg_type
{
	/* Marks en of parameter list for set/get. */
	W_CFG_END       = 0,

	/* Types for element nodes. */
	W_CFG_NONE      = 0,
	W_CFG_STRING    = 0x01,
	W_CFG_NUMBER    = 0x02,
	W_CFG_NODE      = 0x04,

	/* Flags for set/get. */
	W_CFG_SET       = 0x80,
	W_CFG_DEFAULT   = 0x40,

	/* Mask for the type. */
	W_CFG_TYPE_MASK =  W_CFG_STRING | W_CFG_NUMBER | W_CFG_NODE,

	/* Mask for the flags use in get/set. */
	W_CFG_FLAG_MASK = ~W_CFG_TYPE_MASK,
};

typedef enum w_cfg_type w_cfg_type_t;


W_EXPORT w_cfg_t* w_cfg_new(void);
W_EXPORT void w_cfg_free(w_cfg_t *cf);

W_EXPORT wbool w_cfg_has(const w_cfg_t *cf, const char *key);
W_EXPORT wbool w_cfg_del(w_cfg_t *cf, const char *key);
W_EXPORT wbool w_cfg_set(w_cfg_t *cf, ...);
W_EXPORT wbool w_cfg_get(const w_cfg_t *cf, ...);
W_EXPORT w_cfg_type_t w_cfg_type(const w_cfg_t *cf, const char *key);
W_EXPORT wbool w_cfg_dump(const w_cfg_t *cf, FILE *output);

#define w_cfg_set_string(cf, key, val) \
	w_cfg_set(cf, W_CFG_STRING, key, val, W_CFG_END)

#define w_cfg_set_number(cf, key, val) \
	w_cfg_set(cf, W_CFG_NUMBER, key, val, W_CFG_END)

#define w_cfg_set_node(cf, key, val) \
	w_cfg_set(cf, W_CFG_NODE, key, val, W_CFG_END)


#define _G(func_name, conf_type, c_type)              \
	static inline c_type w_cfg_get_ ## func_name         \
	(w_cfg_t *cf, const char *key, c_type defval)         \
	{ c_type value;                                        \
		if (w_cfg_get(cf, conf_type, key, &value, W_CFG_END)) \
			return value; else return defval;	}

_G( number, W_CFG_NUMBER, double      )
_G( string, W_CFG_STRING, const char* )
_G( node,   W_CFG_NODE,   w_cfg_t*    )

#undef _G


#define w_cfg_isnone(cf, key)    (W_CFG_NONE   == w_cfg_type(cf, key))
#define w_cfg_isnode(cf, key)    (W_CFG_NODE   == w_cfg_type(cf, key))
#define w_cfg_isnumber(cf, key)  (W_CFG_NUMBER == w_cfg_type(cf, key))
#define w_cfg_isstring(cf, key)  (W_CFG_STRING == w_cfg_type(cf, key))

#include "wcfg-parser.h"

#endif /* !__wcfg_h__ */


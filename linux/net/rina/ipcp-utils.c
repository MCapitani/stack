/*
 * IPC Processes related utilities
 *
 *    Francesco Salvestrini <f.salvestrini@nextworks.it>
 *    Sander Vrijders <sander.vrijders@intec.ugent.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/export.h>

#define RINA_PREFIX "ipcp-utils"

#include "logs.h"
#include "common.h"
#include "ipcp-utils.h"
#include "utils.h"
#include "debug.h"

struct name * name_create(void)
{ return rkzalloc(sizeof(struct name), GFP_KERNEL); }
EXPORT_SYMBOL(name_create);

/*
 * NOTE:
 *
 * No needs to export the following string_* symbols for the time being. They
 * will be grouped here and moved into their own placeholder later on (as well
 * as all the "common" utilities). Lot of them should even be dropped ...
 *
 *   Francesco
 */
static int string_dup(const string_t * src, string_t ** dst)
{
        ASSERT(dst);

        /*
         * An empty source is allowed (ref. the chain of calls) and it must
         * provoke no consequeunces
         */
        if (src) {
                *dst = kstrdup(src, GFP_KERNEL);
                if (!*dst) {
                        LOG_ERR("Cannot duplicate source string");
                        return -1;
                }
        } else
                *dst = NULL;

        return 0;
}

static int string_cmp(const string_t * a, const string_t * b)
{ return strcmp(a, b); }

/* FIXME: Should we assert here ? */
static int string_len(const string_t * s)
{ return strlen(s); }

#ifdef CONFIG_RINA_DEBUG
static int name_is_initialized(struct name * dst)
{
        ASSERT(dst);

        if (!dst->process_name     &&
            !dst->process_instance &&
            !dst->entity_name      &&
            !dst->entity_instance)
                return 1;
        return 0;
}
#endif

struct name * name_init(struct name *    dst,
                        const string_t * process_name,
                        const string_t * process_instance,
                        const string_t * entity_name,
                        const string_t * entity_instance)
{
        if (!dst)
                return NULL;

        /* Clean up the destination, leftovers might be there ... */
        name_fini(dst);

        ASSERT(name_is_initialized(dst));

        /* Boolean shortcuits ... */
        if (string_dup(process_name,     &dst->process_name)     ||
            string_dup(process_instance, &dst->process_instance) ||
            string_dup(entity_name,      &dst->entity_name)      ||
            string_dup(entity_instance,  &dst->entity_instance)) {
                name_fini(dst);
                return NULL;
        }
        
        return dst;
}
EXPORT_SYMBOL(name_init);

void name_fini(struct name * n)
{
        ASSERT(n);

        if (n->process_name) {
                rkfree(n->process_name);
                n->process_name = NULL;
        }
        if (n->process_instance) {
                rkfree(n->process_instance);
                n->process_instance = NULL;
        }
        if (n->entity_name) {
                rkfree(n->entity_name);
                n->entity_name = NULL;
        }
        if (n->entity_instance) {
                rkfree(n->entity_instance);
                n->entity_instance = NULL;
        }
}
EXPORT_SYMBOL(name_fini);

void name_destroy(struct name * ptr)
{
        ASSERT(ptr);

        name_fini(ptr);

        ASSERT(name_is_initialized(ptr));

        rkfree(ptr);
}
EXPORT_SYMBOL(name_destroy);

struct name * name_create_and_init(const string_t * process_name,
                                   const string_t * process_instance,
                                   const string_t * entity_name,
                                   const string_t * entity_instance)
{
        struct name * tmp1 = name_create();
        struct name * tmp2;

        if (!tmp1)
                return NULL;
        tmp2 = name_init(tmp1,
                         process_name,
                         process_instance,
                         entity_name,
                         entity_instance);
        if (!tmp2) {
                name_destroy(tmp1);
                return NULL;
        }

        return tmp2;
}
EXPORT_SYMBOL(name_create_and_init);

int name_cpy(const struct name * src, struct name * dst)
{
        if (!src || !dst)
                return -1;

        name_fini(dst);

        ASSERT(name_is_initialized(dst));

        /* We rely on short-boolean evaluation ... :-) */
        if (string_dup(src->process_name,     &dst->process_name)     ||
            string_dup(src->process_instance, &dst->process_instance) ||
            string_dup(src->entity_name,      &dst->entity_name)      ||
            string_dup(src->entity_instance,  &dst->entity_instance)) {
                name_fini(dst);
                return -1;
        }

        return 0;
}
EXPORT_SYMBOL(name_cpy);

struct name * name_dup(const struct name * src)
{
        struct name * tmp;

        if (!src)
                return NULL;

        tmp = name_create();
        if (!tmp)
                return NULL;
        if (name_cpy(src, tmp)) {
                name_destroy(tmp);
                return NULL;
        }

        return tmp;
}
EXPORT_SYMBOL(name_dup);

#define NAME_CMP_FIELD(X, Y, FIELD)                                     \
	((X->FIELD && Y->FIELD) ? string_cmp(X->FIELD, Y->FIELD) : -1)

int name_cmp(const struct name * a, const struct name * b)
{
        if (!a || !b)
                return -1;

        if (NAME_CMP_FIELD(a, b, process_name))
                return -1;
        if (NAME_CMP_FIELD(a, b, process_instance))
                return -1;
        if (NAME_CMP_FIELD(a, b, entity_name))
                return -1;
        if (NAME_CMP_FIELD(a, b, entity_instance))
                return -1;

        return 0;
}
EXPORT_SYMBOL(name_cmp);

char * name_tostring(const struct name * n)
{
        char *       tmp;
        size_t       size;
        const char * none = "<NONE>";
        size_t       none_len = strlen(none);

        if (!n)
                return NULL;
        
        size  = 0;

        size += (n->process_name                 ?
                 string_len(n->process_name)     : none_len);
        size += 1; /* SEPARATOR */

        size += (n->process_instance             ?
                 string_len(n->process_instance) : none_len);
        size += 1;  /* SEPARATOR */

        size += (n->entity_name                  ?
                 string_len(n->entity_name)      : none_len);
        size += 1;  /* SEPARATOR */

        size += (n->entity_instance              ?
                 string_len(n->entity_instance)  : none_len);
        size += 1;  /* TERMINATOR */
        
        tmp = rkmalloc(size, GFP_KERNEL);
        if (!tmp)
                return NULL;

        if (snprintf(tmp, size, "%s/%s/%s/%s",
                     (n->process_name     ? n->process_name     : none),
                     (n->process_instance ? n->process_instance : none),
                     (n->entity_name      ? n->entity_name      : none),
                     (n->entity_instance  ? n->entity_instance  : none)) !=
            size - 1) {
                rkfree(tmp);
                return NULL;
        }

        return tmp;
}
EXPORT_SYMBOL(name_tostring);


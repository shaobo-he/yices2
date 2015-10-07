/*
 * Hash consing for arrays of integers
 */

#ifndef __INT_ARRAY_HSETS_H
#define __INT_ARRAY_HSETS_H

#include <stdint.h>


/*
 * Array descriptor:
 * - hash = hash code
 * - nelems = number of elements
 * - data = array of elements, sorted in increasing order
 */
typedef struct harray_s {
  uint32_t hash;
  uint32_t nelems;
  int32_t data[0]; // real size = nelems
} harray_t;

#define MAX_HARRAY_SIZE ((UINT32_MAX-sizeof(harray_t))/sizeof(int32_t))


/*
 * Table of arrays for hash consing
 */
typedef struct int_array_hset_s {
  harray_t **data;
  uint32_t size;
  uint32_t nelems;
  uint32_t ndeleted;
  uint32_t resize_threshold;
  uint32_t cleanup_threshold;
} int_array_hset_t;


/*
 * Default and max size
 */
#define DEF_INT_ARRAY_HSET_SIZE 64
#define MAX_INT_ARRAY_HSET_SIZE (UINT32_MAX/sizeof(harray_t*))

/*
 * Ratios: resize_threshold = size * RESIZE_RATIO
 *         cleanup_threshold = size * CLEANUP_RATIO
 */
#define INT_ARRAY_HSET_RESIZE_RATIO 0.6
#define INT_ARRAY_HSET_CLEANUP_RATIO 0.2

/*
 * Marker for deleted sets
 */
#define DELETED_HARRAY ((harray_t *) 1)


/*
 * Initialize a set
 * - n = initial size: must be a power of 2
 * - if n = 0 the default size is used
 */
extern void init_int_array_hset(int_array_hset_t *set, uint32_t n);


/*
 * Delete
 */
extern void delete_int_array_hset(int_array_hset_t *set);


/*
 * Empty the set
 */
extern void reset_int_array_hset(int_array_hset_t *set);


/*
 * Find array a[0 .. n-1]
 * - a[0 ... n-1] must be an array of n elements
 * - return NULL if it's not in the set
 */
extern harray_t *int_array_hset_find(int_array_hset_t *set, uint32_t n, int32_t *a);


/*
 * Get descriptor for a[0 ... n-1]. Create it if it's not in set already.
 */
extern harray_t *int_array_hset_get(int_array_hset_t *set, uint32_t n, int32_t *a);


/*
 * Remove set a[0...n-1] from the set
 * - no change if a is not present
 */
extern void int_array_hset_remove(int_array_hset_t *set, uint32_t n, int32_t *a);


#endif /* __INT_ARRAY_HSETS_H */
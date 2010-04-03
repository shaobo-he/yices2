/*
 * BUFFERS FOR OPERATIONS ON BIT-VECTOR POLYNOMIALS
 */

/*
 * Bitvector polynomials are sums of pairs <coeff, pp>
 * - the coeff is a bitvector constant
 * - pp is a power product (cf. power_products.h)
 * - all coefficients have the same bit-size
 * - the data structure in this module support arbitrary
 *   bit size; coefficients are pointers to 32bit words
 *   (cf. bv_constants.h).
 *
 * In normal form, polynomials have the following properties:
 * - the coefficients are all reduced modulo 2^n 
 *   and are all non zero
 * - the monomials are stored in deg-lex order: lower degree
 *   monomials appear first; monomials of equal degree are
 *   sorted in lexicographic order.
 */

#ifndef __BVARITH_BUFFERS_H
#define __BVARITH_BUFFERS_H

#include <stdint.h>

#include "bv_constants.h"
#include "object_stores.h"
#include "pprod_table.h"


/*
 * A buffer stores a polynomial as a list of monomials
 * - list = list of pairs <coeff, pp> sorted in increasing order

 *   - coeff is a pointer to an array of 32bit words
 *   - pp is a power product
 * - the list contains an end marker (with pp = end_pp),
 *   which is always the last element in the list.
 * - some of the coefficients may be zero
 * - the normalize operation reduces all the coefficients modulo 2^n
 *   and remove the zero coefficients.
 *
 * Other components of a buffer:
 * - nterms = number of monomials in the list, excluding end marker.
 * - bitsize = size of the coefficients (number of bits).
 * - width = ceil(bitsize/32) = size of the coefficients in number words
 */

// element in a list of monomials
typedef struct bvmlist_s {
  struct bvmlist_s *next;
  uint32_t *coeff;
  pprod_t *prod;
} bvmlist_t;

// buffer
typedef struct bvarith_buffer_s {
  uint32_t nterms;        // length of the list (excluding the end marker)
  uint32_t bitsize;
  uint32_t width;
  bvmlist_t *list;        // start of the list 
  object_store_t *store;  // for allocation of list elements
  pprod_table_t *ptbl;    // for creation of power products
} bvarith_buffer_t;


/*
 * Block size of an bvarith_buffer store
 */
#define BVMLIST_BANK_SIZE 64



/***********************
 * BUFFER  OPERATIONS  *
 **********************/

/*
 * Initialize store s for list elements
 */
extern void init_bvmlist_store(object_store_t *s);


/*
 * Delete store s: free all attached memory and clear all rationals.
 * Must not be called unless all buffers with store s are deleted.
 */
extern void delete_bvmlist_store(object_store_t *s);

/*
 * Initialize buffer b to the zero polynomial
 * - ptbl = attached power product table
 * - s = attached store
 */
extern void init_bvarith_buffer(bvarith_buffer_t *b, pprod_table_t *ptbl, object_store_t *s);


/*
 * Delete b and free all attached memory
 */
extern void delete_bvarith_buffer(bvarith_buffer_t *b);


/*
 * Prepare b for a polynomial of n bits
 * - n must be positive
 * - this clears the current content of b  and 
 *   sets b to the 0 polynomial of n bits
 */
extern void bvarith_buffer_prepare(bvarith_buffer_t *b, uint32_t n);


/*
 * Normalize b:
 * - reduce all coefficients modulo 2^bitsize
 * - remove monomials with a zero coefficient
 */
extern void bvarith_buffer_normalize(bvarith_buffer_t *b);



/*************
 *  QUERIES  *
 ************/

/*
 * Size = number of terms.
 */
static inline uint32_t bvarith_buffer_size(bvarith_buffer_t *b) {
  return b->nterms;
}


/*
 * Bitsize
 */
static inline uint32_t bvarith_buffer_bitsize(bvarith_buffer_t *b) {
  return b->bitsize;
}


/*
 * Width = size of coefficients in 32bit words
 */
static inline uint32_t bvarith_buffer_width(bvarith_buffer_t *b) {
  return b->width;
}


/*
 * Check whether b is zero
 * - b must be normalized
 */
static inline bool bvarith_buffer_is_zero(bvarith_buffer_t *b) {
  return b->nterms == 0;
}


/*
 * Check whether b is constant
 * - b must be normalized
 */
extern bool bvarith_buffer_is_constant(bvarith_buffer_t *b);


/*
 * Read the constant term of b as a 64bit integer.
 * - b's bitsize must be between 1 and 64
 * - b must be normalized
 */
extern uint64_t bvarith_buffer_get_constant64(bvarith_buffer_t *b);


/*
 * Copy constant term of b into c.
 * - b must be normalized.
 * - the returned constant has bitsize equal to b->size.
 */
extern void bvarith_buffer_copy_constant(bvarith_buffer_t *b, bvconstant_t *c);


/*
 * Get the degree of polynomial in buffer b.
 * - b must be normalized
 * - returns 0 if b is zero
 */
extern uint32_t bvarith_buffer_degree(bvarith_buffer_t *b);


/*
 * Main term = maximal power product of b in the deg-lex ordering.
 * - b must be normalized and non zero
 */
extern pprod_t *bvarith_buffer_main_term(bvarith_buffer_t *b);


/*
 * Main monomial = monomial whose pp is the main term
 * - b must be normalized and non zero
 * - this returns the last element in b's monomial list
 */
extern bvmlist_t *bvarith_buffer_main_mono(bvarith_buffer_t *b);


/*
 * Check whether b1 and b2 are equal
 * - both must be normalized and use the same ptbl
 */
extern bool bvarith_buffer_equal(bvarith_buffer_t *b1, bvarith_buffer_t *b2);



/******************************
 *  POLYNOMIAL CONSTRUCTION   *
 *****************************/

/*
 * All operations update the first argument b.
 * They do not ensure that b is normalized.
 *
 * Some operations have a power product r as argument.
 * The power product r must be defined in b's internal 
 * power-product table (i.e., either r is empty_pp, or 
 * r is a tagged variable, or r occurs in b->ptbl).
 *
 * For operations that have a bitvector constant a as an argument,
 * the constant a must have the same bitsize as b.
 * 
 * Some operations use one or two other buffers b1 and b2.  In such
 * cases, b, b1, and b2 must all have the same power-product table
 * and must all have the same bitszie.
 */

/*
 * Assign the constant +1 or -1 to b.
 * This is extended to n bits where n = bitsize of b.
 */
extern void bvarith_buffer_set_one(bvarith_buffer_t *b);
extern void bvarith_buffer_set_minus_one(bvarith_buffer_t *b);


/*
 * Multiply b by -1
 */
extern void bvarith_buffer_negate(bvarith_buffer_t *b);


/*
 * Multiply b by constant a
 */
extern void bvarith_buffer_mul_const(bvarith_buffer_t *b, uint32_t *a);


/*
 * Multiply b by power product r
 */
extern void bvarith_buffer_mul_pp(bvarith_buffer_t *b, pprod_t *r);


/*
 * Multiply b by (- r)
 */
extern void bvarith_buffer_mul_negpp(bvarith_buffer_t *b, pprod_t *r);


/*
 * Multiply b by a * r
 */
extern void bvarith_buffer_mul_mono(bvarith_buffer_t *b, uint32_t *a, pprod_t *r);



/*
 * Add constant a to b
 */
extern void bvarith_buffer_add_const(bvarith_buffer_t *b, uint32_t *a);


/*
 * Add constant (-a) to b
 */
extern void bvarith_buffer_sub_const(bvarith_buffer_t *b, uint32_t *a);


/*
 * Add r to b
 */
extern void bvarith_buffer_add_pp(bvarith_buffer_t *b, pprod_t *r);


/*
 * Add -r to b
 */
extern void bvarith_buffer_sub_pp(bvarith_buffer_t *b, pprod_t *r);


/*
 * Add a * r to b
 */
extern void bvarith_buffer_add_mono(bvarith_buffer_t *b, uint32_t *a, pprod_t *r);


/*
 * Add -a * r to b
 */
extern void bvarith_buffer_sub_mono(bvarith_buffer_t *b, uint32_t *a, pprod_t *r);


/*
 * Add b1 to b
 */
extern void bvarith_buffer_add_buffer(bvarith_buffer_t *b, bvarith_buffer_t *b1);


/*
 * Add (-b1) to b
 */
extern void bvarith_buffer_sub_buffer(bvarith_buffer_t *b, bvarith_buffer_t *b1);


/*
 * Multiply b by b1
 * - b1 must be different from b
 */
extern void bvarith_buffer_mul_buffer(bvarith_buffer_t *b, bvarith_buffer_t *b1);


/*
 * Compute the square of b
 */
extern void bvarith_buffer_square(bvarith_buffer_t *b);


/*
 * Add a * b1 to b
 */
extern void bvarith_buffer_add_const_times_buffer(bvarith_buffer_t *b, bvarith_buffer_t *b1, uint32_t *a);


/*
 * Add (-a) * b1 to b
 */
extern void bvarith_buffer_sub_const_times_buffer(bvarith_buffer_t *b, bvarith_buffer_t *b1, uint32_t *a);


/*
 * Add r * b1 to b
 */
extern void bvarith_buffer_add_pp_times_buffer(bvarith_buffer_t *b, bvarith_buffer_t *b1, pprod_t *r);


/*
 * Add - r * b1 to b
 */
extern void bvarith_buffer_sub_pp_times_buffer(bvarith_buffer_t *b, bvarith_buffer_t *b1, pprod_t *r);


/*
 * Add a * r * b1 to b
 */
extern void bvarith_buffer_add_mono_times_buffer(bvarith_buffer_t *b, bvarith_buffer_t *b1, uint32_t *a, pprod_t *r);

/*
 * Add -a * r * b1 to b
 */
extern void bvarith_buffer_sub_mono_times_buffer(bvarith_buffer_t *b, bvarith_buffer_t *b1, uint32_t *a, pprod_t *r);


/*
 * Add b1 * b2 to b
 * - b1 and b2 must be distinct from b (but b1 may be equal to b2)
 */
extern void bvarith_buffer_add_buffer_times_buffer(bvarith_buffer_t *b, bvarith_buffer_t *b1, bvarith_buffer_t *b2);


/*
 * Add - b1 * b2 to b
 * - b1 and b2 must be distinct from b (but b1 may be equal to b2)
 */
extern void bvarith_buffer_sub_buffer_times_buffer(bvarith_buffer_t *b, bvarith_buffer_t *b1, bvarith_buffer_t *b2);




/*
 * SHORT CUTS
 */

/*
 * All operations that take a power product r have a variant that takes a single
 * variable x as argument.
 */

/*
 * Multiply b by x
 */
static inline void bvarith_buffer_mul_var(bvarith_buffer_t *b, int32_t x) {
  bvarith_buffer_mul_pp(b, var_pp(x));
}


/*
 * Multiply b by (- x)
 */
static inline void bvarith_buffer_mul_negvar(bvarith_buffer_t *b, int32_t x) {
  bvarith_buffer_mul_negpp(b, var_pp(x));
}


/*
 * Multiply b by a * x
 */
static inline void bvarith_buffer_mul_varmono(bvarith_buffer_t *b, uint32_t *a, int32_t x) {
  bvarith_buffer_mul_mono(b, a, var_pp(x));
}


/*
 * Add x to b
 */
static inline void bvarith_buffer_add_var(bvarith_buffer_t *b, int32_t x) {
  bvarith_buffer_add_pp(b, var_pp(x));
}


/*
 * Add -x to b
 */
static inline void bvarith_buffer_sub_var(bvarith_buffer_t *b, int32_t x) {
  bvarith_buffer_sub_pp(b, var_pp(x));
}


/*
 * Add a * x to b
 */
static inline void bvarith_buffer_add_varmono(bvarith_buffer_t *b, uint32_t *a, int32_t x) {
  bvarith_buffer_add_mono(b, a, var_pp(x));
}


/*
 * Add -a * x to b
 */
static inline void bvarith_buffer_sub_varmono(bvarith_buffer_t *b, uint32_t *a, int32_t x) {
  bvarith_buffer_sub_mono(b, a, var_pp(x));
}


/*
 * Add x * b1 to b
 */
static inline void bvarith_buffer_add_var_times_buffer(bvarith_buffer_t *b, bvarith_buffer_t *b1, int32_t x) {
  bvarith_buffer_add_pp_times_buffer(b, b1, var_pp(x));
}


/*
 * Add - x * b1 to b
 */
static inline void bvarith_buffer_sub_var_times_buffer(bvarith_buffer_t *b, bvarith_buffer_t *b1, int32_t x) {
  bvarith_buffer_sub_pp_times_buffer(b, b1, var_pp(x));
}


/*
 * Add a * x * b1 to b
 */
static inline void 
bvarith_buffer_add_varmono_times_buffer(bvarith_buffer_t *b, bvarith_buffer_t *b1, uint32_t *a, int32_t x) {
  bvarith_buffer_add_mono_times_buffer(b, b1, a, var_pp(x));
}

/*
 * Add -a * x * b1 to b
 */
static inline void
bvarith_buffer_sub_varmono_times_buffer(bvarith_buffer_t *b, bvarith_buffer_t *b1, uint32_t *a, int32_t x) {
  bvarith_buffer_sub_mono_times_buffer(b, b1, a, var_pp(x));
}



#endif /* __BVARITH_BUFFERS_H */

/*
 * THE SEARCH PARAMETERS/OPTIONS USED BY THE SOLVER
 */

#include <stdbool.h>
#include <assert.h>
#include <stdint.h>

#include "context.h"



/*******************************
 *  DEFAULT SEARCH PARAMETERS  *
 ******************************/

/*
 * Default restart parameters for SMT solving
 * Minisat-like behavior
 */
#define DEFAULT_FAST_RESTART false
#define DEFAULT_C_THRESHOLD  100
#define DEFAULT_D_THRESHOLD  100
#define DEFAULT_C_FACTOR     1.5
#define DEFAULT_D_FACTOR     1.5

/*
 * Restart parameters if option --fast-restarts is set
 */
#define FAST_RESTART_C_THRESHOLD 100
#define FAST_RESTART_D_THRESHOLD 100
#define FAST_RESTART_C_FACTOR    1.1
#define FAST_RESTART_D_FACTOR    1.1

/*
 * Default clause deletion parameters
 */
#define DEFAULT_R_THRESHOLD   1000
#define DEFAULT_R_FRACTION    0.25
#define DEFAULT_R_FACTOR      1.05


/*
 * The default SMT parameters are copied from smt_core.h
 * - VAR_DECAY_FACTOR  = 0.95
 * - VAR_RANDOM_FACTOR = 0.02
 * - CLAUSE_DECAY_FACTOR = 0.999
 * - clause caching is disabled
 */
#define DEFAULT_VAR_DECAY      VAR_DECAY_FACTOR
#define DEFAULT_RANDOMNESS     VAR_RANDOM_FACTOR
#define DEFAULT_CLAUSE_DECAY   CLAUSE_DECAY_FACTOR
#define DEFAULT_CACHE_TCLAUSES false
#define DEFAULT_TCLAUSE_SIZE   0

/*
 * Default branching = the smt_core default
 */
#define DEFAULT_BRANCHING  BRANCHING_DEFAULT

/*
 * The default EGRAPH parameters are defined in egraph_types.h
 * - DEFAULT_MAX_ACKERMANN = 1000
 * - DEFAULT_MAX_BOOLACKERMANN = 600000
 * - DEFAULT_AUX_EQ_QUOTA = 100
 * - DEFAULT_ACKERMANN_THRESHOLD = 8
 * - DEFAULT_BOOLACK_THRESHOLD = 8
 * - DEFAULT_MAX_INTERFACE_EQS = 200
 *
 * The dynamic ackermann heuristic is disabled for both 
 * boolean and non-boolean terms.
 */
#define DEFAULT_USE_DYN_ACK        false
#define DEFAULT_USE_BOOL_DYN_ACK   false
#define DEFAULT_AUX_EQ_RATIO       0.3


/*
 * Default SIMPLEX parameters defined in simplex_types.h
 * - SIMPLEX_DEFAULT_BLAND_THRESHOLD = 1000
 * - SIMPLEX_DEFAULT_PROP_ROW_SIZE = 30
 * - SIMPLEX_DEFAULT_CHECK_PERIOD = infinity
 * - propagation is disabled by default
 * - model adjustment is also disabled
 */
#define DEFAULT_SIMPLEX_PROP_FLAG     false
#define DEFAULT_SIMPLEX_ADJUST_FLAG   false


/*
 * Default parameters for the array solver (defined in fun_solver.h
 * - MAX_UPDATE_CONFLICTS = 20
 * - MAX_EXTENSIONALITY = 1
 */


/*
 * All default parameters
 */
static param_t default_settings = {
  DEFAULT_FAST_RESTART,
  DEFAULT_C_THRESHOLD,
  DEFAULT_D_THRESHOLD,
  DEFAULT_C_FACTOR,
  DEFAULT_D_FACTOR,

  DEFAULT_R_THRESHOLD,
  DEFAULT_R_FRACTION,
  DEFAULT_R_FACTOR,

  DEFAULT_VAR_DECAY,
  DEFAULT_RANDOMNESS,
  DEFAULT_BRANCHING,
  DEFAULT_CLAUSE_DECAY,
  DEFAULT_CACHE_TCLAUSES,
  DEFAULT_TCLAUSE_SIZE,

  DEFAULT_USE_DYN_ACK,
  DEFAULT_USE_BOOL_DYN_ACK,
  DEFAULT_MAX_ACKERMANN,
  DEFAULT_MAX_BOOLACKERMANN,
  DEFAULT_AUX_EQ_QUOTA,
  DEFAULT_AUX_EQ_RATIO,
  DEFAULT_ACKERMANN_THRESHOLD,
  DEFAULT_BOOLACK_THRESHOLD,
  DEFAULT_MAX_INTERFACE_EQS,

  DEFAULT_SIMPLEX_PROP_FLAG,
  DEFAULT_SIMPLEX_ADJUST_FLAG,
  SIMPLEX_DEFAULT_PROP_ROW_SIZE,
  SIMPLEX_DEFAULT_BLAND_THRESHOLD,
  SIMPLEX_DEFAULT_CHECK_PERIOD,

  DEFAULT_MAX_UPDATE_CONFLICTS,
  DEFAULT_MAX_EXTENSIONALITY,
};



/*************************************
 *  GLOBAL TABLES: PARAMETER NAMES   *
 ************************************/

/*
 * Search parameters and options can be set individually.
 *
 * We use an integer code to identify the parameters + a table of
 * parameter names in lexicographic order. Each parameter 
 * is described in context.h
 */
typedef enum param_key {
  // restart parameters
  PARAM_FAST_RESTART,
  PARAM_C_THRESHOLD,
  PARAM_D_THRESHOLD,
  PARAM_C_FACTOR,
  PARAM_D_FACTOR,
  // clause deletion heuristic
  PARAM_R_THRESHOLD,
  PARAM_R_FRACTION,
  PARAM_R_FACTOR,
  // branching heuristic
  PARAM_VAR_DECAY,
  PARAM_RANDOMNESS,
  PARAM_BRANCHING,
  // learned clauses
  PARAM_CLAUSE_DECAY,
  PARAM_CACHE_TCLAUSES,
  PARAM_TCLAUSE_SIZE,
  // egraph parameters
  PARAM_DYN_ACK,
  PARAM_DYN_BOOL_ACK,
  PARAM_MAX_ACK,
  PARAM_MAX_BOOL_ACK,
  PARAM_AUX_EQ_QUOTA,
  PARAM_AUX_EQ_RATIO,
  PARAM_MAX_INTERFACE_EQS,
  // simplex parameters
  PARAM_EAGER_LEMMAS,
  PARAM_SIMPLEX_PROP,
  PARAM_SIMPLEX_ADJUST,
  PARAM_PROP_THRESHOLD,
  PARAM_BLAND_THRESHOLD,
  PARAM_ICHECK,
  PARAM_ICHECK_PERIOD,
} param_key_t;

#define NUM_PARAM_KEYS (PARAM_ICHECK_PERIOD+1)

// parameter names in lexigraphic ordering
static const char *const param_key_names[NUM_PARAM_KEYS] = {
  "aux-eq-quota",
  "aux-eq-ratio",
  "bland-threshold",
  "branching",
  "c-factor",
  "c-threshold",
  "cache-tclauses",
  "clause-decay",
  "d-factor",
  "d-threshold",
  "dyn-ack",
  "dyn-bool-ack",
  "eager-lemmas",
  "fast-restarts",
  "icheck",
  "icheck-period",
  "max-ack",
  "max-bool-ack",
  "max-interface-eqs",
  "prop-threshold",
  "r-factor",
  "r-fraction",
  "r-threshold",
  "random-seed",
  "randomness",
  "simplex-adjust",
  "simplex-prop",
  "tclause-size",
  "var-decay",
};

// corresponding parameter codes in order
static const yices_param_t param_code[NUM_PARAMETERS] = {
  PARAM_AUX_EQ_QUOTA,
  PARAM_AUX_EQ_RATIO,
  PARAM_BLAND_THRESHOLD,
  PARAM_BRANCHING,
  PARAM_C_FACTOR,
  PARAM_C_THRESHOLD,
  PARAM_CACHE_TCLAUSES,
  PARAM_CLAUSE_DECAY,
  PARAM_D_FACTOR,
  PARAM_D_THRESHOLD,
  PARAM_DYN_ACK,
  PARAM_DYN_BOOL_ACK,
  PARAM_EAGER_LEMMAS,
  PARAM_FAST_RESTART,
  PARAM_ICHECK,
  PARAM_ICHECK_PERIOD,
  PARAM_MAX_ACK,
  PARAM_MAX_BOOL_ACK,
  PARAM_MAX_INTERFACE_EQS,
  PARAM_PROP_THRESHOLD,
  PARAM_R_FACTOR,
  PARAM_R_FRACTION,
  PARAM_R_THRESHOLD,
  PARAM_RANDOM_SEED,
  PARAM_RANDOMNESS,
  PARAM_SIMPLEX_ADJUST,
  PARAM_SIMPLEX_PROP,
  PARAM_TCLAUSE_SIZE,
  PARAM_VAR_DECAY,
};



/*
 * Names of each branching mode (in lexicographic order)
 */
#define NUM_BRANCHING_MODES 6

static const char * const branching_modes[NUM_BRANCHING_MODES] = {
  "default",
  "negative",
  "positive",
  "th-neg",
  "th-pos",
  "theory",
};

static const branch_t branching_code[NUM_BRANCHING_MODES] = {
  BRANCHING_DEFAULT,
  BRANCHING_NEGATIVE,
  BRANCHING_POSITIVE,
  BRANCHING_TH_NEG,
  BRANCHING_TH_POS,
  BRANCHING_THEORY,
};




/****************
 *  FUNCTIONS   *
 ***************/

/*
 * Initialize params with a copy of default_settings
 */
void init_params_to_defaults(param_t *parameters) {
  *parameters = default_settings;
}


/*
 * Parse value as a boolean. Store the result in *v
 * - return 0 if this works
 * - return -1 otherwise
 */
static int32_t set_bool_param(const char *value, bool *v) {
  int32_t k;

  k = parse_as_boolean(value, v);
  return (k == valid_boolean) ? 0 : -1;
}


/*
 * Parse value as a branching mode. Store the result in *v
 * - return 0 if this works
 * - return -1 otherwise
 */
static int32_t set_branching_param(const char *value, branch_t *v) {
  int32_t k;

  k = parse_as_keyword(value, branching_modes, branching_code, NUM_BRANCHING_MODES);
  assert(k >= 0 || k == -1);

  if (k >= 0) {
    assert(BRANCHING_DEFAULT <= k && k <= BRANCHING_TH_POS);
    *v = (branch_t) k;
    k = 0;
  }

  return k;
}


/*
 * Parse val as a signed 32bit integer. Check whether 
 * the result is in the interval [low, high].
 * - if so, store the result into *v and return 0
 * - if val is not an integer, return -1
 * - if the result is not in the interval, return -2
 */
static int32_t set_int32_param(const char *value, int32_t *v, int32_t low, int32_t high) {
  integer_parse_code_t k;
  int32_t aux;

  k = parse_as_integer(value, &aux);
  switch (k) {
  case valid_integer:
    if (low <= aux && aux <= high) {
      *v = aux;
      aux = 0;
    } else {
      aux = -2;   
    }
    break;
  case integer_overflow:
    aux = -2;
    break;
  case invalid_integer:
    aux = -1;
    break;
  }

  return aux;
}




/*
 * Parse value as a double. Check whether 
 * the result is in the interval [low, high].
 * - if so, store the result into *v and return 0
 * - if the string can't be parse as a double, return -1
 * - if the result is not in the interval, return -2
 */
static int32_t set_double_param(const char *value, double *v, double low, double high) {
  double_parse_code_t k;
  double aux;
  int32_t result;

  k = parse_as_double(value, &aux);
  switch (k) {
  case valid_double:
    if (low <= aux && aux <= high) {
      *v = aux;
      result = 0;
    } else {
      result = -2;   
    }
    break;
  case double_overflow:
    result = -2;
    break;
  case invalid_double:
    result = -1;
    break;
  }

  return result;
}




/*
 * Set a field in the parameter record.
 * - key = field name
 * - value = value for that field
 *
 * Return code:
 *   -1 if the key is not recognized
 *   -2 if the value is not recognized
 *   -3 if the value has the wrong type for the key
 *    0 otherwise (i.e., success)
 */
int32_t params_set_field(param_t *parameters, const char *key, const char *value) {
  int32_t k, r;
  int32_t z;
  double x;

  k = parse_as_keyword(key, param_key_names, param_code, NUM_PARAMETERS);
  switch (k) {
  case PARAM_FAST_RESTART:
    r = set_bool_param(value, &parameters->fast_restart);
    break;

  case PARAM_C_THRESHOLD:
    r = set_int32_param(value, &z, 1, INT32_MAX);
    if (r == 0) {
      parameters->c_threshold = (uint32_t) z;
    }
    break;

  case PARAM_D_THRESHOLD:
    r = set_int32_param(value, &z, 1, INT32_MAX);
    if (r == 0) {
      parameters->d_threshold = (uint32_t) z;
    }
    break;

  case PARAM_C_FACTOR:
    r = set_double_param(value, &parameters->c_factor, 1.0, DOUBLE_MAX);
    break;

  case PARAM_D_FACTOR:
    r = set_double_param(value, &parameters->d_factor, 1.0, DOUBLE_MAX);
    break;

  case PARAM_R_THRESHOLD:
    r = set_int32_param(value, &z, 1, INT32_MAX);
    if (r == 0) {
      parameters->r_threshold = r;
    }
    break;

  case PARAM_R_FRACTION:
    r = set_double_param(value, &parameters->r_fraction, 0.0, 1.0);
    break;

  case PARAM_R_FACTOR:
    r = set_double_param(value, &parameters->r_factor, 1.0, DOUBLE_MAX);
    break;

  case PARAM_VAR_DECAY:
    r = set_double_param(value, &parameters->var_decay, 0.0, 1.0);
    break;

  case PARAM_RANDOMNESS:
    r = set_double_param(value, &x, 0.0, 1.0);
    if (r == 0) {
      parameters->randomness = (float) x;
    }
    break;

  default:
    assert(k == -1);
    r = -1;
    break;
  }

  return r;
}

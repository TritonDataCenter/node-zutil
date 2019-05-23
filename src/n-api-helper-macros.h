// This header provides some utility macros for working with the node.js 
// N-API (https://nodejs.org/api/n-api.html). It is copied from:
//
//      https://github.com/nodejs/node/blob/f2061930c83cb579410a7f26bed726d6568575e3/test/js-native-api/common.h
//  
// It looks to me like most/all usage of N-API in the node.js main
// repo uses these macros. From the many hits of `NAPI_CALL` on GitHub:
//
//      https://github.com/search?q=NAPI_CALL&type=Code
//
// I wonder if it is an unspoken rule that one really should be using
// wrappers like these. These aren't mentioned in the N-API docs and
// this part of the N-API docs is easy to miss (emphasis mine):
//
//      https://nodejs.org/api/n-api.html#n_api_error_handling
//      In cases where a return value other than napi_ok or 
//      napi_pending_exception is returned, napi_is_exception_pending *must* be
//      called to check if an exception is pending. See the section on 
//      exceptions for more details.


// Empty value so that macros here are able to return NULL or void
#define NAPI_RETVAL_NOTHING  // Intentionally blank #define

#define GET_AND_THROW_LAST_ERROR(env)                                    \
  do {                                                                   \
    const napi_extended_error_info *error_info;                          \
    napi_get_last_error_info((env), &error_info);                        \
    bool is_pending;                                                     \
    napi_is_exception_pending((env), &is_pending);                       \
    /* If an exception is already pending, don't rethrow it */           \
    if (!is_pending) {                                                   \
      const char* error_message = error_info->error_message != NULL ?    \
        error_info->error_message :                                      \
        "empty error message";                                           \
      napi_throw_error((env), NULL, error_message);                      \
    }                                                                    \
  } while (0)

#define NAPI_ASSERT_BASE(env, assertion, message, ret_val)               \
  do {                                                                   \
    if (!(assertion)) {                                                  \
      napi_throw_error(                                                  \
          (env),                                                         \
        NULL,                                                            \
          "assertion (" #assertion ") failed: " message);                \
      return ret_val;                                                    \
    }                                                                    \
  } while (0)

// Returns NULL on failed assertion.
// This is meant to be used inside napi_callback methods.
#define NAPI_ASSERT(env, assertion, message)                             \
  NAPI_ASSERT_BASE(env, assertion, message, NULL)

// Returns empty on failed assertion.
// This is meant to be used inside functions with void return type.
#define NAPI_ASSERT_RETURN_VOID(env, assertion, message)                 \
  NAPI_ASSERT_BASE(env, assertion, message, NAPI_RETVAL_NOTHING)

#define NAPI_CALL_BASE(env, the_call, ret_val)                           \
  do {                                                                   \
    if ((the_call) != napi_ok) {                                         \
      GET_AND_THROW_LAST_ERROR((env));                                   \
      return ret_val;                                                    \
    }                                                                    \
  } while (0)

// Returns NULL if the_call doesn't return napi_ok.
#define NAPI_CALL(env, the_call)                                         \
  NAPI_CALL_BASE(env, the_call, NULL)

// Returns empty if the_call doesn't return napi_ok.
#define NAPI_CALL_RETURN_VOID(env, the_call)                             \
  NAPI_CALL_BASE(env, the_call, NAPI_RETVAL_NOTHING)

#define DECLARE_NAPI_PROPERTY(name, func)                                \
  { (name), NULL, (func), NULL, NULL, NULL, napi_default, NULL }

#define DECLARE_NAPI_GETTER(name, func)                                  \
  { (name), NULL, NULL, (func), NULL, NULL, napi_default, NULL }

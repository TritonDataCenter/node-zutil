//
// Node.js C bindings for some of illumos' "zone.h" and "libzonecfg.h".
//
//
// Dev Notes:
// - "zu_" prefix is used to namespace identifiers in this file.
//

#include <assert.h>
#include <zone.h>
#include <libzonecfg.h>

#include <node_api.h>

#include "./n-api-helper-macros.h"

// 2048 chars should be enough error for anyone, right?
#define MAXERRMSGLEN 2048

//
// This is a convenience wrapper around `napi_throw_error` that supports
// printf-style arguments for the error msg.
//
napi_status _zu_napi_throw_error(napi_env env, const char* code, char *msg_format, ...)
{
    va_list ap;
    char msg[MAXERRMSGLEN + 1];

    assert(msg_format != NULL);

    va_start(ap, msg_format);
    (void) vsnprintf(msg, sizeof (msg), msg_format, ap);
    va_end(ap);

    return napi_throw_error(env, code, msg);
}



napi_value zu_getzoneid(napi_env env, napi_callback_info info) {
    size_t argc = 0;
    zone_id_t id;
    napi_value js_id;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, NULL, NULL, NULL));
    if (argc != 0) {
        napi_throw_type_error(env, NULL, "incorrect number of arguments");
        return NULL;
    }

    id = getzoneid();

    NAPI_CALL(env, napi_create_int64(env, id, &js_id));
    return js_id;
}


napi_value zu_getzonestate(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1];
    char zone_name[ZONENAME_MAX + 1];
    size_t num_bytes;
    zone_state_t state;
    char *state_str = NULL;
    int err;
    napi_value js_state_str;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != 1) {
        napi_throw_type_error(env, NULL, "incorrect number of arguments");
        return NULL;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], zone_name,
        ZONENAME_MAX+1, &num_bytes));

    if ((err = zone_get_state(zone_name, &state)) != Z_OK) {
        // TODO: Eventually would be nice to have the libzonecfg.h "Z_*"
        // names used as the error "code".
        _zu_napi_throw_error(env, NULL, "could not get zone \"%s\" state: %s",
            zone_name, zonecfg_strerror(err));
    }

    state_str = zone_state_str(state);

    NAPI_CALL(env, napi_create_string_utf8(env, state_str, NAPI_AUTO_LENGTH,
        &js_state_str));
    return js_state_str;
}


static napi_value Init(napi_env env, napi_value exports) {
    // - https://nodejs.org/api/n-api.html#n_api_napi_property_descriptor
    //   for fields descriptions.
    // - `0b010` is `napi_property_attributes` for enumerable, read-only.
    //   https://nodejs.org/api/n-api.html#n_api_napi_property_attributes
    napi_property_descriptor properties[] = {
        // zone.h
        { "getzoneid", NULL, zu_getzoneid, NULL, NULL, NULL, 0b010, NULL },

        // libzonecfg.h
        { "getzonestate", NULL, zu_getzonestate, NULL, NULL, NULL, 0b010, NULL },
    };

    NAPI_CALL(env, napi_define_properties(
          env, exports, sizeof(properties) / sizeof(*properties), properties));

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)

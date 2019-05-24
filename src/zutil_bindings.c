//
// Copyright 2019 Joyent, Inc.
//

//
// Node.js C bindings for some of illumos' "zone.h" and "libzonecfg.h".
//
//
// Dev Notes:
// - "zu_" prefix is used to namespace identifiers in this file.
//

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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
    zoneid_t id;
    napi_value nv_id;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, NULL, NULL, NULL));
    if (argc != 0) {
        napi_throw_type_error(env, NULL, "incorrect number of arguments");
        return NULL;
    }

    id = getzoneid();

    NAPI_CALL(env, napi_create_int32(env, id, &nv_id));
    return nv_id;
}


napi_value zu_getzoneidbyname(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1];
    zoneid_t id;
    size_t num_bytes;
    napi_value nv_id;
    char zonename[ZONENAME_MAX + 1];

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != 1) {
        napi_throw_type_error(env, NULL, "incorrect number of arguments");
        return NULL;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], zonename,
        sizeof(zonename), &num_bytes));

    if ((id = getzoneidbyname(zonename)) == -1) {
        _zu_napi_throw_error(env, NULL, "could not id for zonename \"%s\": %s",
            zonename, strerror(errno));
        return NULL;
    }

    NAPI_CALL(env, napi_create_int32(env, id, &nv_id));
    return nv_id;
}


napi_value zu_getzonenamebyid(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1];
    zoneid_t id;
    napi_value nv_zonename;
    int rv;
    char zonename[ZONENAME_MAX + 1];

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != 1) {
        napi_throw_type_error(env, NULL, "incorrect number of arguments");
        return NULL;
    }
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &id));

    if ((rv = getzonenamebyid(id, zonename, sizeof(zonename))) == -1) {
        _zu_napi_throw_error(env, NULL, "could not get zonename for id %d: %s",
            id, strerror(errno));
        return NULL;
    }

    NAPI_CALL(env, napi_create_string_utf8(env, zonename, NAPI_AUTO_LENGTH, 
        &nv_zonename));
    return nv_zonename;
}


napi_value zu_getzonename(napi_env env, napi_callback_info info) {
    size_t argc = 0;
    zoneid_t id;
    napi_value nv_zonename;
    int rv;
    char zonename[ZONENAME_MAX + 1];

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, NULL, NULL, NULL));
    if (argc != 0) {
        napi_throw_type_error(env, NULL, "incorrect number of arguments");
        return NULL;
    }

    id = getzoneid();
    if ((rv = getzonenamebyid(id, zonename, sizeof(zonename))) == -1) {
        _zu_napi_throw_error(env, NULL, "could not get current zonename: %s",
            id, strerror(errno));
        return NULL;
    }

    NAPI_CALL(env, napi_create_string_utf8(env, zonename, NAPI_AUTO_LENGTH, 
        &nv_zonename));
    return nv_zonename;
}


napi_value zu_getzonestate(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1];
    char zonename[ZONENAME_MAX + 1];
    size_t num_bytes;
    zone_state_t state;
    char *state_str = NULL;
    int err;
    napi_value nv_state_str;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != 1) {
        napi_throw_type_error(env, NULL, "incorrect number of arguments");
        return NULL;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], zonename,
        sizeof(zonename), &num_bytes));

    if ((err = zone_get_state(zonename, &state)) != Z_OK) {
        // TODO: Eventually would be nice to have the libzonecfg.h "Z_*"
        // names used as the error "code".
        _zu_napi_throw_error(env, NULL, "could not get zone \"%s\" state: %s",
            zonename, zonecfg_strerror(err));
        return NULL;
    }

    state_str = zone_state_str(state);

    NAPI_CALL(env, napi_create_string_utf8(env, state_str, NAPI_AUTO_LENGTH,
        &nv_state_str));
    return nv_state_str;
}


//
// Getting zone attributes
//
//      zonecfg.zone_get_attr(zone_name, attr_name, function (err, attr) {});
//      zonecfg.zone_get_attrs(zone_name, function (err, attrs) {});
//
// where each `attr` is an object with `name`, `type`, and `value` fields.
//

// From libzonecfg.h:
//    struct zone_attrtab {
//        char    zone_attr_name[MAXNAMELEN];
//        char    zone_attr_type[MAXNAMELEN];
//        char    zone_attr_value[2 * BUFSIZ];
//    };
typedef struct zone_attrtab zone_attrtab_t;



typedef struct {
    char zone_name[ZONENAME_MAX + 1];
    zone_attrtab_t attr;
    char errmsg[MAXERRMSGLEN + 1];
    napi_ref callback;
    napi_async_work work;
} _zu_getzoneattr_data_t;

// Fill in `data->attr` using `zonecfg_getattrent` et al.
void _zu_getzoneattr_execute(napi_env env, void* data_) {
    zone_attrtab_t attr;
    _zu_getzoneattr_data_t* data = (_zu_getzoneattr_data_t*) data_;
    zone_dochandle_t handle;
    int rc;

    if ((handle = zonecfg_init_handle()) == NULL) {
        // Guessing ENOMEM is the issue here (cargoculting from node-zutil).
        strlcat(data->errmsg, "zonecfg_init_handle error: ", MAXERRMSGLEN);
        strlcat(data->errmsg, zonecfg_strerror(Z_NOMEM), MAXERRMSGLEN);
        return;
    }
    if ((rc = zonecfg_get_handle(data->zone_name, handle)) != Z_OK) {
        strlcat(data->errmsg, data->zone_name, MAXERRMSGLEN);
        strlcat(data->errmsg, ": ", MAXERRMSGLEN);
        strlcat(data->errmsg, zonecfg_strerror(rc), MAXERRMSGLEN);
        goto fini;
    }
    if ((rc = zonecfg_setattrent(handle)) != Z_OK) {
        strlcat(data->errmsg, "zonecfg_setattrent error: ", MAXERRMSGLEN);
        strlcat(data->errmsg, zonecfg_strerror(rc), MAXERRMSGLEN);
        goto fini;
    }

    while (zonecfg_getattrent(handle, &attr) == Z_OK) {
        if (strncmp(data->attr.zone_attr_name, attr.zone_attr_name,
                sizeof(data->attr.zone_attr_name)) == 0) {
            strlcpy(data->attr.zone_attr_type, attr.zone_attr_type,
                sizeof(data->attr.zone_attr_type));
            strlcpy(data->attr.zone_attr_value, attr.zone_attr_value,
                sizeof(data->attr.zone_attr_value));
            break;
        }
    }
    (void) zonecfg_endattrent(handle);

fini:
    zonecfg_fini_handle(handle);
    return;
}

void _zu_getzoneattr_complete(napi_env env, napi_status status, void* data_) {
    napi_value args[2];
    napi_value attr;
    napi_value callback;
    _zu_getzoneattr_data_t* data = (_zu_getzoneattr_data_t*) data_;
    napi_value errmsg;
    napi_value global;
    napi_value k;
    napi_value result;
    napi_value v;

    if (status != napi_ok) {
        napi_throw_error(env, NULL, "zone_get_attr execute failed");
        return;
    }

    // Setup the args for the callback.
    if (strnlen(data->errmsg, MAXERRMSGLEN) != 0) {
        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(env, data->errmsg, NAPI_AUTO_LENGTH, 
                &errmsg));
        NAPI_CALL_RETURN_VOID(env,
            napi_create_error(env, NULL, errmsg, &args[0]));
        NAPI_CALL_RETURN_VOID(env,
            napi_get_null(env, &args[1]));
    } else if (strnlen(data->attr.zone_attr_type, MAXNAMELEN) == 0) {
        NAPI_CALL_RETURN_VOID(env,
            napi_get_null(env, &args[0]));
        NAPI_CALL_RETURN_VOID(env,
            napi_get_null(env, &args[1]));
    } else {
        NAPI_CALL_RETURN_VOID(env,
            napi_get_null(env, &args[0]));

        NAPI_CALL_RETURN_VOID(env,
            napi_create_object(env, &attr));
        args[1] = attr;

        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(env, "name", NAPI_AUTO_LENGTH, &k));
        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(env, data->attr.zone_attr_name,
                NAPI_AUTO_LENGTH, &v));
        NAPI_CALL_RETURN_VOID(env,
            napi_set_property(env, attr, k, v));

        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(env, "type", NAPI_AUTO_LENGTH, &k));
        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(env, data->attr.zone_attr_type,
                NAPI_AUTO_LENGTH, &v));
        NAPI_CALL_RETURN_VOID(env,
            napi_set_property(env, attr, k, v));

        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(env, "value", NAPI_AUTO_LENGTH, &k));
        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(env, data->attr.zone_attr_value,
                NAPI_AUTO_LENGTH, &v));
        NAPI_CALL_RETURN_VOID(env,
            napi_set_property(env, attr, k, v));
    }

    // Call the caller's given callback.
    NAPI_CALL_RETURN_VOID(env,
        napi_get_reference_value(env, data->callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_get_global(env, &global));
    NAPI_CALL_RETURN_VOID(env,
        napi_call_function(env, global, callback, 2, args, &result));

    // Clean up.
    NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, data->callback));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, data->work));
    free(data);
}

napi_value zu_getzoneattr(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value argv[3];
    size_t num_bytes;
    napi_value resource_name;
    _zu_getzoneattr_data_t *data = calloc(1, sizeof(_zu_getzoneattr_data_t));

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != 3) {
        napi_throw_type_error(env, NULL, "incorrect number of arguments");
        return NULL;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], data->zone_name,
        ZONENAME_MAX+1, &num_bytes));
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[1], data->attr.zone_attr_name,
        sizeof(data->attr.zone_attr_name), &num_bytes));
    NAPI_CALL(env, napi_create_reference(env, argv[2], 1, &(data->callback)));

    NAPI_CALL(env, napi_create_string_utf8(
                env, "NullResource", NAPI_AUTO_LENGTH, &resource_name));
    NAPI_CALL(env, napi_create_async_work(env, NULL, resource_name,
        _zu_getzoneattr_execute, _zu_getzoneattr_complete, data, &(data->work)));
    NAPI_CALL(env, napi_queue_async_work(env, data->work));

    return NULL;
}


static napi_value Init(napi_env env, napi_value exports) {
    // - https://nodejs.org/api/n-api.html#n_api_napi_property_descriptor
    //   for fields descriptions.
    // - `0b010` is `napi_property_attributes` for enumerable, read-only.
    //   https://nodejs.org/api/n-api.html#n_api_napi_property_attributes
    napi_property_descriptor properties[] = {
        // zone.h
        { "getzoneid", NULL, zu_getzoneid, NULL, NULL, NULL, 0b010, NULL },
        { "getzoneidbyname", NULL, zu_getzoneidbyname, NULL, NULL, NULL, 0b010, NULL },
        { "getzonenamebyid", NULL, zu_getzonenamebyid, NULL, NULL, NULL, 0b010, NULL },

        { "getzonename", NULL, zu_getzonename, NULL, NULL, NULL, 0b010, NULL },

        // libzonecfg.h
        { "getzonestate", NULL, zu_getzonestate, NULL, NULL, NULL, 0b010, NULL },
        { "getzoneattr", NULL, zu_getzoneattr, NULL, NULL, NULL, 0b010, NULL },
    };

    NAPI_CALL(env, napi_define_properties(
          env, exports, sizeof(properties) / sizeof(*properties), properties));

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)

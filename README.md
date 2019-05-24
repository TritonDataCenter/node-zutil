node-zutil is a small library specific to SunOS/Solaris/Illumos/SmartOS
that provides a wrapper over all three APIs in
[zone.h](https://github.com/joyent/illumos-joyent/blob/master/usr/src/head/zone.h)
and *one* from
[libzonecfg.h](https://github.com/joyent/illumos-joyent/blob/master/usr/src/head/libzonecfg.h).

"libzonecfg" is not documented in illumos and hence (IIUC) considered "unstable".
That's a significant reason why bindings for more of its API is not provided
here.

Version 2 of this lib is a significant departure from earlier versions. See
[the changelog entry](./CHANGES.md#200).


# Usage

Assuming "5d4f7599-a991-6b35-dd44-d91936957a6b" is your current zone.

    > var zutil = require('zutil')

From zone.h:

    > id =  zutil.getzoneid()               // get your current zone numeric ID
    403

    > zonename = zutil.getzonenamebyid(id)  // get the zonename from a zone ID
    '5d4f7599-a991-6b35-dd44-d91936957a6b'

    > zutil.getzoneidbyname(zonename)
    403

As a convenience, because working zone *name* is more common, this API
is added by this module:

    > zutil.getzonename()
    '5d4f7599-a991-6b35-dd44-d91936957a6b'

From libzonecfg.h:

    > zutil.getzonestate('5d4f7599-a991-6b35-dd44-d91936957a6b')
    'running'



# Install

    npm install zutil


# Reference

## `getzoneid()`

Gets the integer zone ID for the current process. In the global zone the id
is zero (0).


## `getzonenamebyid(<id>)`

Gets the string zonename for the given zone id.

#### Parameters

- `id` - Integer zone id.

#### Examples

```
> zutil.getzonenamebyid(403)
'5d4f7599-a991-6b35-dd44-d91936957a6b'
```


## `getzoneidbyname(<zonename>)`

Gets the integer zone id from the zonename.

#### Parameters

- `zonename` - String zone name. Typically on SmartOS this is 'global' or a
  UUID.

#### Examples

```
> zutil.getzoneidbyname('5d4f7599-a991-6b35-dd44-d91936957a6b')
403
```


## `getzonename()`

A convenience function that gets the current zonename.

#### Examples

```
> zutil.getzonename()
'5d4f7599-a991-6b35-dd44-d91936957a6b'
```


## `getzonestate(<zonename>)`

Gets the zone state string (e.g. "running", "configured") for the given
`zonename` string. Throws an `Error` on any failure. If you are in a non-global
zone, you may only get a successful result for the current zone. Other zones
will appear to not exist.

At the time of writing the possible zone state strings in illumos are the
[`ZONE_STATE_STR_...` vars defined
here](https://github.com/joyent/illumos-joyent/blob/ab6a47af7ee9daefa937f7e8ca0531e68d003686/usr/src/lib/libzonecfg/common/zonecfg_impl.h#L48-L55)
plus `"unknown"`.

As stated above, "libzonecfg" is undocumented. This function name is made up.
It combines the C `zone_get_state` and `zone_state_str` functions.


#### Parameters

- `zonename` - String zone name. Typically on SmartOS this is 'global' or a
  UUID.


#### Examples

```
> zutil.getzonestate('5d4f7599-a991-6b35-dd44-d91936957a6b')
'running'

> zutil.getzonestate('global')      # errors if no in the global zone
Error: could not get zone "global" state: No such zone configured
    at Error (native)
    ...

> zutil.getzonestate('nosuchzone')
Error: could not get zone "nosuchzone" state: No such zone configured
    at Error (native)
    ...
```

# Development

    npm install

Note: If you are running as root you will see this warning
(see [npm/npm#3497](https://github.com/npm/npm/issues/3497)):

    npm WARN lifecycle zonecfg@1.0.0~install: cannot run in wd zonecfg@1.0.0 node-gyp rebuild (wd=...)

You then need to use:

    npm install --unsafe-perm



# License

MIT.

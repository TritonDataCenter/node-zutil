node-zutil is a small library specific to SunOS/Solaris/Illumos/SmartOS
that provides a wrapper over all three APIs in
[zone.h](https://github.com/joyent/illumos-joyent/blob/master/usr/src/head/zone.h)
and *one* from
[libzonecfg.h](https://github.com/joyent/illumos-joyent/blob/master/usr/src/head/libzonecfg.h).

"libzonecfg" is not documented in illumos and hence (IIUC) considered "unstable".
That's a significant reason why bindings for more of its API is not provided
here. See API-specific notes in the reference section below.

Version 2 of this lib is a significant departure from earlier versions. See
[the changelog entry](./CHANGES.md#v200).


# Usage

    var zutil = require('zutil');

    // zone.h
    // XXX
    var myZone = zutil.getZone();
    var someOtherZone = zutil.getZoneByName('foo');
    var yetAnotherZone = zutil.getZoneById(20);

    // libzonecfg.h
    var state = zutil.getzonestate('foo'); // => "running"

    > var zutil = require('zutil')

    > zutil.getzonestate('5d4f7599-a991-6b35-dd44-d91936957a6b')
    'running'


Assuming "5d4f7599-a991-6b35-dd44-d91936957a6b" is your current zone. If you are 

# Install

    npm install zutil


# Reference

## `getzonestate(zonename)`

Gets the zone state string (e.g. "running", "configured") for the given
`zonename` string.  Throws an `Error` on any failure.

Parameters:

- `zonename` - String zone name. Typically on SmartOS this is 'global' or a
  UUID.

At the time of writing the possible zone state strings in illumos are the
[`ZONE_STATE_STR_...` vars defined
here](https://github.com/joyent/illumos-joyent/blob/ab6a47af7ee9daefa937f7e8ca0531e68d003686/usr/src/lib/libzonecfg/common/zonecfg_impl.h#L48-L55)
plus `"unknown"`.

If you are in a non-global zone, you may only get a successful result for the
current zone. Other zones will appear to not exist.

Examples:

```
> zutil.getzonestate('5d4f7599-a991-6b35-dd44-d91936957a6b')
'running'

> zutil.getzonestate('global')
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

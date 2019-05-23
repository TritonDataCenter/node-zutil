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


# Install

    npm install zutil


# Reference

## `getzonestate(zonename)`

XXX


# Development

    npm install

Note: If you are running as root you will see this warning
(see [npm/npm#3497](https://github.com/npm/npm/issues/3497)):

    npm WARN lifecycle zonecfg@1.0.0~install: cannot run in wd zonecfg@1.0.0 node-gyp rebuild (wd=...)

You then need to use:

    npm install --unsafe-perm



# License

MIT.

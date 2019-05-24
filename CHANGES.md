# node-zutil changelog

## not yet released

(nothing yet)

## 2.0.0

- A re-write a this module to work with node v6 and later. This module is almost
  completely different. Changes:

    - Works with node v6 and drops support for earlier versions. Earlier
      versions only worked with node versions up to v0.10.
    - Drops some APIs:
        - `listZones`: Not used in Triton/Manta stack. Not a documented part
          of zone.h.
        - `getZoneAttribute`, `getZoneAttributes`: Not documented stable API
          (none of libzonecfg.h is). I ran into crashes implementing this
          with N-API and node v6. I suspect libzonecfg and its usage of
          libxml2 is just not threadsafe. Also this isn't heavily used by
          Triton/Manta. See [the
          PR](https://github.com/joyent/node-zutil/pull/8) and [the
          branch](https://github.com/joyent/node-zutil/tree/getzoneattr) for
          the crashing attempt.  See workaround in the migration table below.
    - Renames all the other APIs. The new names match the name of the C
      functions, where those exist.

  Migrating from zutil 0.x and 1.x to 2.x:

    | old API                                                      | new API |
    | -------------------------------------------------------------| ------- |
    | `getZone() -> {id: <id>, name: <zonename>}`                  | `getzoneid() -> <id>`, `getzonename() -> <zonename>` |
    | `getZoneByName(zonename) -> {id: <id>, name: <zonename>}`    | `getzoneidbyname(zonename) -> <id>` |
    | `getZoneById() -> {id: <id>, name: <zonename>}`              | `getzonenamebyid(id) -> <zonename>` |
    | `getZoneState(zonename) -> '<state>'`                        | `getzonestate(<zonename>) -> '<state>'` |
    | `listZones()`                                                | dropped, exec `zoneadm list -p` and parse |
    | `getZoneAttribute(zonename, attrname, function (err, attr))` | dropped, exec `zonecfg -z <zonename> attr name=<attrname>` and parse |
    | `getZoneAttributes(zonename, function (err, attrs))`         | dropped, exec `zonecfg -z <zonename> attr` and parse |

## 1.0.0

- joyent/node-zutil#4 Added support for building with node 0.10.

## 0.2.1

- Exclude the "build/" dir from the published npm package.

## 0.2.0

(Use the commit history.)

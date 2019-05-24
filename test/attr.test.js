//
// Copyright 2019 Joyent, Inc.
//

//
// Test the methods related to getting zone attributes.
// These can only run in the "global" zone.
//
// Assumptions:
// - There is at least a single non-global zone with which we can test.
// - The first non-global zone (from `zoneadm list -p`) has a 
//   'create-timestamp' attr -- which is typically true for SmartOS-y zones.

var child_process = require('child_process');
var exec = child_process.exec;
var execSync = child_process.execSync;
var test = require('tap').test;

var zutil = require('../');


var zonename = execSync('zonename', {encoding: 'utf-8'}).trim();
var OPTS = {
    skip: (zonename !== 'global' 
        && 'must run in global zone to test getting attrs')
};

test('attr-related functions', OPTS, function (suite) {
    var testZoneName;

    suite.test('setup', function (t) {
        // Select the first non-global zone for testing.
        exec('zoneadm list -p | head -2 | tail -1 | cut -d: -f2',
            function (err, stdout, stderr) {
                t.error(err);
                t.notOk(stderr);
                testZoneName = stdout.trim();
                if (!testZoneName) {
                    OPTS.skip = 'cannot find non-global zone with which to test';
                } else {
                    t.comment('testing with zone ' + testZoneName);
                }
                t.end();
            });
    });

    suite.test('error cases', OPTS, function (t) {
        t.throws(function () { zutil.getzoneattr(); },
            /incorrect number of arguments/);
        t.end();
    });

    suite.test('no such zone', function (t) {
        zutil.getzoneattr('nosuchzone', 'create-timestamp', function (err, _) {
            t.ok(err);
            t.equal(err.message, 'nosuchzone: No such zone configured');
            t.end();
        }); 
    });

    suite.test('no such attr', function (t) {
        // TODO: This second call to getzoneattr crashes.
        zutil.getzoneattr(testZoneName, 'nosuchattr', function (err, attr) {
            t.ok(err);
            t.equal(err.message, 'nosuchzone: No such zone configured');
            t.end();
        }); 
    });

    suite.test('zone_get_attr', function (t) {
        zutil.getzoneattr(testZoneName, 'create-timestamp', function (err, attr) {
            t.error(err);
            t.deepEqual(attr, {name: 'create-timestamp', type: 'string'});
            t.ok(attr.value);
            t.end();
        }); 
    });

    suite.end();
});

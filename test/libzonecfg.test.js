//
// Copyright 2019 Joyent, Inc.
//

//
// Test the libzonecfg.h bindings
//

var exec = require('child_process').exec;
var test = require('tap').test;

var zutil = require('../');


test('getzonestate', function (suite) {
    var currZoneName;

    suite.test('setup: get current zonename', function (t) {
        exec('zonename', function (err, stdout, stderr) {
            t.error(err);
            t.notOk(stderr);
            currZoneName = stdout.trim();
            t.end();
        });
    });

    suite.test('error cases', function (t) {
        t.throws(function () { zutil.getzonestate('one', 'two'); },
            /incorrect number of arguments/);
        t.throws(function () { zutil.getzonestate('nosuchzone'); },
            /could not get zone "nosuchzone" state: No such zone configured/);
        t.end();
    });

    suite.test('getzonestate', function (t) {
        var stateStr = zutil.getzonestate(currZoneName);
        t.equal(stateStr, 'running');
        t.end();
    });

    suite.end();
});

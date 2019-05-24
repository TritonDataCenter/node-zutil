//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2019 Joyent, Inc.
//

//
// Test the zone.h and related bindings
//

var exec = require('child_process').exec;
var test = require('tap').test;

var zutil = require('../');


test('zone.h bindings', function (suite) {
    var id;
    var zonename;

    suite.test('error cases', function (t) {
        t.throws(function () { zutil.getzoneid('one'); },
            /incorrect number of arguments/);
        t.throws(function () { zutil.getzoneidbyname(); },
            /incorrect number of arguments/);
        t.throws(function () { zutil.getzonenamebyid(); },
            /incorrect number of arguments/);
        t.throws(function () { zutil.getzonename('one'); },
            /incorrect number of arguments/);

        t.throws(function () { zutil.getzoneidbyname('nosuchzone'); },
            /could not id for zonename "nosuchzone": Invalid argument/);
        t.throws(function () { zutil.getzonenamebyid(-2); },
            /could not get zonename for id -2: Invalid argument/);
        t.end();
    });

    suite.test('getzoneid', function (t) {
        id = zutil.getzoneid();
        t.equal(typeof(id), 'number');
        t.ok(id >= 0, 'id is greater than zero');
        t.end();
    });

    suite.test('getzonename', function (t) {
        zonename = zutil.getzonename();
        t.ok(zonename);
        t.equal(typeof(zonename), 'string');
        t.end();
    });

    suite.test('getzonenamebyid', function (t) {
        var rv = zutil.getzonenamebyid(id);
        t.equal(rv, zonename);
        t.end();
    });

    suite.test('getzoneidbyname', function (t) {
        var rv = zutil.getzoneidbyname(zonename);
        t.equal(rv, id);
        t.end();
    });

    suite.end();
});

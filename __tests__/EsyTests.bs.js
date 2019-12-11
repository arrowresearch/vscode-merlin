// Generated by BUCKLESCRIPT, PLEASE EDIT WITH CARE
'use strict';

var Esy = require("../src/Esy.bs.js");
var Jest = require("@glennsl/bs-jest/src/jest.js");
var Curry = require("bs-platform/lib/js/curry.js");
var Filename = require("bs-platform/lib/js/filename.js");

var projPath = Curry._1(Filename.dirname, __dirname);

Jest.describe("Esy.getStatus", (function (param) {
        return Jest.testPromise("Checking if running esy status in __dirname works: " + projPath, undefined, (function (param) {
                      return Esy.getStatus(projPath).then((function (status) {
                                    return Promise.resolve(Jest.Expect.toBe(false, Jest.Expect.expect(status.isProject)));
                                  }));
                    }));
      }));

exports.projPath = projPath;
/* projPath Not a pure module */

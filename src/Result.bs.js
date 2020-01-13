// Generated by BUCKLESCRIPT, PLEASE EDIT WITH CARE
'use strict';

var Block = require("bs-platform/lib/js/block.js");
var Curry = require("bs-platform/lib/js/curry.js");

function map(e, f) {
  if (e.tag) {
    return /* Error */Block.__(1, [e[0]]);
  } else {
    return /* Ok */Block.__(0, [Curry._1(f, e[0])]);
  }
}

function bind(e, f) {
  if (e.tag) {
    return /* Error */Block.__(1, [e[0]]);
  } else {
    return Curry._1(f, e[0]);
  }
}

var $great$pipe = map;

var $great$great$eq = bind;

exports.map = map;
exports.$great$pipe = $great$pipe;
exports.bind = bind;
exports.$great$great$eq = $great$great$eq;
/* No side effect */

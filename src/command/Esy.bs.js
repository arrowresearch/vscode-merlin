// Generated by BUCKLESCRIPT, PLEASE EDIT WITH CARE
'use strict';

var $$Node = require("../bindings/Node.bs.js");
var Block = require("bs-platform/lib/js/block.js");
var Utils = require("../Utils.bs.js");
var Js_exn = require("bs-platform/lib/js/js_exn.js");
var Js_dict = require("bs-platform/lib/js/js_dict.js");
var Js_json = require("bs-platform/lib/js/js_json.js");
var Pervasives = require("bs-platform/lib/js/pervasives.js");
var Caml_option = require("bs-platform/lib/js/caml_option.js");
var Caml_exceptions = require("bs-platform/lib/js/caml_exceptions.js");
var Caml_js_exceptions = require("bs-platform/lib/js/caml_js_exceptions.js");

var UnexpectedJSONValue = Caml_exceptions.create("Esy.UnexpectedJSONValue");

var Stderr = Caml_exceptions.create("Esy.Stderr");

var UnknownError = Caml_exceptions.create("Esy.UnknownError");

var JSError = Caml_exceptions.create("Esy.JSError");

function toString(param) {
  if (typeof param === "number") {
    return "An unknown error occurred";
  } else {
    switch (param.tag | 0) {
      case /* CmdFailed */0 :
          return " `" + (String(param[0]) + "` command failed ");
      case /* NonZeroExit */1 :
          return " " + (String(param[0]) + (" failed:\nexitCode: " + (String(param[1]) + ("\nstdout: " + (String(param[2]) + ("\nstderr: " + (String(param[3]) + " ")))))));
      case /* UnexpectedJSONValue */2 :
          return " Unexpected json value ";
      case /* JsonParseExn */3 :
          return "Error: " + (String(param[0]) + "");
      
    }
  }
}

var E = {
  toString: toString
};

function bool$prime(x) {
  if (typeof x === "number") {
    switch (x) {
      case /* JSONFalse */0 :
          return false;
      case /* JSONTrue */1 :
          return true;
      case /* JSONNull */2 :
          throw [
                UnexpectedJSONValue,
                x
              ];
      
    }
  } else {
    throw [
          UnexpectedJSONValue,
          x
        ];
  }
}

function raiseIfNone(param) {
  if (param !== undefined) {
    return Caml_option.valFromOption(param);
  } else {
    return Pervasives.failwith("Found None where it was not expected");
  }
}

function bool_(x) {
  return bool$prime(Js_json.classify(raiseIfNone(x)));
}

function nullableString$prime(x) {
  if (typeof x === "number") {
    if (x === /* JSONNull */2) {
      return null;
    } else {
      throw [
            UnexpectedJSONValue,
            x
          ];
    }
  } else if (x.tag) {
    throw [
          UnexpectedJSONValue,
          x
        ];
  } else {
    return x[0];
  }
}

function nullableString(param) {
  if (param !== undefined) {
    return nullableString$prime(Js_json.classify(Caml_option.valFromOption(param)));
  } else {
    return null;
  }
}

function status(path) {
  return $$Node.ChildProcess.exec("esy status", {
                cwd: path
              }).then((function (param) {
                return Utils.$less$less((function (prim) {
                              return Promise.resolve(prim);
                            }), (function (param) {
                              if (param.tag) {
                                return /* Error */Block.__(1, [/* CmdFailed */Block.__(0, ["esy status"])]);
                              } else {
                                var match = param[0];
                                if (match[2] === "") {
                                  try {
                                    var json = JSON.parse(match[1]);
                                    var x = Js_json.classify(json);
                                    var dict;
                                    if (typeof x === "number") {
                                      throw [
                                            UnexpectedJSONValue,
                                            x
                                          ];
                                    } else if (x.tag === /* JSONObject */2) {
                                      dict = x[0];
                                    } else {
                                      throw [
                                            UnexpectedJSONValue,
                                            x
                                          ];
                                    }
                                    var x$1 = Js_dict.get(dict, "isProject");
                                    var isProject = bool$prime(Js_json.classify(raiseIfNone(x$1)));
                                    var x$2 = Js_dict.get(dict, "isProjectSolved");
                                    var isProjectSolved = bool$prime(Js_json.classify(raiseIfNone(x$2)));
                                    var x$3 = Js_dict.get(dict, "isProjectFetched");
                                    var isProjectFetched = bool$prime(Js_json.classify(raiseIfNone(x$3)));
                                    var x$4 = Js_dict.get(dict, "isProjectReadyForDev");
                                    var isProjectReadyForDev = bool$prime(Js_json.classify(raiseIfNone(x$4)));
                                    var rootBuildPath = nullableString(Js_dict.get(dict, "rootBuildPath"));
                                    var rootInstallPath = nullableString(Js_dict.get(dict, "rootInstallPath"));
                                    var rootPackageConfigPath = nullableString(Js_dict.get(dict, "rootPackageConfigPath"));
                                    return /* Ok */Block.__(0, [{
                                                isProject: isProject,
                                                isProjectSolved: isProjectSolved,
                                                isProjectFetched: isProjectFetched,
                                                isProjectReadyForDev: isProjectReadyForDev,
                                                rootBuildPath: rootBuildPath,
                                                rootInstallPath: rootInstallPath,
                                                rootPackageConfigPath: rootPackageConfigPath
                                              }]);
                                  }
                                  catch (raw_exn){
                                    var exn = Caml_js_exceptions.internalToOCamlException(raw_exn);
                                    if (exn[0] === UnexpectedJSONValue) {
                                      return /* Error */Block.__(1, [/* UnexpectedJSONValue */Block.__(2, [exn[1]])]);
                                    } else if (exn[0] === Js_exn.$$Error) {
                                      var match$1 = exn[1].message;
                                      if (match$1 !== undefined) {
                                        return /* Error */Block.__(1, [/* JsonParseExn */Block.__(3, [match$1])]);
                                      } else {
                                        return /* Error */Block.__(1, [/* UnknownError */0]);
                                      }
                                    } else {
                                      throw exn;
                                    }
                                  }
                                } else {
                                  return /* Error */Block.__(1, [/* CmdFailed */Block.__(0, ["esy status"])]);
                                }
                              }
                            }), param);
              }));
}

function prepareProjectPathArgs(param) {
  if (param !== undefined) {
    return "-P " + (String(Caml_option.valFromOption(param)) + " ");
  } else {
    return "";
  }
}

function prepareCommand(a) {
  return a.join(" ");
}

function subcommand(c, p) {
  var cmd = [
      "esy install",
      prepareProjectPathArgs(p)
    ].join(" ");
  return $$Node.ChildProcess.exec(cmd, { }).then((function (param) {
                if (param.tag) {
                  return Promise.resolve(/* Error */Block.__(1, [/* CmdFailed */Block.__(0, [cmd])]));
                } else {
                  var match = param[0];
                  var stdout = match[1];
                  var exitCode = match[0];
                  if (exitCode === 0) {
                    return Promise.resolve(/* Ok */Block.__(0, [stdout]));
                  } else {
                    return Promise.resolve(/* Error */Block.__(1, [/* NonZeroExit */Block.__(1, [
                                      cmd,
                                      exitCode,
                                      stdout,
                                      match[2]
                                    ])]));
                  }
                }
              }));
}

function install(param) {
  return subcommand("install", param);
}

function i(param) {
  return subcommand("i", param);
}

function importDependencies(param) {
  return subcommand("import-dependencies", param);
}

function build(param) {
  return subcommand("build", param);
}

function b(param) {
  return subcommand("b", param);
}

exports.UnexpectedJSONValue = UnexpectedJSONValue;
exports.Stderr = Stderr;
exports.UnknownError = UnknownError;
exports.JSError = JSError;
exports.E = E;
exports.bool$prime = bool$prime;
exports.raiseIfNone = raiseIfNone;
exports.bool_ = bool_;
exports.nullableString$prime = nullableString$prime;
exports.nullableString = nullableString;
exports.status = status;
exports.prepareProjectPathArgs = prepareProjectPathArgs;
exports.prepareCommand = prepareCommand;
exports.subcommand = subcommand;
exports.install = install;
exports.i = i;
exports.importDependencies = importDependencies;
exports.build = build;
exports.b = b;
/* Node Not a pure module */

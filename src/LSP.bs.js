// Generated by BUCKLESCRIPT, PLEASE EDIT WITH CARE
'use strict';

var Fs = require("fs");
var Esy = require("./Esy.bs.js");
var $$Node = require("./bindings/Node.bs.js");
var Path = require("path");
var Block = require("bs-platform/lib/js/block.js");
var Curry = require("bs-platform/lib/js/curry.js");
var Utils = require("./Utils.bs.js");
var Result = require("./Result.bs.js");
var Vscode = require("vscode");
var Js_dict = require("bs-platform/lib/js/js_dict.js");
var Js_json = require("bs-platform/lib/js/js_json.js");
var $$Request = require("request");
var Caml_array = require("bs-platform/lib/js/caml_array.js");
var Caml_option = require("bs-platform/lib/js/caml_option.js");
var RequestProgress = require("request-progress");
var Caml_builtin_exceptions = require("bs-platform/lib/js/caml_builtin_exceptions.js");

function download(url, file, progress, end_, error, data) {
  var stream = RequestProgress($$Request(url));
  $$Node.RequestProgress.onProgress(stream, (function (state) {
          return Curry._1(progress, state.size.transferred / (134 * 1024 * 1024));
        }));
  $$Node.RequestProgress.onData(stream, data);
  $$Node.RequestProgress.onEnd(stream, end_);
  $$Node.RequestProgress.onError(stream, error);
  stream.pipe(Fs.createWriteStream(file));
  return /* () */0;
}

function detect(folder) {
  return Esy.getStatus(folder).then((function (status) {
                if (status.isProject) {
                  var match = status.rootPackageConfigPath;
                  var manifestFile = (match == null) ? "" : match;
                  if (new RegExp(".json$").test(manifestFile)) {
                    if (status.isProjectReadyForDev) {
                      return Promise.resolve(/* Esy */Block.__(0, [/* readyForDev */true]));
                    } else {
                      return $$Node.Fs.readFile(manifestFile).then((function (manifest) {
                                    var manifestJson = JSON.parse(manifest);
                                    var manifestHasEsyConfig = Utils.propertyExists(manifestJson, "esy");
                                    var manifestIsEsyJSON = new RegExp("esy.json$").test(manifestFile);
                                    if (manifestIsEsyJSON || manifestHasEsyConfig) {
                                      return Promise.resolve(/* Esy */Block.__(0, [/* readyForDev */status.isProjectReadyForDev]));
                                    } else {
                                      var esyToolChainFolder = Path.join(folder, ".vscode", "esy");
                                      return $$Node.Fs.exists(esyToolChainFolder).then((function (doesToolChainEsyManifestExist) {
                                                    if (doesToolChainEsyManifestExist) {
                                                      return Esy.getStatus(esyToolChainFolder).then((function (toolChainStatus) {
                                                                    if (toolChainStatus.isProject) {
                                                                      return Promise.resolve(/* Bsb */Block.__(1, [/* readyForDev */toolChainStatus.isProjectSolved]));
                                                                    } else {
                                                                      return Promise.reject([
                                                                                  Caml_builtin_exceptions.failure,
                                                                                  "Weird invariant violation. Why would .vscode/esy exist but not be a valid esy project. TODO"
                                                                                ]);
                                                                    }
                                                                  }));
                                                    } else {
                                                      return Promise.resolve(/* Bsb */Block.__(1, [/* readyForDev */false]));
                                                    }
                                                  }));
                                    }
                                  }));
                    }
                  } else {
                    return Promise.resolve(/* Opam */0);
                  }
                } else {
                  return Promise.reject([
                              Caml_builtin_exceptions.failure,
                              "Not a valid esy/opam/bsb project"
                            ]);
                }
              }));
}

var ProjectType = {
  detect: detect
};

function make(folder) {
  process.env["OCAMLRUNPARAM"] = "b";
  process.env["MERLIN_LOG"] = "-";
  return detect(folder).then((function (projectType) {
                var setupPromise;
                setupPromise = typeof projectType === "number" ? Promise.resolve(/* () */0) : (
                    projectType.tag ? (
                        projectType[/* readyForDev */0] ? Promise.resolve(/* () */0) : Esy.setup(Path.join(folder, "package.json")).then((function (param) {
                                  return Vscode.window.withProgress({
                                              location: 15,
                                              title: "Setting up toolchain..."
                                            }, (function (progress) {
                                                progress.report({
                                                      increment: 10
                                                    });
                                                var hiddenEsyRoot = Path.join(folder, ".vscode", "esy");
                                                return $$Node.ChildProcess.exec("esy i -P " + hiddenEsyRoot, {
                                                                      cwd: folder
                                                                    }).then((function (param) {
                                                                      progress.report({
                                                                            increment: 10
                                                                          });
                                                                      var restBase = "https://dev.azure.com/arrowresearch/";
                                                                      var proj = "vscode-merlin";
                                                                      var match = process.platform;
                                                                      var os;
                                                                      switch (match) {
                                                                        case "darwin" :
                                                                            os = "Darwin";
                                                                            break;
                                                                        case "linux" :
                                                                            os = "Linux";
                                                                            break;
                                                                        case "win32" :
                                                                            os = "Windows_NT";
                                                                            break;
                                                                        default:
                                                                          throw [
                                                                                Caml_builtin_exceptions.match_failure,
                                                                                /* tuple */[
                                                                                  "LSP.re",
                                                                                  186,
                                                                                  39
                                                                                ]
                                                                              ];
                                                                      }
                                                                      var artifactName = "cache-" + (String(os) + "-install");
                                                                      return $$Node.Https.getCompleteResponse("" + (String(restBase) + ("/" + (String(proj) + ("/_apis/build/builds?" + (String("deletedFilter=excludeDeleted&statusFilter=completed&resultFilter=succeeded") + ("&" + (String("branchName=refs%2Fheads%2Fmaster") + ("&" + (String("queryOrder=finishTimeDescending&$top=1") + "&api-version=4.1")))))))))).then((function (param) {
                                                                                          return Promise.resolve(Result.$great$great$eq(param, (function (responseText) {
                                                                                                            try {
                                                                                                              var json = JSON.parse(responseText);
                                                                                                              var match = Js_json.classify(json);
                                                                                                              if (typeof match === "number") {
                                                                                                                return /* Error */Block.__(1, [" Response from Azure wasn\'t an object "]);
                                                                                                              } else if (match.tag === /* JSONObject */2) {
                                                                                                                var match$1 = Js_dict.get(match[0], "value");
                                                                                                                if (match$1 !== undefined) {
                                                                                                                  var match$2 = Js_json.classify(Caml_option.valFromOption(match$1));
                                                                                                                  if (typeof match$2 === "number") {
                                                                                                                    return /* Error */Block.__(1, [" Response from Azure did not contain build \'value\' "]);
                                                                                                                  } else if (match$2.tag === /* JSONArray */3) {
                                                                                                                    var o = Caml_array.caml_array_get(match$2[0], 0);
                                                                                                                    var match$3 = Js_json.classify(o);
                                                                                                                    if (typeof match$3 === "number") {
                                                                                                                      return /* Error */Block.__(1, [" First item in the \'value\' field array isn\'t an object as expected "]);
                                                                                                                    } else if (match$3.tag === /* JSONObject */2) {
                                                                                                                      var match$4 = Js_dict.get(match$3[0], "id");
                                                                                                                      if (match$4 !== undefined) {
                                                                                                                        var match$5 = Js_json.classify(Caml_option.valFromOption(match$4));
                                                                                                                        if (typeof match$5 === "number") {
                                                                                                                          return /* Error */Block.__(1, [" Field id was expected to be a number "]);
                                                                                                                        } else if (match$5.tag === /* JSONNumber */1) {
                                                                                                                          return /* Ok */Block.__(0, [match$5[0]]);
                                                                                                                        } else {
                                                                                                                          return /* Error */Block.__(1, [" Field id was expected to be a number "]);
                                                                                                                        }
                                                                                                                      } else {
                                                                                                                        return /* Error */Block.__(1, [" Field id was missing "]);
                                                                                                                      }
                                                                                                                    } else {
                                                                                                                      return /* Error */Block.__(1, [" First item in the \'value\' field array isn\'t an object as expected "]);
                                                                                                                    }
                                                                                                                  } else {
                                                                                                                    return /* Error */Block.__(1, [" Response from Azure did not contain build \'value\' "]);
                                                                                                                  }
                                                                                                                } else {
                                                                                                                  return /* Error */Block.__(1, ["Field 'value' in Azure's response was undefined"]);
                                                                                                                }
                                                                                                              } else {
                                                                                                                return /* Error */Block.__(1, [" Response from Azure wasn\'t an object "]);
                                                                                                              }
                                                                                                            }
                                                                                                            catch (exn){
                                                                                                              return /* Error */Block.__(1, [" Failed to parse response from Azure "]);
                                                                                                            }
                                                                                                          })));
                                                                                        })).then((function (r) {
                                                                                        if (r.tag) {
                                                                                          return Promise.resolve(/* Error */Block.__(1, [r[0]]));
                                                                                        } else {
                                                                                          return $$Node.Https.getCompleteResponse("" + (String(restBase) + ("/" + (String(proj) + ("/_apis/build/builds/" + (String(r[0]) + ("/artifacts?artifactname=" + (String(artifactName) + "&api-version=4.1"))))))));
                                                                                        }
                                                                                      })).then((function (param) {
                                                                                      return Promise.resolve(Result.$great$great$eq(param, (function (responseText) {
                                                                                                        try {
                                                                                                          var json = JSON.parse(responseText);
                                                                                                          var match = Js_json.classify(json);
                                                                                                          if (typeof match === "number") {
                                                                                                            return /* Error */Block.__(1, [" Response from Azure wasn\'t an object "]);
                                                                                                          } else if (match.tag === /* JSONObject */2) {
                                                                                                            var match$1 = Js_dict.get(match[0], "resource");
                                                                                                            if (match$1 !== undefined) {
                                                                                                              var match$2 = Js_json.classify(Caml_option.valFromOption(match$1));
                                                                                                              if (typeof match$2 === "number") {
                                                                                                                return /* Error */Block.__(1, [" First item in the \'resource\' field array isn\'t an object as expected "]);
                                                                                                              } else if (match$2.tag === /* JSONObject */2) {
                                                                                                                var match$3 = Js_dict.get(match$2[0], "downloadUrl");
                                                                                                                if (match$3 !== undefined) {
                                                                                                                  var match$4 = Js_json.classify(Caml_option.valFromOption(match$3));
                                                                                                                  if (typeof match$4 === "number") {
                                                                                                                    return /* Error */Block.__(1, [" Field downloadUrl was expected to be a string "]);
                                                                                                                  } else if (match$4.tag) {
                                                                                                                    return /* Error */Block.__(1, [" Field downloadUrl was expected to be a string "]);
                                                                                                                  } else {
                                                                                                                    return /* Ok */Block.__(0, [match$4[0]]);
                                                                                                                  }
                                                                                                                } else {
                                                                                                                  return /* Error */Block.__(1, [" Field downloadUrl was missing "]);
                                                                                                                }
                                                                                                              } else {
                                                                                                                return /* Error */Block.__(1, [" First item in the \'resource\' field array isn\'t an object as expected "]);
                                                                                                              }
                                                                                                            } else {
                                                                                                              return /* Error */Block.__(1, ["Field 'value' in Azure's response was undefined"]);
                                                                                                            }
                                                                                                          } else {
                                                                                                            return /* Error */Block.__(1, [" Response from Azure wasn\'t an object "]);
                                                                                                          }
                                                                                                        }
                                                                                                        catch (exn){
                                                                                                          return /* Error */Block.__(1, [" Failed to parse response from Azure "]);
                                                                                                        }
                                                                                                      })));
                                                                                    })).then((function (r) {
                                                                                    if (r.tag) {
                                                                                      var x = r[0];
                                                                                      console.log("Error>>>>>>", x);
                                                                                      return Promise.resolve(/* Error */Block.__(1, [x]));
                                                                                    } else {
                                                                                      var downloadUrl = r[0];
                                                                                      console.log("download", downloadUrl);
                                                                                      var lastProgress = {
                                                                                        contents: 0
                                                                                      };
                                                                                      return new Promise((function (resolve, param) {
                                                                                                    return download(downloadUrl, Path.join(hiddenEsyRoot, "cache.zip"), (function (progressFraction) {
                                                                                                                  var percent = progressFraction * 80.0 | 0;
                                                                                                                  progress.report({
                                                                                                                        increment: percent - lastProgress.contents | 0
                                                                                                                      });
                                                                                                                  console.log("incremented by", percent, lastProgress.contents);
                                                                                                                  lastProgress.contents = percent;
                                                                                                                  return /* () */0;
                                                                                                                }), (function (param) {
                                                                                                                  return resolve(/* Ok */Block.__(0, [/* () */0]));
                                                                                                                }), (function (e) {
                                                                                                                  return resolve(/* Error */Block.__(1, ["Failed to download " + (String(downloadUrl) + " ")]));
                                                                                                                }), (function (param) {
                                                                                                                  return /* () */0;
                                                                                                                }));
                                                                                                  }));
                                                                                    }
                                                                                  }));
                                                                    })).then((function (_result) {
                                                                    return $$Node.ChildProcess.exec("unzip cache.zip", {
                                                                                cwd: hiddenEsyRoot
                                                                              });
                                                                  })).then((function (param) {
                                                                  return $$Node.ChildProcess.exec("esy import-dependencies -P " + hiddenEsyRoot, {
                                                                              cwd: hiddenEsyRoot
                                                                            });
                                                                })).then((function (param) {
                                                                return $$Node.ChildProcess.exec("esy build -P " + hiddenEsyRoot, {
                                                                            cwd: hiddenEsyRoot
                                                                          });
                                                              })).then((function (param) {
                                                              return Promise.resolve(/* () */0);
                                                            }));
                                              }));
                                }))
                      ) : (
                        projectType[/* readyForDev */0] ? Promise.resolve(/* () */0) : Vscode.window.withProgress({
                                location: 15,
                                title: "Setting up toolchain..."
                              }, (function (progress) {
                                  progress.report({
                                        increment: 10
                                      });
                                  return $$Node.ChildProcess.exec("esy", {
                                                cwd: folder
                                              }).then((function (param) {
                                                console.log("Finished running esy");
                                                return Promise.resolve(/* () */0);
                                              }));
                                }))
                      )
                  );
                return setupPromise.then((function (param) {
                              if (typeof projectType === "number") {
                                if (process.platform === "win32") {
                                  return Promise.reject([
                                              Caml_builtin_exceptions.failure,
                                              "Opam workflow for Windows is not supported yet"
                                            ]);
                                } else {
                                  return Promise.resolve({
                                              command: "opam",
                                              args: /* array */[
                                                "exec",
                                                "ocamllsp"
                                              ],
                                              options: {
                                                env: process.env
                                              }
                                            });
                                }
                              } else if (projectType.tag) {
                                var match = process.platform === "win32";
                                return Promise.resolve({
                                            command: match ? "esy.cmd" : "esy",
                                            args: /* array */[
                                              "-P",
                                              Path.join(folder, ".vscode", "esy"),
                                              "ocamllsp"
                                            ],
                                            options: {
                                              env: process.env
                                            }
                                          });
                              } else {
                                var match$1 = process.platform === "win32";
                                return Promise.resolve({
                                            command: match$1 ? "esy.cmd" : "esy",
                                            args: /* array */[
                                              "exec-command",
                                              "--include-current-env",
                                              "ocamllsp"
                                            ],
                                            options: {
                                              env: process.env
                                            }
                                          });
                              }
                            }));
              }));
}

var Server = {
  make: make
};

function make$1(param) {
  return {
          documentSelector: /* array */[
            {
              scheme: "file",
              language: "ocaml"
            },
            {
              scheme: "file",
              language: "reason"
            }
          ]
        };
}

var Client = {
  make: make$1
};

var LanguageClient = { };

exports.download = download;
exports.ProjectType = ProjectType;
exports.Server = Server;
exports.Client = Client;
exports.LanguageClient = LanguageClient;
/* fs Not a pure module */

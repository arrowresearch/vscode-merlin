// Generated by BUCKLESCRIPT, PLEASE EDIT WITH CARE
'use strict';

var Fs = require("fs");
var Esy = require("./command/Esy.bs.js");
var Json = require("@glennsl/bs-json/src/Json.bs.js");
var $$Node = require("./bindings/Node.bs.js");
var Path = require("path");
var Block = require("bs-platform/lib/js/block.js");
var Curry = require("bs-platform/lib/js/curry.js");
var Unzip = require("./command/Unzip.bs.js");
var Utils = require("./Utils.bs.js");
var $$Option = require("./Option.bs.js");
var Events = require("events");
var $$Request = require("request");
var Bindings = require("./bindings/Bindings.bs.js");
var Filename = require("bs-platform/lib/js/filename.js");
var AzurePipelines = require("./AzurePipelines.bs.js");
var RequestProgress = require("request-progress");
var CheckBucklescriptCompat = require("./CheckBucklescriptCompat.bs.js");

function onProgress(t, cb) {
  t.on("progress", cb);
  return /* () */0;
}

function onEnd(t, cb) {
  t.on("end", cb);
  return /* () */0;
}

function onError(t, cb) {
  t.on("error", cb);
  return /* () */0;
}

function reportProgress(t, v) {
  t.emit("progress", v);
  return /* () */0;
}

function reportEnd(t) {
  t.emit("end");
  return /* () */0;
}

function reportError(t, errorMsg) {
  t.emit("error", errorMsg);
  return /* () */0;
}

var Internal = {
  onProgress: onProgress,
  onEnd: onEnd,
  onError: onError,
  reportProgress: reportProgress,
  reportEnd: reportEnd,
  reportError: reportError
};

function make(param) {
  return new Events();
}

function run(eventEmitter, projectPath) {
  reportProgress(eventEmitter, 0.1);
  return $$Node.ChildProcess.exec("esy", {
                cwd: projectPath
              }).then((function (param) {
                reportProgress(eventEmitter, 1);
                eventEmitter.emit("end");
                return Promise.resolve(/* () */0);
              }));
}

var Esy$1 = {
  onProgress: onProgress,
  onEnd: onEnd,
  onError: onError,
  reportProgress: reportProgress,
  reportEnd: reportEnd,
  reportError: reportError,
  make: make,
  run: run
};

function make$1(param) {
  return new Events();
}

function run$1(eventEmitter, param) {
  reportProgress(eventEmitter, 1);
  eventEmitter.emit("end");
  return Promise.resolve(/* () */0);
}

var Opam = {
  onProgress: onProgress,
  onEnd: onEnd,
  onError: onError,
  reportProgress: reportProgress,
  reportEnd: reportEnd,
  reportError: reportError,
  make: make$1,
  run: run$1
};

function toString(param) {
  if (typeof param === "number") {
    switch (param) {
      case /* EsyImportDependenciesFailure */1 :
          return "'esy import-dependencies' failed";
      case /* EsyBuildFailure */0 :
      case /* EsyInstallFailure */2 :
          return "'esy install' failed";
      
    }
  } else {
    switch (param.tag | 0) {
      case /* SetupChainFailure */0 :
          return "Setup failed: " + (String(param[0]) + "");
      case /* CacheFailure */1 :
          return " Azure artifacts cache failure: " + (String(param[0]) + " ");
      case /* BucklescriptCompatFailure */2 :
          var msg = CheckBucklescriptCompat.E.toString(param[0]);
          return " BucklescriptCompatFailure: " + (String(msg) + " ");
      case /* InvalidPath */3 :
          return " Setup failed with because of invalid path provided to it: " + (String(param[0]) + " ");
      case /* Failure */4 :
          return " Bsb setup failed. Reason: " + (String(param[0]) + " ");
      
    }
  }
}

var E = {
  toString: toString
};

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

function dropAnEsyJSON(path) {
  return $$Node.Fs.writeFile(path, Bindings.thisProjectsEsyJson);
}

function make$2(param) {
  return new Events();
}

function run$2(eventEmitter, projectPath) {
  var manifestPath = Path.join(projectPath, "package.json");
  var folder = Curry._1(Filename.dirname, manifestPath);
  return $$Node.Fs.readFile(manifestPath).then((function (manifest) {
                  return $$Option.toPromise(/* SetupChainFailure */Block.__(0, ["Failed to parse manifest file"]), $$Option.$great$great$pipe($$Option.$great$great$pipe(Json.parse(manifest), CheckBucklescriptCompat.run), (function (param) {
                                    if (param.tag) {
                                      return Promise.resolve(/* Error */Block.__(1, [/* BucklescriptCompatFailure */Block.__(2, [param[0]])]));
                                    } else {
                                      var folder$1 = folder;
                                      var hiddenEsyRoot = Path.join(folder$1, ".vscode", "esy");
                                      return $$Node.Fs.mkdir(true, hiddenEsyRoot).then((function (param) {
                                                              if (param.tag) {
                                                                return Promise.resolve(/* Error */Block.__(1, [param[0]]));
                                                              } else {
                                                                var path = Filename.concat(hiddenEsyRoot, "esy.json");
                                                                return $$Node.Fs.writeFile(path, Bindings.thisProjectsEsyJson).then((function (param) {
                                                                              return Promise.resolve(/* Ok */Block.__(0, [/* () */0]));
                                                                            }));
                                                              }
                                                            })).then((function (param) {
                                                            if (param.tag) {
                                                              return Promise.resolve(/* Error */Block.__(1, [/* InvalidPath */Block.__(3, [hiddenEsyRoot])]));
                                                            } else {
                                                              return Esy.install(hiddenEsyRoot).then((function (param) {
                                                                            if (param.tag) {
                                                                              return Promise.resolve(/* Error */Block.__(1, [/* EsyInstallFailure */2]));
                                                                            } else {
                                                                              reportProgress(eventEmitter, 0.1);
                                                                              return AzurePipelines.getBuildID(/* () */0).then((function (param) {
                                                                                              if (param.tag) {
                                                                                                return Promise.resolve(/* Error */Block.__(1, [param[0]]));
                                                                                              } else {
                                                                                                return AzurePipelines.getDownloadURL(param[0]);
                                                                                              }
                                                                                            })).then((function (param) {
                                                                                            if (param.tag) {
                                                                                              return Promise.resolve(/* Error */Block.__(1, [/* CacheFailure */Block.__(1, ["<TODO>"])]));
                                                                                            } else {
                                                                                              return Promise.resolve(/* Ok */Block.__(0, [param[0]]));
                                                                                            }
                                                                                          }));
                                                                            }
                                                                          }));
                                                            }
                                                          })).then((function (param) {
                                                          if (param.tag) {
                                                            return Promise.resolve(/* Error */Block.__(1, [/* CacheFailure */Block.__(1, ["Couldn't compute downloadUrl"])]));
                                                          } else {
                                                            var downloadUrl = param[0];
                                                            console.log("download", downloadUrl);
                                                            var lastProgress = {
                                                              contents: 0.0
                                                            };
                                                            return new Promise((function (resolve, param) {
                                                                          return download(downloadUrl, Path.join(hiddenEsyRoot, "cache.zip"), (function (progressFraction) {
                                                                                        var percent = progressFraction * 80.0;
                                                                                        reportProgress(eventEmitter, percent - lastProgress.contents);
                                                                                        lastProgress.contents = percent;
                                                                                        return /* () */0;
                                                                                      }), (function (param) {
                                                                                        return resolve(/* Ok */Block.__(0, [/* () */0]));
                                                                                      }), (function (e) {
                                                                                        return resolve(/* Error */Block.__(1, [/* CacheFailure */Block.__(1, [e.message])]));
                                                                                      }), (function (param) {
                                                                                        return /* () */0;
                                                                                      }));
                                                                        }));
                                                          }
                                                        })).then((function (param) {
                                                        if (param.tag) {
                                                          return Promise.resolve(/* Error */Block.__(1, [param[0]]));
                                                        } else {
                                                          reportProgress(eventEmitter, 93.33);
                                                          return Unzip.run(hiddenEsyRoot, "cache.zip").then((function (param) {
                                                                        if (param.tag) {
                                                                          return Promise.resolve(/* Error */Block.__(1, [/* CacheFailure */Block.__(1, ["Failed to unzip downloaded cache"])]));
                                                                        } else {
                                                                          return Promise.resolve(/* Ok */Block.__(0, [/* () */0]));
                                                                        }
                                                                      }));
                                                        }
                                                      })).then((function (param) {
                                                      if (param.tag) {
                                                        return Promise.resolve(/* Error */Block.__(1, [param[0]]));
                                                      } else {
                                                        reportProgress(eventEmitter, 96.66);
                                                        return Esy.importDependencies(hiddenEsyRoot).then((function (param) {
                                                                      return Utils.$less$less((function (prim) {
                                                                                    return Promise.resolve(prim);
                                                                                  }), (function (param) {
                                                                                    if (param.tag) {
                                                                                      return /* Error */Block.__(1, [/* EsyImportDependenciesFailure */1]);
                                                                                    } else {
                                                                                      return /* Ok */Block.__(0, [/* () */0]);
                                                                                    }
                                                                                  }), param);
                                                                    }));
                                                      }
                                                    })).then((function (param) {
                                                    if (param.tag) {
                                                      return Promise.resolve(/* Error */Block.__(1, [param[0]]));
                                                    } else {
                                                      reportProgress(eventEmitter, 99.99);
                                                      return Esy.build(hiddenEsyRoot).then((function (param) {
                                                                    return Utils.$less$less((function (prim) {
                                                                                  return Promise.resolve(prim);
                                                                                }), (function (param) {
                                                                                  if (param.tag) {
                                                                                    return /* Error */Block.__(1, [/* EsyBuildFailure */0]);
                                                                                  } else {
                                                                                    return /* Ok */Block.__(0, [/* () */0]);
                                                                                  }
                                                                                }), param);
                                                                  }));
                                                    }
                                                  }));
                                    }
                                  })));
                })).then((function (param) {
                if (param.tag) {
                  reportError(eventEmitter, toString(param[0]));
                  return Promise.resolve(/* () */0);
                } else {
                  eventEmitter.emit("end");
                  return Promise.resolve(/* () */0);
                }
              }));
}

var Bsb = {
  E: E,
  onProgress: onProgress,
  onEnd: onEnd,
  onError: onError,
  reportProgress: reportProgress,
  reportEnd: reportEnd,
  reportError: reportError,
  download: download,
  dropAnEsyJSON: dropAnEsyJSON,
  make: make$2,
  run: run$2
};

exports.Internal = Internal;
exports.Esy = Esy$1;
exports.Opam = Opam;
exports.Bsb = Bsb;
/* fs Not a pure module */

[@bs.module "./fs-stub.js"]
external thisProjectsEsyJson: string = "thisProjectsEsyJson";

module CamlArray = Array;
open Js;

[@bs.val] external __dirname: string = "__dirname";

[@bs.val] [@bs.scope "process"]
external processEnv: Js.Dict.t(string) = "env";

[@bs.val] [@bs.scope "process"] external processPlatform: string = "platform";
module Process = {
  type t;
  [@bs.val] external v: t = "process";
  [@bs.val] [@bs.scope "process"] external platform: string = "platform";
  /* TODO [@bs.val] [@bs.scope "process"] external env: Js.Dict.t(string) = "env"; */
  module Stdout = {
    type t;
    [@bs.val] external v: t = "process.stdout";
    [@bs.send] external write: (t, string) => unit = "write";
  };
};

module Error = {
  type t;
  [@bs.new] external make: string => t = "Error";
  let ofPromiseError = [%raw
    error => "return error.message || 'Unknown error'"
  ];
};

module Buffer = {
  type t = {. "byteLength": int};
  [@bs.send] external toString: t => string = "toString";
  [@bs.val] [@bs.scope "Buffer"] external from: string => t = "from";
  let ofString = from;
};

/* Did not work. Was insteaded pointed to https://gist.github.com/mrmurphy/f0a499d4510a358927b8179071145ff9 */
/* [@bs.module "util"] */
/* external promisify: */
/*   (('a, (. Js.Nullable.t(Error.t), 'b) => unit) => unit, 'a) => */
/*   Js.Promise.t('b) = */
/*   "promisify"; */
/* [@bs.module "util"] */
/* external promisify2: */
/*   (('a, 'b, (. Js.Nullable.t(Error.t), 'c) => unit) => unit, 'a, 'b) => */
/*   Js.Promise.t('c) = */
/*   "promisify"; */

module type STREAM = {
  type t;
  let on: (t, string, Buffer.t => unit) => unit;
};

module StreamFunctor = (S: {type t;}) => {
  type t = S.t;
  [@bs.send] external on: (t, string, Buffer.t => unit) => unit = "on";
};

module Stream =
  StreamFunctor({
    type t;
  });

module Fs = {
  /* [@bs.module "fs"] */
  /* external writeFile: */
  /*   (string, string, (. Js.Nullable.t(Error.t), unit) => unit) => unit = */
  /*   "writeFile"; */
  /* let writeFile = promisify2(writeFile); */

  /* [@bs.module "fs"] */
  /* external readFile: */
  /*   (string, (. Js.Nullable.t(Error.t), string) => unit) => unit = */
  /*   "readFile"; */
  /* let readFile = promisify(readFile); */
  type fd;
  [@bs.module "fs"] external writeSync: (. fd, Buffer.t) => unit = "writeSync";

  [@bs.module "./fs-stub.js"]
  external writeFile: (string, string) => Js.Promise.t(unit) = "writeFile";

  [@bs.module "./fs-stub.js"]
  external readFile: string => Js.Promise.t(string) = "readFile";

  [@bs.module "./fs-stub.js"]
  external mkdir': string => Js.Promise.t(unit) = "mkdir";

  [@bs.module "./fs-stub.js"]
  external exists: string => Js.Promise.t(bool) = "exists";

  [@bs.module "./fs-stub.js"]
  external open_: (string, string) => Js.Promise.t(fd) = "open";

  [@bs.module "./fs-stub.js"]
  external write: (fd, Buffer.t) => Js.Promise.t(unit) = "write";

  [@bs.module "./fs-stub.js"]
  external close: (fd, Buffer.t) => Js.Promise.t(unit) = "close";

  [@bs.module "fs"]
  external createWriteStream: string => Stream.t = "createWriteStream";

  [@bs.module "./fs-stub.js"]
  external unlink: string => Js.Promise.t(bool) = "unlink";

  let rec mkdir = (~p=?, path) => {
    let forceCreate =
      switch (p) {
      | Some(x) => x
      | None => false
      };
    Js.Promise.(
      if (forceCreate) {
        exists(path)
        |> then_(doesExist =>
             if (doesExist) {
               resolve();
             } else {
               let homePath = Sys.getenv(Sys.unix ? "HOME" : "USERPROFILE");
               if (path == homePath) {
                 reject(
                   Failure(
                     "mkdir(~p=true) received home path and it was not found",
                   ),
                 );
               } else {
                 mkdir(~p=true, Filename.dirname(path))
                 |> then_(() => mkdir'(path));
               };
             }
           );
      } else {
        mkdir'(path);
      }
    );
  };
};

module ChildProcess = {
  type t;

  module Options = {
    type t;
    [@bs.obj]
    external make: (~cwd: string=?, ~env: Js.Dict.t(string)=?, unit) => t =
      "";
  };

  [@bs.val] [@bs.module "child_process"]
  external exec:
    (string, Options.t, (Js.Nullable.t(Error.t), string, string) => unit) =>
    t /* This should have been `t` - ChildProcess object in Node.js TODO: figure a way to pass it to callback */ =
    "exec";

  let exec = (cmd, options) => {
    Js.Promise.(
      make((~resolve, ~reject) =>
        ignore @@
        exec(cmd, options, (err, stdout, stderr) =>
          if (Js.Nullable.isNullable(err)) {
            resolve(. (stdout, stderr));
          } else {
            Js.log(err);
            reject(. Failure("Error during exec"));
          }
        )
      )
    );
  };
};

module Path = {
  [@bs.module "path"] [@bs.variadic]
  external join: array(string) => string = "join";

  [@bs.module "path"] external basename: string => string = "basename";

  [@bs.module "path"] external dirname: string => string = "dirname";
};

module Response = {
  type t = {
    .
    "statusCode": int,
    "headers": Js.Dict.t(string),
  };
  [@bs.send] external setEncoding: (t, string) => unit = "setEncoding";
  [@bs.send] external on: (t, string, Buffer.t => unit) => unit = "on";
};

module Request = {
  [@bs.module] external request: string => Stream.t = "request";
};

module RequestProgress = {
  type t;
  type state = {
    .
    "percent": float,
    "speed": int,
    "size": {
      .
      "total": int,
      "transferred": int,
    },
    "time": {
      .
      "elapsed": float,
      "remaining": float,
    },
  };
  [@bs.module] external requestProgress: Stream.t => t = "request-progress";
  [@bs.send] external onData': (t, string, Buffer.t => unit) => unit = "on";
  let onData = (t, cb) => onData'(t, "data", cb);
  [@bs.send] external onProgress': (t, string, state => unit) => unit = "on";
  let onProgress = (t, cb) => {
    onProgress'(t, "progress", cb);
  };
  [@bs.send] external onError': (t, string, Error.t => unit) => unit = "on";
  let onError = (t, cb) => {
    onError'(t, "error", cb);
  };
  [@bs.send] external onEnd': (t, string, unit => unit) => unit = "on";
  let onEnd = (t, cb) => {
    onEnd'(t, "end", cb);
  };
  [@bs.send] external pipe: (t, Stream.t) => unit = "pipe";
};

module Https = {
  [@bs.module "https"]
  external get: (string, Response.t => unit) => unit = "get";
  let getCompleteResponse = url =>
    Promise.make((~resolve, ~reject as _) => {
      get(
        url,
        response => {
          let _statusCode = response##statusCode;
          let responseText = ref("");
          Response.on(response, "data", c => {
            responseText := responseText^ ++ Buffer.toString(c)
          });
          Response.on(response, "end", _ => {resolve(. Ok(responseText^))});
          Response.on(response, "error", _err => {
            resolve(. Error({j|Failed to fetch $url|j}))
          });
        },
      )
    });
};

module Os = {
  [@bs.module "os"] external tmpdir: unit => string = "tmpdir";
};

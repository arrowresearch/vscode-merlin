[@bs.val] external __dirname: string = "__dirname";

[@bs.val] [@bs.scope "process"]
external processEnv: Js.Dict.t(string) = "env";

[@bs.val] [@bs.scope "process"] external processPlatform: string = "platform";

module Error = {
  type t;
  [@bs.new] external make: string => t = "Error";
  let ofPromiseError: Js.Promise.error => string = [%raw
    error => "return error.message || 'Unknown error'"
  ];
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

  [@bs.module "./fs-stub.js"]
  external writeFile: (string, string) => Js.Promise.t(unit) = "writeFile";

  [@bs.module "./fs-stub.js"]
  external readFile: string => Js.Promise.t(string) = "readFile";

  [@bs.module "./fs-stub.js"]
  external mkdir': string => Js.Promise.t(unit) = "mkdir";

  [@bs.module "./fs-stub.js"]
  external exists: string => Js.Promise.t(bool) = "exists";

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
};

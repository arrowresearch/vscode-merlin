open Bindings;

/* False bindings to emulate EventEmitters. Node.js event emitters
   take unrestricted types on their .emit() methods */
module Internal = {
  type t;
  [@bs.module] [@bs.new] external make: unit => t = "events";
  [@bs.send] external onProgress': (t, string, float => unit) => unit = "on";
  let onProgress = (t, cb) => {
    onProgress'(t, "progress", cb);
  };

  [@bs.send] external onEnd': (t, string, unit => unit) => unit = "on";
  let onEnd = (t, cb) => {
    onEnd'(t, "end", cb);
  };

  [@bs.send] external onError': (t, string, string => unit) => unit = "on";
  let onError = (t, cb) => {
    onError'(t, "error", cb);
  };

  [@bs.send] external reportProgress': (t, string, float) => unit = "emit";
  let reportProgress = (t, v) => {
    reportProgress'(t, "progress", v);
  };

  [@bs.send] external reportEnd': (t, string) => unit = "emit";
  let reportEnd = t => {
    reportEnd'(t, "end");
  };

  [@bs.send] external reportError': (t, string, string) => unit = "emit";
  let reportError = (t, errorMsg) => {
    reportError'(t, "error", errorMsg);
  };
};

module Esy = {
  open Js.Promise;
  open ChildProcess;
  include Internal;
  let make = () => make();
  let run = (eventEmitter, projectPath) => {
    reportProgress(eventEmitter, 0.1);
    exec("esy", Options.make(~cwd=projectPath, ()))
    |> then_(_ => {
         reportProgress(eventEmitter, 1.);
         reportEnd(eventEmitter);
         resolve();
       });
  };
};

module Opam = {
  open Js.Promise;
  open ChildProcess;
  include Internal;
  let make = () => make();
  let run = (eventEmitter, _) => {
    reportProgress(eventEmitter, 1.);
    reportEnd(eventEmitter);
    resolve();
  };
};

module Bsb = {
  open Utils;
  include Internal;
  let download = (url, file, ~progress, ~end_, ~error, ~data) => {
    let stream = RequestProgress.requestProgress(Request.request(url));
    RequestProgress.onProgress(stream, state => {
      progress(
        float_of_int(state##size##transferred) /. (134. *. 1024. *. 1024.),
      )
    });
    RequestProgress.onData(stream, data);
    RequestProgress.onEnd(stream, end_);
    RequestProgress.onError(stream, error);
    RequestProgress.pipe(stream, Fs.createWriteStream(file));
  };

  let dropAnEsyJSON = path => {
    Fs.writeFile(path, thisProjectsEsyJson);
  };

  let make = () => make();
  let run = (eventEmitter, projectPath) => {
    let manifestPath = Path.join([|projectPath, "package.json"|]);
    let runEsyCommands = folder => {
      let hiddenEsyRoot = Path.join([|folder, ".vscode", "esy"|]);
      Js.Promise.(
        {
          Fs.mkdir(~p=true, hiddenEsyRoot)
          |> then_(_ => {
               Filename.concat(hiddenEsyRoot, "esy.json") |> dropAnEsyJSON
             })
          |> then_(() => {
               /* Running esy */
               ChildProcess.exec(
                 "esy i -P " ++ hiddenEsyRoot,
                 ChildProcess.Options.make(~cwd=projectPath, ()),
               )
             })
          |> then_(((_stdout, _stderr)) => {
               reportProgress(eventEmitter, 0.1);
               AzurePipelines.getBuildID();
             })
          |> then_(AzurePipelines.getDownloadURL)
          |> then_(r =>
               switch (r) {
               | Ok(downloadUrl) =>
                 Js.log2("download", downloadUrl);
                 let lastProgress = ref(0.0);
                 Js.Promise.make((~resolve, ~reject as _) =>
                   download(
                     downloadUrl,
                     Path.join([|hiddenEsyRoot, "cache.zip"|]),
                     ~progress=
                       progressFraction => {
                         let percent = progressFraction *. 80.0;
                         reportProgress(
                           eventEmitter,
                           percent -. lastProgress^,
                         );
                         lastProgress := percent;
                       },
                     ~data=_ => (),
                     ~error=
                       _e => {
                         resolve(.
                           Error({j|Failed to download $downloadUrl |j}),
                         )
                       },
                     ~end_=() => {resolve(. Ok())},
                   )
                 );
               | Error(x) => resolve(Error(x))
               }
             )
          |> then_(_result => {
               reportProgress(eventEmitter, 93.33);
               ChildProcess.exec(
                 "unzip cache.zip",
                 ChildProcess.Options.make(~cwd=hiddenEsyRoot, ()),
               );
             })
          |> then_(_ => {
               reportProgress(eventEmitter, 96.66);
               ChildProcess.exec(
                 "esy import-dependencies -P " ++ hiddenEsyRoot,
                 ChildProcess.Options.make(~cwd=hiddenEsyRoot, ()),
               );
             })
          |> then_(_ => {
               reportProgress(eventEmitter, 99.99);
               ChildProcess.exec(
                 "esy build -P " ++ hiddenEsyRoot,
                 ChildProcess.Options.make(~cwd=hiddenEsyRoot, ()),
               );
             })
          |> then_(_ => {resolve(Ok())});
        }
      );
    };
    Js.Promise.(
      {
        let folder = Filename.dirname(manifestPath);
        Fs.readFile(manifestPath)
        |> then_(manifest => {
             Option.(
               Json.(
                 parse(manifest)
                 >>| CheckBucklescriptCompat.run
                 >>| (
                   fun
                   | Ok () => runEsyCommands(folder)
                   | Error(e) => resolve(Error(e))
                 )
               )
               |> toPromise("Failed to parse manifest file")
             )
           })
        |> then_(
             fun
             | Ok () => {
                 reportEnd(eventEmitter);
                 resolve();
               }
             | Error(msg) => {
                 reportError(eventEmitter, msg);
                 resolve();
               },
           );
      }
    );
  };
};

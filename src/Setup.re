open Bindings;

module Esy = {
  open Js.Promise;
  let run = projectPath => {
    Window.withProgress(
      {
        "location": 15, /* Window.(locationToJs(Notification)) */
        "title": "Setting up toolchain...",
      },
      progress => {
        progress.report(. {"increment": 10});
        ChildProcess.exec(
          "esy",
          ChildProcess.Options.make(~cwd=projectPath, ()),
        )
        |> then_(((_stdout, _stderr)) => {
             Js.log("Finished running esy");
             resolve();
           });
      },
    )
    |> then_(() => resolve(Ok()));
  };
};

module Opam = {
  let run = () => Js.Promise.resolve();
};

module Bsb = {
  open Utils;

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

  let run = projectPath => {
    let manifestPath = Path.join([|projectPath, "package.json"|]);
    let runEsyCommands = folder => {
      let hiddenEsyRoot = Path.join([|folder, ".vscode", "esy"|]);
      Js.Promise.(
        Window.(
          withProgress(
            {
              "location": 15 /* Window.(locationToJs(Notification)) */,
              "title": "Setting up toolchain...",
            },
            progress => {
              progress.report(. {"increment": 10});
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
                   progress.report(. {"increment": 10});
                   AzurePipelines.getBuildID();
                 })
              |> then_(AzurePipelines.getDownloadURL)
              |> then_(r =>
                   switch (r) {
                   | Ok(downloadUrl) =>
                     Js.log2("download", downloadUrl);
                     let lastProgress = ref(0);
                     Js.Promise.make((~resolve, ~reject as _) =>
                       download(
                         downloadUrl,
                         Path.join([|hiddenEsyRoot, "cache.zip"|]),
                         ~progress=
                           progressFraction => {
                             let percent =
                               int_of_float(progressFraction *. 80.0);
                             progress.report(. {
                               "increment": percent - lastProgress^,
                             });
                             lastProgress := percent;
                           },
                         ~data=_ => (),
                         ~error=
                           _e =>
                             resolve(.
                               Error({j|Failed to download $downloadUrl |j}),
                             ),
                         ~end_=() => {resolve(. Ok())},
                       )
                     );
                   | Error(x) => resolve(Error(x))
                   }
                 )
              |> then_(_result => {
                   ChildProcess.exec(
                     "unzip cache.zip",
                     ChildProcess.Options.make(~cwd=hiddenEsyRoot, ()),
                   )
                 })
              |> then_(_ => {
                   ChildProcess.exec(
                     "esy import-dependencies -P " ++ hiddenEsyRoot,
                     ChildProcess.Options.make(~cwd=hiddenEsyRoot, ()),
                   )
                 })
              |> then_(_ => {
                   ChildProcess.exec(
                     "esy build -P " ++ hiddenEsyRoot,
                     ChildProcess.Options.make(~cwd=hiddenEsyRoot, ()),
                   )
                 })
              |> then_(_ => resolve());
            },
          )
        )
        |> then_(() => Ok() |> resolve)
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
           });
      }
    );
  };
};

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
    );
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

  let dropAnEsyJSON = (~compilerVersion, ~folder) => {
    let esyJsonTargetDir = Path.join([|folder, ".vscode", "esy"|]);
    Js.Promise.(
      Fs.mkdir(~p=true, esyJsonTargetDir)
      |> then_(() =>
           Fs.writeFile(
             Filename.concat(esyJsonTargetDir, "esy.json"),
             thisProjectsEsyJson,
           )
         )
    );
  };

  let processDeps = (dependenciesJson, folder) => {
    Js.Promise.(
      Js.Json.(
        switch (dependenciesJson->object_->classify) {
        | JSONObject(dependenciesDict) =>
          switch (Js.Dict.get(dependenciesDict, "bs-platform")) {
          | Some(bsPlatformVersionJson) =>
            switch (Js.Json.classify(bsPlatformVersionJson)) {
            | JSONString(bsPlatformVersion) =>
              if (bsPlatformVersion
                  ->Semver.minVersion
                  ->Semver.satisfies(">=6.0.0")) {
                dropAnEsyJSON(~folder, ~compilerVersion="4.6.x");
              } else {
                dropAnEsyJSON(~folder, ~compilerVersion="4.2.x");
              }
            | _ =>
              reject(
                Failure(
                  "'bs-platform' (in dependencies section) was expected to contain a semver string, but it was not!",
                ),
              )
            }

          | None =>
            reject(
              Failure(
                "'bs-platform' was expected in the 'dependencies' section of the manifest file, but was not found!",
              ),
            )
          }
        | _ =>
          reject(
            Failure(
              "'dependencies' section in the manifest file was expected to be dictionary, but it was not!",
            ),
          )
        }
      )
    );
  };

  let run = projectPath => {
    let manifestPath = Path.join([|projectPath, "package.json"|]);
    Js.Promise.(
      Js.Json.(
        {
          let folder = Filename.dirname(manifestPath);
          Fs.readFile(manifestPath)
          |> then_(manifest => {
               let manifestJson = parseExn(manifest);
               switch (classify(manifestJson)) {
               | JSONObject(dict) =>
                 switch (
                   getSubDict(dict, "dependencies"),
                   getSubDict(dict, "devDependencies"),
                 ) {
                 | (Some(dependenciesJson), None)
                 | (None, Some(dependenciesJson)) =>
                   processDeps(dependenciesJson, folder)
                 | (Some(dependenciesJson), Some(devDependenciesJson)) =>
                   processDeps(
                     mergeDicts(dependenciesJson, devDependenciesJson),
                     folder,
                   )
                 | (None, None) =>
                   reject(
                     Failure(
                       "The manifest file doesn't seem to contain `dependencies` or `devDependencies` property",
                     ),
                   )
                 }
               | _ =>
                 reject(
                   Failure(
                     "The entire manifest was expected to be dictionary of key-vals, but it was not!:",
                   ),
                 )
               };
             });
        }
      )
      |> then_(() => {
           Window.(
             withProgress(
               {
                 "location": 15 /* Window.(locationToJs(Notification)) */,
                 "title": "Setting up toolchain...",
               },
               progress => {
                 progress.report(. {"increment": 10});
                 /* Running esy */
                 let hiddenEsyRoot =
                   Path.join([|projectPath, ".vscode", "esy"|]);
                 ChildProcess.exec(
                   "esy i -P " ++ hiddenEsyRoot,
                   ChildProcess.Options.make(~cwd=projectPath, ()),
                 )
                 |> then_(((_stdout, _stderr)) => {
                      progress.report(. {"increment": 10});
                      Js.(
                        Promise.(
                          AzurePipelines.getBuildID()
                          |> then_(AzurePipelines.getDownloadURL)
                          |> then_(r =>
                               switch (r) {
                               | Ok(downloadUrl) =>
                                 Js.log2("download", downloadUrl);
                                 let lastProgress = ref(0);
                                 Promise.make((~resolve, ~reject as _) =>
                                   download(
                                     downloadUrl,
                                     Path.join([|
                                       hiddenEsyRoot,
                                       "cache.zip",
                                     |]),
                                     ~progress=
                                       progressFraction => {
                                         let percent =
                                           int_of_float(
                                             progressFraction *. 80.0,
                                           );
                                         progress.report(. {
                                           "increment":
                                             percent - lastProgress^,
                                         });
                                         lastProgress := percent;
                                       },
                                     ~data=_ => (),
                                     ~error=
                                       e =>
                                         resolve(.
                                           Error(
                                             {j|Failed to download $downloadUrl |j},
                                           ),
                                         ),
                                     ~end_=() => {resolve(. Ok())},
                                   )
                                 );
                               | Error(x) => resolve(Error(x))
                               }
                             )
                        )
                      );
                    })
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
         })
    );
  };
};

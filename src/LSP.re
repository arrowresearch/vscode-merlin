open Bindings;

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

module Server = {
  type processOptions = {env: Js.Dict.t(string)};
  type options = {
    command: string,
    args: array(string),
    options: processOptions,
  };
  let make = folder => {
    Js.Dict.set(processEnv, "OCAMLRUNPARAM", "b");
    Js.Dict.set(processEnv, "MERLIN_LOG", "-");
    Js.Promise.(
      ProjectType.(
        {
          ProjectType.detect(folder)
          |> then_(projectType => {
               let setupPromise =
                 switch (projectType) {
                 | Esy({readyForDev}) =>
                   if (readyForDev) {
                     resolve();
                   } else {
                     Window.withProgress(
                       {
                         "location": 15, /* Window.(locationToJs(Notification)) */
                         "title": "Setting up toolchain...",
                       },
                       progress => {
                         progress.report(. {"increment": 10});
                         ChildProcess.exec(
                           "esy",
                           ChildProcess.Options.make(~cwd=folder, ()),
                         )
                         |> then_(((_stdout, _stderr)) => {
                              Js.log("Finished running esy");
                              resolve();
                            });
                       },
                     );
                   }
                 | Opam => resolve()
                 | Bsb({readyForDev}) =>
                   if (readyForDev) {
                     resolve();
                   } else {
                     /* Not ready for dev. Setting up */
                     Esy.setup(
                       ~manifestPath=Path.join([|folder, "package.json"|]),
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
                                  Path.join([|folder, ".vscode", "esy"|]);
                                ChildProcess.exec(
                                  "esy i -P " ++ hiddenEsyRoot,
                                  ChildProcess.Options.make(~cwd=folder, ()),
                                )
                                |> then_(((_stdout, _stderr)) => {
                                     progress.report(. {"increment": 10});
                                     Js.(
                                       Promise.(
                                         AzurePipelines.getBuildID()
                                         |> then_(
                                              AzurePipelines.getDownloadURL,
                                            )
                                         |> then_(r =>
                                              switch (r) {
                                              | Ok(downloadUrl) =>
                                                Js.log2(
                                                  "download",
                                                  downloadUrl,
                                                );
                                                let lastProgress = ref(0);
                                                Promise.make(
                                                  (~resolve, ~reject as _) =>
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
                                                            progressFraction
                                                            *. 80.0,
                                                          );
                                                        progress.report(. {
                                                          "increment":
                                                            percent
                                                            - lastProgress^,
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
                                                    ~end_=
                                                      () => {resolve(. Ok())},
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
                                       ChildProcess.Options.make(
                                         ~cwd=hiddenEsyRoot,
                                         (),
                                       ),
                                     )
                                   })
                                |> then_(_ => {
                                     ChildProcess.exec(
                                       "esy import-dependencies -P "
                                       ++ hiddenEsyRoot,
                                       ChildProcess.Options.make(
                                         ~cwd=hiddenEsyRoot,
                                         (),
                                       ),
                                     )
                                   })
                                |> then_(_ => {
                                     ChildProcess.exec(
                                       "esy build -P " ++ hiddenEsyRoot,
                                       ChildProcess.Options.make(
                                         ~cwd=hiddenEsyRoot,
                                         (),
                                       ),
                                     )
                                   })
                                |> then_(_ => resolve());
                              },
                            )
                          )
                        });
                   }
                 };
               setupPromise
               |> then_(() => {
                    switch (projectType) {
                    | Esy(_) =>
                      resolve({
                        command:
                          Process.platform == "win32" ? "esy.cmd" : "esy",
                        args: [|
                          "exec-command",
                          "--include-current-env",
                          "ocamllsp",
                        |],
                        options: {
                          env: processEnv,
                        },
                      })
                    | Opam =>
                      if (processPlatform == "win32") {
                        reject(
                          Failure(
                            "Opam workflow for Windows is not supported yet",
                          ),
                        );
                      } else {
                        resolve({
                          command: "opam",
                          args: [|"exec", "ocamllsp"|],
                          options: {
                            env: processEnv,
                          },
                        });
                      }
                    | Bsb(_) =>
                      resolve({
                        command:
                          Bindings.processPlatform == "win32"
                            ? "esy.cmd" : "esy",
                        args: [|
                          "-P",
                          Path.join([|folder, ".vscode", "esy"|]),
                          "ocamllsp",
                        |],
                        options: {
                          env: processEnv,
                        },
                      })
                    }
                  });
             });
        }
      )
    );
  };
};

module Client = {
  type documentSelectorItem = {
    scheme: string,
    language: string,
  };

  type options = {documentSelector: array(documentSelectorItem)};

  let make = () => {
    {
      documentSelector: [|
        {scheme: "file", language: "ocaml"},
        {scheme: "file", language: "reason"},
      |],
    };
  };
};

module LanguageClient = {
  type t = {
    start: (. unit) => unit,
    stop: (. unit) => unit,
  };
  [@bs.new] [@bs.module "vscode-languageclient"]
  external make:
    (
      ~id: string,
      ~name: string,
      ~serverOptions: Server.options,
      ~clientOptions: Client.options
    ) =>
    t =
    "LanguageClient";
};

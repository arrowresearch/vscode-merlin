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

module ProjectType = {
  type t =
    | Esy({readyForDev: bool})
    | Opam
    | Bsb({readyForDev: bool});

  let detect = folder => {
    Js.Promise.(
      Esy.getStatus(folder)
      |> then_((status: Esy.status) =>
           if (status.isProject) {
             /* All npm and opam projects are valid esy projects too! Picking
                the right package manager is important - we don't want to run
                `esy` for a user who never intended to. Example: bsb/npm
                users. Similarly, opam users wouldn't want prompts to run
                `esy`. Why is prompting `esy i` even necessary in the first
                place? `esy ocamllsp` needs projects to install/solve deps*/

             let manifestFile =
               switch (Js.Nullable.toOption(status.rootPackageConfigPath)) {
               | Some(x) => x
               | None => ""
               };

             /* Looking for manifest file. If it is not a json file, it's an opam project */
             if (Js.Re.fromString(".json$")->Js.Re.test_(manifestFile)) {
               /* if isProjectReadyForDev is true, it's an esy project. So, there's no need for any setup */
               if (status.isProjectReadyForDev) {
                 /* Definitely an esy project */
                 Esy({readyForDev: true}) |> resolve;
               } else {
                 /* Could be an esy project withouts deps installed, or a bsb project */
                 /* Looking into the manifest file for more details */
                 Fs.readFile(manifestFile)
                 |> then_(manifest =>
                      Js.Json.(
                        {
                          let manifestJson = parseExn(manifest);
                          let manifestHasEsyConfig =
                            Utils.propertyExists(manifestJson, "esy"); /* ie "esy" exists in a package.json */
                          let manifestIsEsyJSON =
                            Js.Re.fromString("esy.json$")
                            ->Js.Re.test_(manifestFile);
                          if (manifestIsEsyJSON || manifestHasEsyConfig) {
                            /* Definitely an esy project */
                            resolve(
                              Esy({readyForDev: status.isProjectReadyForDev}),
                            );
                          } else {
                            /* Must be a bsb project. We install the toolchain using esy */
                            let esyToolChainFolder =
                              Path.join([|folder, ".vscode", "esy"|]);
                            esyToolChainFolder
                            |> Fs.exists
                            |> then_(doesToolChainEsyManifestExist =>
                                 if (doesToolChainEsyManifestExist) {
                                   Esy.getStatus(esyToolChainFolder)
                                   |> then_((toolChainStatus: Esy.status) =>
                                        if (!toolChainStatus.isProject) {
                                          reject(
                                            Failure(
                                              "Weird invariant violation. Why would .vscode/esy exist but not be a valid esy project. TODO",
                                            ),
                                          );
                                        } else {
                                          Bsb({
                                            readyForDev:
                                              toolChainStatus.isProjectSolved,
                                          })
                                          |> resolve;
                                        }
                                      );
                                 } else {
                                   Bsb({readyForDev: false}) |> resolve;
                                 }
                               );
                          };
                        }
                      )
                    );
               };
             } else {
               Opam |> resolve;
             };
           } else {
             reject(Failure("Not a valid esy/opam/bsb project"));
           }
         )
    );
  };
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

                                     let mapResultAndResolvePromise = (f, r) =>
                                       Js.Promise.(Result.(r >| f |> resolve));

                                     let bindResultAndResolvePromise = (f, r) =>
                                       Js.Promise.(
                                         Result.(r >>= f |> resolve)
                                       );

                                     let restBase = "https://dev.azure.com/arrowresearch/";
                                     let proj = "vscode-merlin";
                                     let os =
                                       switch (Process.platform) {
                                       | "darwin" => "Darwin"
                                       | "linux" => "Linux"
                                       | "win32" => "Windows_NT"
                                       };
                                     let artifactName = {j|cache-$os-install|j};
                                     let master = "branchName=refs%2Fheads%2Fmaster";
                                     let filter = "deletedFilter=excludeDeleted&statusFilter=completed&resultFilter=succeeded";
                                     let latest = "queryOrder=finishTimeDescending&$top=1";
                                     Js.(
                                       Promise.(
                                         Https.getCompleteResponse(
                                           {j|$restBase/$proj/_apis/build/builds?$filter&$master&$latest&api-version=4.1|j},
                                         )
                                         |> then_(
                                              bindResultAndResolvePromise(
                                                responseText =>
                                                try({
                                                  let json =
                                                    Json.parseExn(
                                                      responseText,
                                                    );
                                                  switch (
                                                    Js.Json.classify(json)
                                                  ) {
                                                  | JSONObject(dict) =>
                                                    switch (
                                                      Dict.get(dict, "value")
                                                    ) {
                                                    | None =>
                                                      Error(
                                                        "Field 'value' in Azure's response was undefined",
                                                      )
                                                    | Some(json) =>
                                                      switch (
                                                        Js.Json.classify(json)
                                                      ) {
                                                      | JSONArray(builds) =>
                                                        let o =
                                                          CamlArray.get(
                                                            builds,
                                                            0,
                                                          );
                                                        switch (
                                                          Json.classify(o)
                                                        ) {
                                                        | JSONObject(dict) =>
                                                          switch (
                                                            Dict.get(
                                                              dict,
                                                              "id",
                                                            )
                                                          ) {
                                                          | Some(id) =>
                                                            switch (
                                                              Json.classify(
                                                                id,
                                                              )
                                                            ) {
                                                            | JSONNumber(n) =>
                                                              Ok(n)
                                                            | _ =>
                                                              Error(
                                                                {j| Field id was expected to be a number |j},
                                                              )
                                                            }
                                                          | None =>
                                                            Error(
                                                              {j| Field id was missing |j},
                                                            )
                                                          }
                                                        | _ =>
                                                          Error(
                                                            {j| First item in the 'value' field array isn't an object as expected |j},
                                                          )
                                                        };

                                                      | _ =>
                                                        Error(
                                                          {j| Response from Azure did not contain build 'value' |j},
                                                        )
                                                      }
                                                    }
                                                  | _ =>
                                                    Error(
                                                      {j| Response from Azure wasn't an object |j},
                                                    )
                                                  };
                                                }) {
                                                | _ =>
                                                  Error(
                                                    {j| Failed to parse response from Azure |j},
                                                  )
                                                }
                                              ),
                                            )
                                         |> then_(r => {
                                              switch (r) {
                                              | Ok(latestBuildID) =>
                                                Https.getCompleteResponse(
                                                  {j|$restBase/$proj/_apis/build/builds/$latestBuildID/artifacts?artifactname=$artifactName&api-version=4.1|j},
                                                )
                                              | Error(x) => resolve(Error(x))
                                              }
                                            })
                                         |> then_(
                                              bindResultAndResolvePromise(
                                                responseText =>
                                                try({
                                                  let json =
                                                    Json.parseExn(
                                                      responseText,
                                                    );
                                                  switch (
                                                    Js.Json.classify(json)
                                                  ) {
                                                  | JSONObject(dict) =>
                                                    switch (
                                                      Dict.get(
                                                        dict,
                                                        "resource",
                                                      )
                                                    ) {
                                                    | None =>
                                                      Error(
                                                        "Field 'value' in Azure's response was undefined",
                                                      )
                                                    | Some(json) =>
                                                      switch (
                                                        Json.classify(json)
                                                      ) {
                                                      | JSONObject(dict) =>
                                                        switch (
                                                          Dict.get(
                                                            dict,
                                                            "downloadUrl",
                                                          )
                                                        ) {
                                                        | Some(id) =>
                                                          switch (
                                                            Json.classify(id)
                                                          ) {
                                                          | JSONString(s) =>
                                                            Ok(s)
                                                          | _ =>
                                                            Error(
                                                              {j| Field downloadUrl was expected to be a string |j},
                                                            )
                                                          }
                                                        | None =>
                                                          Error(
                                                            {j| Field downloadUrl was missing |j},
                                                          )
                                                        }
                                                      | _ =>
                                                        Error(
                                                          {j| First item in the 'resource' field array isn't an object as expected |j},
                                                        )
                                                      }
                                                    }
                                                  | _ =>
                                                    Error(
                                                      {j| Response from Azure wasn't an object |j},
                                                    )
                                                  };
                                                }) {
                                                | _ =>
                                                  Error(
                                                    {j| Failed to parse response from Azure |j},
                                                  )
                                                }
                                              ),
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
                                                        Js.log3(
                                                          "incremented by",
                                                          percent,
                                                          lastProgress^,
                                                        );
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
                                              | Error(x) =>
                                                Js.log2("Error>>>>>>", x);
                                                resolve(Error(x));
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

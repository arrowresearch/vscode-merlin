open Bindings;

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
                place? `esy ocamlmerlin-lsp` needs projects to install/solve deps*/

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
                                ChildProcess.exec(
                                  "esy -P "
                                  ++ Path.join([|folder, ".vscode", "esy"|]),
                                  ChildProcess.Options.make(~cwd=folder, ()),
                                )
                                |> then_(((_stdout, _stderr)) => {
                                     Js.log("Finished running esy");
                                     resolve();
                                   });
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
                          Bindings.processPlatform == "win32"
                            ? "esy.cmd" : "esy",
                        args: [|
                          "exec-command",
                          "--include-current-env",
                          "ocamlmerlin-lsp",
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
                          args: [|"exec", "ocamlmerlin-lsp"|],
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
                          "ocamlmerlin-lsp",
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

module MultiWorkspace = {
  module FoldersMap = Belt.HashMap.String;
  let start = (~context, ~commands, ~createClient): Js.Promise.t(unit) => {
    let workspaceFolders = FoldersMap.make(~hintSize=1);
    let startClient = (document: TextDocument.event) => {
      let uri = document.uri;
      if (uri.scheme === "file") {
        let folder = Workspace.getWorkspaceFolder(uri);
        switch (folder) {
        | Some(folder)
            when FoldersMap.has(workspaceFolders, folder.uri.fsPath) =>
          Js.Promise.resolve()
        | Some(folder) =>
          createClient(document, folder)
          |> Js.Promise.then_(client =>
               switch (client) {
               | Ok(client) =>
                 FoldersMap.set(
                   workspaceFolders,
                   Folder.key(folder),
                   client,
                 );
                 client.LanguageClient.start(.) |> Js.Promise.resolve;
               | Error(message) =>
                 Window.showErrorMessage({j|Error: $message|j})
               }
             )
        | None => Js.Promise.resolve()
        };
      } else {
        Js.Promise.resolve();
      };
    };
    Workspace.onDidOpenTextDocument(document =>
      startClient(document) |> ignore
    );
    let openTasks =
      Workspace.textDocuments
      |> Js.Array.map(startClient)
      |> Js.Promise.all
      |> Js.Promise.then_(_ => Js.Promise.resolve());
    Workspace.onDidChangeWorkspaceFolders(event => {
      event.removed
      |> Js.Array.forEach(folder => {
           switch (FoldersMap.get(workspaceFolders, Folder.key(folder))) {
           | Some(client) =>
             FoldersMap.remove(workspaceFolders, Folder.key(folder));
             client.LanguageClient.stop(.);
           | None => ()
           }
         })
    });

    commands
    |> Js.Array.forEach(((cmd, handler)) => {
         Commands.register(~command=cmd, ~handler)
       });

    ExtensionContext.(
      Js.Array.push(
        {
          dispose:
            (.) => {
              FoldersMap.forEach(workspaceFolders, (_, client) => {
                client.LanguageClient.stop(.)
              });
              FoldersMap.clear(workspaceFolders);
            },
        },
        context.subscriptions,
      )
    )
    |> ignore;
    openTasks;
  };
};

open Bindings;

module Server = {
  open Js.Promise;
  open ProjectType;
  type processOptions = {env: Js.Dict.t(string)};
  type options = {
    command: string,
    args: array(string),
    options: processOptions,
  };
  let make = folder => {
    Js.Dict.set(processEnv, "OCAMLRUNPARAM", "b");
    Js.Dict.set(processEnv, "MERLIN_LOG", "-");
    ProjectType.detect(folder)
    |> then_(projectType => {
         let setupPromise =
           switch (projectType) {
           | Esy({readyForDev}) =>
             if (readyForDev) {
               resolve(Ok());
             } else {
               Setup.Esy.run(folder);
             }
           | Opam => resolve(Ok())
           | Bsb({readyForDev}) =>
             if (readyForDev) {
               resolve(Ok());
             } else {
               /* Not ready for dev. Setting up */
               Setup.Bsb.run(folder);
             }
           };
         setupPromise
         |> then_(r => {
              switch (r) {
              | Ok () =>
                switch (projectType) {
                | Esy(_) =>
                  Ok({
                    command: Process.platform == "win32" ? "esy.cmd" : "esy",
                    args: [|
                      "exec-command",
                      "--include-current-env",
                      "ocamllsp",
                    |],
                    options: {
                      env: processEnv,
                    },
                  })
                  |> resolve
                | Opam =>
                  if (processPlatform == "win32") {
                    Error("Opam workflow for Windows is not supported yet")
                    |> resolve;
                  } else {
                    Ok({
                      command: "opam",
                      args: [|"exec", "ocamllsp"|],
                      options: {
                        env: processEnv,
                      },
                    })
                    |> resolve;
                  }
                | Bsb(_) =>
                  Ok({
                    command:
                      Bindings.processPlatform == "win32" ? "esy.cmd" : "esy",
                    args: [|
                      "-P",
                      Path.join([|folder, ".vscode", "esy"|]),
                      "ocamllsp",
                    |],
                    options: {
                      env: processEnv,
                    },
                  })
                  |> resolve
                }
              | Error(e) => resolve(Error(e))
              }
            });
       });
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

open Bindings;
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

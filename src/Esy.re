type status = {
  isProject: bool,
  isProjectSolved: bool,
  isProjectFetched: bool,
  isProjectReadyForDev: bool,
  rootBuildPath: Js.Nullable.t(string),
  rootInstallPath: Js.Nullable.t(string),
  rootPackageConfigPath: Js.Nullable.t(string),
};

open Bindings;

exception UnexpectedJSONValue(Js.Json.tagged_t);
exception Stderr(string);
exception UnknownError(string);
exception JSError(string);

let bool' =
  Js.Json.(
    fun
    | JSONTrue => true
    | JSONFalse => false
    | _ as x => raise(UnexpectedJSONValue(x))
  );

let raiseIfNone =
  fun
  | Some(x) => x
  | None => failwith("Found None where it was not expected");

let bool_ = x => x |> raiseIfNone |> Js.Json.classify |> bool';

let nullableString' =
  Js.Json.(
    fun
    | JSONString(x) => Js.Nullable.return(x)
    | JSONNull => Js.Nullable.null
    | _ as x => raise(UnexpectedJSONValue(x))
  );

let nullableString =
  fun
  | Some(x) => x |> Js.Json.classify |> nullableString'
  | None => Js.Nullable.null;

let getStatus = path => {
  Js.Promise.(
    ChildProcess.exec("esy status", ChildProcess.Options.make(~cwd=path, ()))
    |> then_(((statusOutputString, statusErrorString)) =>
         if (statusErrorString == "") {
           Js.Json.(
             try({
               let json = parseExn(statusOutputString);
               /* Getting key-value pairs */
               let dict =
                 switch (Js.Json.classify(json)) {
                 | JSONObject(dict) => dict
                 | _ as x => raise(UnexpectedJSONValue(x))
                 };

               let isProject = bool_(Js.Dict.get(dict, "isProject"));

               let isProjectSolved =
                 bool_(Js.Dict.get(dict, "isProjectSolved"));

               let isProjectFetched =
                 bool_(Js.Dict.get(dict, "isProjectFetched"));

               let isProjectReadyForDev =
                 bool_(Js.Dict.get(dict, "isProjectReadyForDev"));

               let rootBuildPath =
                 nullableString(Js.Dict.get(dict, "rootBuildPath"));

               let rootInstallPath =
                 nullableString(Js.Dict.get(dict, "rootInstallPath"));
               let rootPackageConfigPath =
                 nullableString(Js.Dict.get(dict, "rootPackageConfigPath"));

               resolve({
                 isProject,
                 isProjectSolved,
                 isProjectFetched,
                 isProjectReadyForDev,
                 rootBuildPath,
                 rootInstallPath,
                 rootPackageConfigPath,
               });
             }) {
             | UnexpectedJSONValue(_) as x => reject(x)
             | Js.Exn.Error(e) =>
               switch (Js.Exn.message(e)) {
               | Some(message) => reject(JSError({j|Error: $message|j}))
               | None => reject(UnknownError("An unknown error occurred"))
               }
             }
           );
         } else {
           reject(Stderr(statusErrorString));
         }
       )
  );
};

let dropAnEsyJSON = (~compilerVersion, ~folder) => {
  let esyJsonTargetDir = Path.join([|folder, ".vscode", "esy"|]);
  Js.Promise.(
    Fs.mkdir(~p=true, esyJsonTargetDir)
    |> then_(() =>
         Fs.writeFile(
           Filename.concat(esyJsonTargetDir, "esy.json"),
           Printf.sprintf(
             "{\"dependencies\": {\"ocaml\": \"%s\", \"@esy-ocaml/reason\": \"*\", \"@opam/merlin-lsp\": \"ocaml/merlin:merlin-lsp.opam#f030d5da7a\"}}",
             compilerVersion,
           ),
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
            if (Semver.satisfies(bsPlatformVersion, ">=6.0.0")) {
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

let getSubDict = (dict, key) =>
  dict->Js.Dict.get(key)->Belt.Option.flatMap(Js.Json.decodeObject);

let mergeDicts = (dict1, dict2) =>
  Js.Dict.fromArray(Js.Array.concat(Js.Dict.entries(dict1), Js.Dict.entries(dict2)));

let setup = (~manifestPath) => {
  Js.Promise.(
    Js.Json.(
      {
        let folder = Filename.dirname(manifestPath);
        Fs.readFile(manifestPath)
        |> then_(manifest => {
             let manifestJson = parseExn(manifest);
             switch (classify(manifestJson)) {
             | JSONObject(dict) =>
               switch (getSubDict(dict, "dependencies"), getSubDict(dict, "devDependencies")) {
               | (Some(dependenciesJson), None) | (None, Some(dependenciesJson)) =>
                 processDeps(dependenciesJson, folder)
               | (Some(dependenciesJson), Some(devDependenciesJson)) =>
                  processDeps(mergeDicts(dependenciesJson, devDependenciesJson), folder)
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
  );
};

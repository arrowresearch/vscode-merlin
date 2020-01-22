/** Esy API **/
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

open Bindings;

let (<<) = (f, g, x) => f(g(x));

module JSONResponse = {
  module CamlArray = Array;
  open Js;
  /* A module where hand written json parsing logic resides temporarily */
  let getBuildId = responseText =>
    try({
      let json = Json.parseExn(responseText);
      switch (Js.Json.classify(json)) {
      | JSONObject(dict) =>
        switch (Dict.get(dict, "value")) {
        | None => Error("Field 'value' in Azure's response was undefined")
        | Some(json) =>
          switch (Js.Json.classify(json)) {
          | JSONArray(builds) =>
            let o = CamlArray.get(builds, 0);
            switch (Json.classify(o)) {
            | JSONObject(dict) =>
              switch (Dict.get(dict, "id")) {
              | Some(id) =>
                switch (Json.classify(id)) {
                | JSONNumber(n) => Ok(n)
                | _ => Error({j| Field id was expected to be a number |j})
                }
              | None => Error({j| Field id was missing |j})
              }
            | _ =>
              Error(
                {j| First item in the 'value' field array isn't an object as expected |j},
              )
            };

          | _ =>
            Error({j| Response from Azure did not contain build 'value' |j})
          }
        }
      | _ => Error({j| Response from Azure wasn't an object |j})
      };
    }) {
    | _ => Error({j| Failed to parse response from Azure |j})
    };

  let getDownloadURL = responseText =>
    try({
      let json = Json.parseExn(responseText);
      switch (Js.Json.classify(json)) {
      | JSONObject(dict) =>
        switch (Dict.get(dict, "resource")) {
        | None => Error("Field 'value' in Azure's response was undefined")
        | Some(json) =>
          switch (Json.classify(json)) {
          | JSONObject(dict) =>
            switch (Dict.get(dict, "downloadUrl")) {
            | Some(id) =>
              switch (Json.classify(id)) {
              | JSONString(s) => Ok(s)
              | _ =>
                Error({j| Field downloadUrl was expected to be a string |j})
              }
            | None => Error({j| Field downloadUrl was missing |j})
            }
          | _ =>
            Error(
              {j| First item in the 'resource' field array isn't an object as expected |j},
            )
          }
        }
      | _ => Error({j| Response from Azure wasn't an object |j})
      };
    }) {
    | _ => Error({j| Failed to parse response from Azure |j})
    };
};

let restBase = "https://dev.azure.com/arrowresearch/";
let proj = "vscode-merlin";
let os =
  switch (Process.platform) {
  | "darwin" => "Darwin"
  | "linux" => "Linux"
  | "win32" => "Windows"
  };
let artifactName = {j|cache-$os-install|j};
let master = "branchName=refs%2Fheads%2Fmaster";
let filter = "deletedFilter=excludeDeleted&statusFilter=completed&resultFilter=succeeded";
let latest = "queryOrder=finishTimeDescending&$top=1";

let getBuildID = () => {
  Utils.(
    Js.Promise.(
      Https.getCompleteResponse(
        {j|$restBase/$proj/_apis/build/builds?$filter&$master&$latest&api-version=4.1|j},
      )
      |> then_(bindResultAndResolvePromise(JSONResponse.getBuildId))
    )
  );
};

let getDownloadURL = latestBuildID =>
  Utils.(
    Js.Promise.(
      Https.getCompleteResponse(
        {j|$restBase/$proj/_apis/build/builds/$latestBuildID/artifacts?artifactname=$artifactName&api-version=4.1|j},
      )
      |> then_(bindResultAndResolvePromise(JSONResponse.getDownloadURL))
    )
  );

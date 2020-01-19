let propertyExists = (json, property) => {
  Js.Json.(
    switch (classify(json)) {
    | JSONObject(json) =>
      switch (Js.Dict.get(json, property)) {
      | Some(j) =>
        switch (classify(j)) {
        | JSONNull => false
        | _ => true
        }
      | None => false
      }
    | _ => false
    }
  );
};

let mapResultAndResolvePromise = (f, r) =>
  Js.Promise.(Result.(r >| f |> resolve));

let bindResultAndResolvePromise = (f, r) =>
  Js.Promise.(Result.(r >>= f |> resolve));

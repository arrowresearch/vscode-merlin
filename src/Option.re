open Belt.Option;

let (>>=) = flatMap;
let (>>|) = map;

let (|!) = (o, f) =>
  switch (o) {
  | None => f()
  | _ => ()
  };

let toResult = msg =>
  fun
  | Some(x) => Ok(x)
  | None => Error(msg);

let toPromise = msg =>
  fun
  | Some(x) => x
  | None => Js.Promise.resolve(Error(msg));

let (>>=) = (o, f) =>
  switch (o) {
  | Some(x) => f(x)
  | None => None
  };

let (>|) = (o, f) =>
  switch (o) {
  | Some(x) => Some(f(x))
  | None => None
  };

let (|!) = (o, f) =>
  switch (o) {
  | None => f()
  | _ => ()
  };

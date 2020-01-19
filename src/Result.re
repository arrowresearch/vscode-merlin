let map = (e, f) =>
  switch (e) {
  | Ok(x) => Ok(f(x))
  | Error(x) => Error(x)
  };
let (>|) = map;
let bind = (e, f: 'a => result('c, 'b)) =>
  switch (e) {
  | Ok(x) => f(x)
  | Error(x) => Error(x)
  };
let (>>=) = bind;

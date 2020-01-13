let map: (result('a, 'b), 'a => 'c) => result('c, 'b);
let (>|): (result('a, 'b), 'a => 'c) => result('c, 'b);
let bind: (result('a, 'b), 'a => result('c, 'b)) => result('c, 'b);
let (>>=): (result('a, 'b), 'a => result('c, 'b)) => result('c, 'b);

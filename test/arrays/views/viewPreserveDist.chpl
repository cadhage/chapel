use BlockDist;

const D1 = {1..8} dmapped Block({1..8});
const A1: [D1] real;

const D2 = {1..8, 1..8} dmapped Block({1..8, 1..8});
var A2: [D2] real;

writeln("D1._value.whole = ", D1._value.whole);
foo(A1);
//foo(A1[2..8]);
foo(A2[.., 3]);
//foo(A2[7, ..]);
//foo(A1.reindex({0..15 by 2}));

proc foo(X: [?D] real) {
  writeln("in foo");
  forall i in D do
    X[i] = here.id / 10.0;
  writeln("X is: ", X);
  writeln();
  var Y: [D] real;
  forall y in Y do
    Y = here.id / 10.0;
  writeln("Y is: ", Y);
  writeln();
}

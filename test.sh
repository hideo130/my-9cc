#!/bin/bash
assert(){
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    echo $1, $2
    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 "0;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5;"
assert 47 "5+6*7;"
assert 15 "5*(9 - 6);"
assert 7 "1 + 3 + (9 - 6);"
assert 1 "2 + -1;"
assert 1 "2 + (-1);"
assert 1 "-1 + 2;"
# equal, not equal
assert 1 "1 == 1;"
assert 0 "-1 == 1;"
assert 1 "1 < 2;"
assert 1 "2 > 1;"
assert 0 "3 < 2;"
assert 0 "2 > 3;"
assert 1 "1 <= 1;"
assert 1 "1 >= 1;"
assert 0 "1 < 1;"
assert 0 "1 > 1;"

assert 3 'a=3; a;'
assert 8 'a=3; z=5; a+z;'
assert 6 'a=b=3; a+b;'

echo ok

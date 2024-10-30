#!/bin/bash
cat <<EOF | gcc -xc -c -o tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
int identity(int a) { return a; }
int add2(int a, int b) { return a + b; }
int add6(int a, int b, int c, int d, int e, int f) { return a + b + c + d + e + f; }
EOF

assert(){
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s tmp2.o
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
assert 1 "{-1 + 2;}"

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

assert 3 'ab1=3; ab1;'
assert 8 'a1=3; z2=5; a1+z2;'
assert 6 'a1=b2=3; a1+b2;'
assert 3 'a_1=3; a_1;'

# return
assert 1 'return 1;'
assert 1 'a=1; return a;'

# "if" statement
assert 3 'if(0) return 2; 3;'
assert 2 'if(1) return 2; 3;'
assert 3 'a = 3; if(0) return 2; else return a;'
assert 2 'a = 3; if(1) return 2; else return a;'
assert 2 'if(1) if(1) return 2;'
assert 0 'if(1) if(0) return 2; else return 0;'

# "while" statement
assert 2 'a=0; while(a<2) a=a+1; return a;'
assert 4 'a=0; b=0; while(a<2){ a=a+1; b=b+1;} return a+b;'

# "for" statement
assert 2 'a=0; for(i=0;i<2;i=i+1) a=a+1; return a;'
assert 2 'a=0; for(;;) if(a<2) a=a+1; else return a; return a;'

# function call
assert 3 '{ return ret3(); }'
assert 5 '{ return ret5(); }'

assert 1 '{ return identity(1); }'
assert 3 '{ return add2(1, 2); }'
assert 21 '{ return add6(1,2,3,4,5,6); }'
assert 66 '{ return add6(1,2,add6(3,4,5,6,7,8),9,10,11); }'
assert 136 '{ return add6(1,2,add6(3,add6(4,5,6,7,8,9),10,11,12,13),14,15,16); }'

echo ok

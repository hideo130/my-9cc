#!/bin/bash
cat <<EOF | gcc -xc -c -o tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
int identity(int a) { return a; }
int add2(int a, int b) { return a + b; }
int add6(int a, int b, int c, int d, int e, int f) { return a + b + c + d + e + f; }
int rec2(int a){ if(a==0)return 2; return rec2(a-1); }
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

assert 0 "main() { return 0;}"
assert 21 "main() { return 5+20-4; }"
assert 41 "main() { return  12 + 34 - 5; }"
assert 47 "main() { return 5+6*7; }"
assert 15 "main() { return 5*(9 - 6); }"
assert 7 "main() { return 1 + 3 + (9 - 6); }"
assert 1 "main() { return 2 + -1; }"
assert 1 "main() { return 2 + (-1); }"
assert 1 "main() { return -1 + 2; }"

# equal, not equal
assert 1 "main() { return 1 == 1; }"
assert 0 "main() { return -1 == 1; }"
assert 1 "main() { return 1 < 2; }"
assert 1 "main() { return 2 > 1; }"
assert 0 "main() { return 3 < 2; }"
assert 0 "main() { return 2 > 3; }"
assert 1 "main() { return 1 <= 1; }"
assert 1 "main() { return 1 >= 1; }"
assert 0 "main() { return 1 < 1; }"
assert 0 "main() { return 1 > 1; }"

assert 3 'main() { a=3; return a;}'
assert 8 'main() { a=3; z=5; return a+z; }'
assert 15 'main() { a=3; z=5; return a*z; }'
assert 6 'main() { a=b=3; return a+b; }'

assert 3 'main() { ab1=3; return ab1; }'
assert 8 'main() { a1=3; z2=5; return a1+z2; }'
assert 6 'main() { a1=b2=3; return a1+b2; }'
assert 3 'main() { a_1=3; return a_1; }'

# "if" statement
assert 3 'main() { if(0) return 2; return 3; }'
assert 2 'main() { if(1) return 2; return 3; }'
assert 3 'main() { a = 3; if(0) return 2; else return a; }'
assert 2 'main() { a = 3; if(1) return 2; else return a; }'
assert 2 'main() { if(1) if(1) return 2; }'
assert 0 'main() { if(1) if(0) return 2; else return 0; }'

# "while" statement
assert 2 'main() { a=0; while(a<2) a=a+1; return a; }'
assert 4 'main() { a=0; b=0; while(a<2){ a=a+1; b=b+1;} return a+b; }'

# "for" statement
assert 2 'main() { a=0; for(i=0;i<2;i=i+1) a=a+1; return a; }'
assert 2 'main() { a=0; for(;;) if(a<2) a=a+1; else return a; return a; }'

# function call
assert 3 'main() { return ret3(); }'
assert 5 'main() { return ret5(); }'

assert 1 'main() { return identity(1); }'
assert 3 'main() { return add2(1, 2); }'
assert 21 'main() { return add6(1,2,3,4,5,6); }'
assert 66 'main() { return add6(1,2,add6(3,4,5,6,7,8),9,10,11); }'
assert 136 'main() { return add6(1,2,add6(3,add6(4,5,6,7,8,9),10,11,12,13),14,15,16); }'

# functaion definition 
assert 1 'ret1() { return 1; } main() { return ret1(); }'
assert 2 'myident(a) { return a; } main() { return myident(2); }'
assert 6 'mymul(a, b) { return a*b; } main() { return mymul(2, 3); }'
assert 21 'myadd6(a, b, c, d, e, f) { return a + b + c + d + e + f; } main() { return myadd6(1,2,3,4,5,6); }'
assert 66 'myadd6(a, b, c, d, e, f) { return a + b + c + d + e + f; } main() { return myadd6(1,2,myadd6(3,4,5,6,7,8),9,10,11); }'
assert 136 'myadd6(a, b, c, d, e, f) { return a + b + c + d + e + f; } main() { return myadd6(1,2,myadd6(3,myadd6(4,5,6,7,8,9),10,11,12,13),14,15,16); }'


assert 2 'myret2(a) { if(a == 0) return 2;  return myret2(a-1); } main() { return myret2(5); }'
assert 5 'fib(a) { if(a == 0) return 0; if(a == 1) return 1; return fib(a-1) + fib(a-2); } main() { return fib(5); }'
assert 55 'fib(a) { if(a == 0) return 0; if(a == 1) return 1; return fib(a-1) + fib(a-2); } main() { return fib(10); }'



echo ok

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

assert 0 "int main() { return 0;}"
assert 21 "int main() { return 5+20-4; }"
assert 41 "int main() { return  12 + 34 - 5; }"
assert 47 "int main() { return 5+6*7; }"
assert 15 "int main() { return 5*(9 - 6); }"
assert 7 "int main() { return 1 + 3 + (9 - 6); }"
assert 1 "int main() { return 2 + -1; }"
assert 1 "int main() { return 2 + (-1); }"
assert 1 "int main() { return -1 + 2; }"

# equal, not equal
assert 1 "int main() { return 1 == 1; }"
assert 0 "int main() { return -1 == 1; }"
assert 1 "int main() { return 1 < 2; }"
assert 1 "int main() { return 2 > 1; }"
assert 0 "int main() { return 3 < 2; }"
assert 0 "int main() { return 2 > 3; }"
assert 1 "int main() { return 1 <= 1; }"
assert 1 "int main() { return 1 >= 1; }"
assert 0 "int main() { return 1 < 1; }"
assert 0 "int main() { return 1 > 1; }"

assert 3 'int main() {int a=3; return a;}'
assert 8 'int main() {int a; int z; a=3; z=5; return a+z; }'
assert 15 'int main() {int a; int z; a=3; z=5; return a*z; }'
assert 6 'int main() {int a; int b; a=b=3; return a+b; }'
assert 2 "int main() {int a=b=1; return a + b;}"


assert 3 'int main() {int ab1; ab1=3; return ab1; }'
assert 8 'int main() {int a1;int z2; a1=3; z2=5; return a1+z2; }'
assert 6 'int main() {int a1;int b2; a1=b2=3; return a1+b2; }'
assert 3 'int main() {int a_1; a_1=3; return a_1; }'

# "if" statement
assert 3 'int main() { if(0) return 2; return 3; }'
assert 2 'int main() { if(1) return 2; return 3; }'
assert 3 'int main() {int a; a = 3; if(0) return 2; else return a; }'
assert 2 'int main() {int a; a = 3; if(1) return 2; else return a; }'
assert 2 'int main() { if(1) if(1) return 2; }'
assert 0 'int main() { if(1) if(0) return 2; else return 0; }'

# "while" statement
assert 2 'int main() {int a; a=0; while(a<2) a=a+1; return a; }'
assert 4 'int main() {int a;int b; a=0; b=0; while(a<2){ a=a+1; b=b+1;} return a+b; }'

# "for" statement
assert 2 'int main() {int a; a=0; for(int i=0;i<2;i=i+1) a=a+1; return a; }'
assert 2 'int main() {int a; a=0; for(;;) if(a<2) a=a+1; else return a; return a; }'

# function call
assert 3 'int main() { return ret3(); }'
assert 5 'int main() { return ret5(); }'

assert 1 'int main() { return identity(1); }'
assert 3 'int main() { return add2(1, 2); }'
assert 21 'int main() { return add6(1,2,3,4,5,6); }'
assert 66 'int main() { return add6(1,2,add6(3,4,5,6,7,8),9,10,11); }'
assert 136 'int main() { return add6(1,2,add6(3,add6(4,5,6,7,8,9),10,11,12,13),14,15,16); }'

# functaion definition 
assert 1 'int ret1() { return 1; } int main() { return ret1(); }'
assert 2 'int myident(int a) { return a; } int main() { return myident(2); }'
assert 6 'int mymul(int a, int b) { return a*b; } int main() { return mymul(2, 3); }'
assert 21 'int myadd6(int a,int b,int c,int d,int e,int f) { return a + b + c + d + e + f; } int main() { return myadd6(1,2,3,4,5,6); }'
assert 66 'int myadd6(int a,int b,int c,int d,int e,int f) { return a + b + c + d + e + f; } int main() { return myadd6(1,2,myadd6(3,4,5,6,7,8),9,10,11); }'
assert 136 'int myadd6(int a,int b,int c,int d,int e,int f) { return a + b + c + d + e + f; } int main() { return myadd6(1,2,myadd6(3,myadd6(4,5,6,7,8,9),10,11,12,13),14,15,16); }'


assert 2 'int myret2(int a) { if(a == 0) return 2;  return myret2(a-1); } int main() { return myret2(5); }'
assert 5 'int fib(int a) { if(a == 0) return 0; if(a == 1) return 1; return fib(a-1) + fib(a-2); } int main() { return fib(5); }'
assert 55 'int fib(int a) { if(a == 0) return 0; if(a == 1) return 1; return fib(a-1) + fib(a-2); } int main() { return fib(10); }'


# reference and
assert 3 "int main() {int x; int y; x=3;y=&x; return *y;}"
assert 3 "int main() {int x; int y; int z; x=3; y=5; z=&y+8; return *z;}"

echo ok

/*
This file references an extern function c() that lives in
a separate compilation unit that also has a() and b().
*/

extern void c();

void a() {
}

void b() {
}

int main() {
c();
}

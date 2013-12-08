create np 5 2 .

create dn 1 .

b 0 /h0
b 1 /h1

cr bf /h0/a.log

w bf /h0/a.log 1234567890 0 32

r bf /h0/a.log 0 32


w bf /h0/a.log abcdefghijklnmopqrstuvwxyz 0 32

w bf /h0/a.log 1234567890 26 32

r bf /h0/a.log 0 64

w bf /h0/a.log ABCDEF 40 64

r bf /h0/a.log 0 64
r bf /h0/a.log 40 64

w bf /h0/a.log #+++...---# 67108864 64

# hexdump -c xxx/0

w bf /h0/a.log #!@#$%^&*()-+# 67108800 64


w bf /h0/a.log #11223344556677889900# 67108860 64

r bf /h0/a.log 67108860 64

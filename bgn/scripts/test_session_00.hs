session add name session_01 expire 60 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_01 key root_01/level01/level02/level03 val 0303 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_01 key root_01/level01/level02/level03 val 030303 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_01 key root_01/level01/level02/level04 val 0404 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_01 key root_01/level01/level02 val 0202 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_01 key root_01/level01 val 0101 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_01 key root_01 val 0x0x on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console

session add name session_02 expire 60 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_02 key root_01/level01/level02/level03 val 0303 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_02 key root_01/level01/level02/level03 val 030303 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_02 key root_01/level01/level02/level04 val 0404 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_02 key root_01/level01/level02 val 0202 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_02 key root_01/level01 val 0101 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_02 key root_01 val 0x0x on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console

session set name session_02 key root_02/level01/level02/level03 val 0303 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_02 key root_02/level01/level02/level03 val 030303 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_02 key root_02/level01/level02/level04 val 0404 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_02 key root_02/level01/level02 val 0202 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_02 key root_02/level01 val 0101 on tcid 10.10.10.1 rank 0 modi 0 at console
#session show on tcid 10.10.10.1 rank 0 modi 0 at console
session set name session_02 key root_02 val 0x0x on tcid 10.10.10.1 rank 0 modi 0 at console
session show on tcid 10.10.10.1 rank 0 modi 0 at console

session get name session_01 key root/level01 on tcid 10.10.10.1 rank 0 modi 0 at console
session get name session_01 key root.*/level01 on tcid 10.10.10.1 rank 0 modi 0 at console
session get name session_02 key root.*/level01 on tcid 10.10.10.1 rank 0 modi 0 at console
session get name session_02 key root.*/level01 on tcid 10.10.10.1 rank 0 modi 0 at console
session get nameregex session_.* key root.*/level01 on tcid 10.10.10.1 rank 0 modi 0 at console

session get id 1 key root.*/level01 on tcid 10.10.10.1 rank 0 modi 0 at console
session get id 2 key root.*/level01 on tcid 10.10.10.1 rank 0 modi 0 at console

#session rmv nameregex .* on tcid 10.10.10.1 rank 0 modi 0 at console
#session rmv idregex .* on tcid 10.10.10.1 rank 0 modi 0 at console
#session rmv idregex [1-9] on tcid 10.10.10.1 rank 0 modi 0 at console

# node00
# rfs00
hsrfs 0 create np model 9 max num 2 with hash algo 1 and root ./rfs00 on tcid 10.10.10.1 at console

hsrfs 0 create dn with root ./rfs00 on tcid 10.10.10.1 at console
hsrfs 0 add disk 0 on tcid 10.10.10.1 at console
hsrfs 0 add disk 1 on tcid 10.10.10.1 at console

hsrfs 0 write file /h1/a.log with content "helloworld" and expire 0 seconds on tcid 10.10.10.1 at console
hsrfs 0 read file /h1/a.log on tcid 10.10.10.1 at console

hsrfs 0 delete dir /h1 on tcid 10.10.10.1 at console
hsrfs 0 recycle on tcid 10.10.10.1 at console
hsrfs 0 show npp on tcid 10.10.10.1 at console
hsrfs 0 show dn on tcid  10.10.10.1 at console

# rfs01
hsrfs 1 create np model 9 max num 2 with hash algo 1 and root ./rfs01 on tcid 10.10.10.1 at console

hsrfs 1 create dn with root ./rfs01 on tcid 10.10.10.1 at console
hsrfs 1 add disk 0 on tcid 10.10.10.1 at console
hsrfs 1 add disk 1 on tcid 10.10.10.1 at console

hsrfs 1 write file /h1/a.log with content "helloworld" and expire 0 seconds on tcid 10.10.10.1 at console
hsrfs 1 read file /h1/a.log on tcid 10.10.10.1 at console

hsrfs 1 delete dir /h1 on tcid 10.10.10.1 at console
hsrfs 1 recycle on tcid 10.10.10.1 at console
hsrfs 1 show npp on tcid 10.10.10.1 at console
hsrfs 1 show dn on tcid  10.10.10.1 at console

# rfs02
hsrfs 2 create np model 9 max num 2 with hash algo 1 and root ./rfs02 on tcid 10.10.10.1 at console

hsrfs 2 create dn with root ./rfs02 on tcid 10.10.10.1 at console
hsrfs 2 add disk 0 on tcid 10.10.10.1 at console
hsrfs 2 add disk 1 on tcid 10.10.10.1 at console

hsrfs 2 write file /h1/a.log with content "helloworld" and expire 0 seconds on tcid 10.10.10.1 at console
hsrfs 2 read file /h1/a.log on tcid 10.10.10.1 at console

hsrfs 2 delete dir /h1 on tcid 10.10.10.1 at console
hsrfs 2 recycle on tcid 10.10.10.1 at console
hsrfs 2 show npp on tcid 10.10.10.1 at console
hsrfs 2 show dn on tcid  10.10.10.1 at console

# rfs03
hsrfs 3 create np model 9 max num 2 with hash algo 1 and root ./rfs03 on tcid 10.10.10.1 at console

hsrfs 3 create dn with root ./rfs03 on tcid 10.10.10.1 at console
hsrfs 3 add disk 0 on tcid 10.10.10.1 at console
hsrfs 3 add disk 1 on tcid 10.10.10.1 at console

hsrfs 3 write file /h1/a.log with content "helloworld" and expire 0 seconds on tcid 10.10.10.1 at console
hsrfs 3 read file /h1/a.log on tcid 10.10.10.1 at console

hsrfs 3 delete dir /h1 on tcid 10.10.10.1 at console
hsrfs 3 recycle on tcid 10.10.10.1 at console
hsrfs 3 show npp on tcid 10.10.10.1 at console
hsrfs 3 show dn on tcid  10.10.10.1 at console

# rfs04
hsrfs 4 create np model 9 max num 2 with hash algo 1 and root ./rfs04 on tcid 10.10.10.1 at console

hsrfs 4 create dn with root ./rfs04 on tcid 10.10.10.1 at console
hsrfs 4 add disk 0 on tcid 10.10.10.1 at console
hsrfs 4 add disk 1 on tcid 10.10.10.1 at console

hsrfs 4 write file /h1/a.log with content "helloworld" and expire 0 seconds on tcid 10.10.10.1 at console
hsrfs 4 read file /h1/a.log on tcid 10.10.10.1 at console

hsrfs 4 delete dir /h1 on tcid 10.10.10.1 at console
hsrfs 4 recycle on tcid 10.10.10.1 at console
hsrfs 4 show npp on tcid 10.10.10.1 at console
hsrfs 4 show dn on tcid  10.10.10.1 at console

# rfs05
hsrfs 5 create np model 9 max num 2 with hash algo 1 and root ./rfs05 on tcid 10.10.10.1 at console

hsrfs 5 create dn with root ./rfs05 on tcid 10.10.10.1 at console
hsrfs 5 add disk 0 on tcid 10.10.10.1 at console
hsrfs 5 add disk 1 on tcid 10.10.10.1 at console

hsrfs 5 write file /h1/a.log with content "helloworld" and expire 0 seconds on tcid 10.10.10.1 at console
hsrfs 5 read file /h1/a.log on tcid 10.10.10.1 at console

hsrfs 5 delete dir /h1 on tcid 10.10.10.1 at console
hsrfs 5 recycle on tcid 10.10.10.1 at console
hsrfs 5 show npp on tcid 10.10.10.1 at console
hsrfs 5 show dn on tcid  10.10.10.1 at console


hsrfs 0 create np model 4 max num 2 with hash algo 1 and root ./rfs00 on tcid 10.10.10.1 at console

hsrfs 0 create dn with root ./rfs00 on tcid 10.10.10.1 at console
hsrfs 0 add disk 0 on tcid 10.10.10.1 at console
hsrfs 0 add disk 1 on tcid 10.10.10.1 at console

hsrfs 0 write file /h1/a.log with content "helloworld" and expire 0 seconds on tcid 10.10.10.1 at console
hsrfs 0 read file /h1/a.log on tcid 10.10.10.1 at console

hsrfs 0 delete dir /h1 on tcid 10.10.10.1 at console
hsrfs 0 recycle on tcid 10.10.10.1 at console
hsrfs 0 show npp on tcid 10.10.10.1 at console
hsrfs 0 show dn on tcid  10.10.10.1 at console

hsrfs 1 create np model 4 max num 2 with hash algo 1 and root ./rfs01 on tcid 10.10.10.1 at console

hsrfs 1 create dn with root ./rfs01 on tcid 10.10.10.1 at console
hsrfs 1 add disk 0 on tcid 10.10.10.1 at console
hsrfs 1 add disk 1 on tcid 10.10.10.1 at console

hsrfs 1 write file /h1/a.log with content "helloworld" and expire 0 seconds on tcid 10.10.10.1 at console
hsrfs 1 read file /h1/a.log on tcid 10.10.10.1 at console

hsrfs 1 delete dir /h1 on tcid 10.10.10.1 at console
hsrfs 1 recycle on tcid 10.10.10.1 at console
hsrfs 1 show npp on tcid 10.10.10.1 at console
hsrfs 1 show dn on tcid  10.10.10.1 at console


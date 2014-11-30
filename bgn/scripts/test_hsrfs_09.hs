hsrfs create np model 9 max num 2 with hash algo 1 and root . on tcid 10.10.10.1 at console

hsrfs create dn with root . on tcid 10.10.10.1 at console
hsrfs add disk 0 on tcid 10.10.10.1 at console
hsrfs add disk 1 on tcid 10.10.10.1 at console
hsrfs add disk 2 on tcid 10.10.10.1 at console
hsrfs add disk 3 on tcid 10.10.10.1 at console
hsrfs add disk 4 on tcid 10.10.10.1 at console
hsrfs add disk 5 on tcid 10.10.10.1 at console
hsrfs add disk 6 on tcid 10.10.10.1 at console
hsrfs add disk 7 on tcid 10.10.10.1 at console
hsrfs add disk 8 on tcid 10.10.10.1 at console
hsrfs add disk 9 on tcid 10.10.10.1 at console

hsrfs write file /h1/a.log with content "helloworld" on tcid 10.10.10.1 at console

hsrfs read file /h1/a.log on tcid 10.10.10.1 at console

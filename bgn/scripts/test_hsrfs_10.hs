hsrfs create np model 6 max num 2 with hash algo 1 and root . on tcid 10.10.10.1 at console

hsrfs create dn with root . on tcid 10.10.10.1 at console
hsrfs add disk 0 on tcid 10.10.10.1 at console
hsrfs add disk 1 on tcid 10.10.10.1 at console

hsrfs write file /h1/a.log with content "helloworld" and expire 0 seconds on tcid 10.10.10.1 at console

hsrfs read file /h1/a.log on tcid 10.10.10.1 at console

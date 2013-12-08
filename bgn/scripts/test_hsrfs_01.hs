hsrfs create np model 4 max num 2 with 1st hash algo 1 2nd hash algo 2 and root . on tcid 10.10.10.1 at console

hsrfs create dn 1 GB and root . on tcid 10.10.10.1 at console

hsrfs bind np 0 with home /h1 on tcid 10.10.10.1 at console

hsrfs bind np 1 with home /h2 on tcid 10.10.10.1 at console

hsrfs write file /h1/a.log with content "helloworld" on tcid 10.10.10.1 at console

hsrfs read file /h1/a.log on tcid 10.10.10.1 at console

hshfs create np model 4 max num 2 with 1st hash algo 1 2nd hash algo 2 and bucket 8192 and root . on tcid 10.10.10.1 at console

hshfs create dn with root . on tcid 10.10.10.1 at console
hshfs add disk 0 on tcid 10.10.10.1 at console
hshfs add disk 1 on tcid 10.10.10.1 at console

hshfs bind np 0 with home /h1 on tcid 10.10.10.1 at console

hshfs bind np 1 with home /h2 on tcid 10.10.10.1 at console

hshfs write file /h1/a.log with content "helloworld" on tcid 10.10.10.1 at console

hshfs read file /h1/a.log on tcid 10.10.10.1 at console

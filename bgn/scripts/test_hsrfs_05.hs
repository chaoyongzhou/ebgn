#hsrfs create np model 5 max num 2 with 1st hash algo 1 2nd hash algo 2 and root . on tcid 10.10.10.1 at console
#hsrfs create dn 1 GB and root . on tcid 10.10.10.1 at console
#hsrfs bind np 0 with home /h0 on tcid 10.10.10.1 at console
#hsrfs bind np 1 with home /h1 on tcid 10.10.10.1 at console
hsrfs create bigfile /h1/c.log on tcid 10.10.10.1 at console

hsrfs write bigfile /h1/c.log with content 1234567890 at offset 0 max 32 on tcid 10.10.10.1 at console
hsrfs read bigfile /h1/c.log from offset 0 max 32 on tcid 10.10.10.1 at console
hsrfs write bigfile /h1/c.log with content abcdefghijklnmopqrstuvwxyz at offset 0 max 32 on tcid 10.10.10.1 at console
hsrfs write bigfile /h1/c.log with content 1234567890 at offset 26 max 32 on tcid 10.10.10.1 at console
hsrfs read bigfile /h1/c.log from offset 0 max 64 on tcid 10.10.10.1 at console
hsrfs write bigfile /h1/c.log with content ABCDEF at offset 40 max 64 on tcid 10.10.10.1 at console
hsrfs read bigfile /h1/c.log from offset 0 max 64 on tcid 10.10.10.1 at console
hsrfs read bigfile /h1/c.log from offset 40 max 64 on tcid 10.10.10.1 at console
hsrfs write bigfile /h1/c.log with content #+++...---# at offset 67108864 max 64 on tcid 10.10.10.1 at console
hsrfs write bigfile /h1/c.log with content #!@#$%^&*()-+# at offset 67108800 max 64 on tcid 10.10.10.1 at console
hsrfs write bigfile /h1/c.log with content #11223344556677889900# at offset 67108860 max 64 on tcid 10.10.10.1 at console
hsrfs read bigfile /h1/c.log from offset 67108860 max 64 on tcid 10.10.10.1 at console


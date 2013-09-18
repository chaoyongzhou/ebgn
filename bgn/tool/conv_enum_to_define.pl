#! /usr/bin/perl -w

use strict;

&conv_enum_to_define(@ARGV);

sub conv_enum_to_define
{
	my $fname = shift;
	my $mname = shift;

	my $line;
	my $num;

	open(FP, "< $fname") || die("cannot open file $fname\n");

	$num = 0;
	while(<FP>)
	{
		chomp($line = $_);	
		next if($line =~ /^\s+$/);
		$line =~ s/\s+//g;

		if($line =~ /(\S+)/)
		{
			printf STDOUT ("#define %41s  ((UINT32) ((%s << (WORDSIZE/2)) + %2d))\n", $1, $mname, $num);
			$num ++;
		}

	}

	close(FP);
}



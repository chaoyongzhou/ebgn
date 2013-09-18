#! /usr/bin/perl -w


my @src_file_list;
my $src_file;


@src_file_list = `ls ../src/*.c`;
#@src_file_list = `ls *.c`;
chomp(@src_file_list);

foreach $src_file ( @src_file_list )
{
    #print "source file: $src_file\n";
    #&do_one_src_file($src_file,"tmp.012.log");
} 


###############################################################################################################################
# do_one_src_file($src_file)
###############################################################################################################################
sub do_one_src_file
{
    my $src_file_name;
    my $tmp_file_name;

    my $def_func_name = "[a-zA-Z_][a-zA-Z0-9_]*";
    #my $def_func_ret  = "[a-zA-Z_][a-zA-Z0-9_]*\s*[\*]*";
    my $def_func_ret  = "[a-zA-Z_][a-zA-Z0-9_]*";

    my $file_context;
    my $cur_line;

    ($src_file_name, $tmp_file_name) = @_;


    $file_context = "";

    #if( $file_context =~ /($def_func_ret)\s+($def_func_name)\s*\((.*)\)/ )
    #{
    #    print ">   $1\n";
    #    print ">>  $2\n";
    #    print ">>> $3\n";
    #    print "$1 $2 $3\n";
    #}

    open(SRC_FILE, "< $src_file_name") || die("do_one_src_file: cannot open src file $src_file_name\n");

    while( <SRC_FILE> )
    {
        #if( $_ =~ /^($def_func_ret)\s+($def_func_name)\s*\((.*)\)\s*$/s )
        #{
        #    print $_;
         #    next;
        #}

        if( $_ =~ /\{.*\}/ )
        {
            next;
        }
        if( $_ =~ /^\{/ )
        {
            &skip_big_brace(\*SRC_FILE);
            $file_context .= "\n";
            next;
        } 
        if( $_ =~ /^#/ )
        {
            next;
        }
        if( $_ =~ /extern/ )
        {
            next;
        }
        if( $_ =~ /^typedef/ )
        {
            next;
        }

        if( $_ =~ /^static/ )
        {
            next;
        }    

        $cur_line = $_;
        chomp($cur_line);
        $cur_line =~ s/\s+/ /g;
        if( $cur_line eq "" )
        {
            next;
        }

        $file_context .= $cur_line;
        $file_context =~ s/\/\*.*\*\///;
    }

    close(SRC_FILE);

    open(TMP_FILE, "> $tmp_file_name") || die("do_one_src_file: cannot write to $tmp_file_name\n");
    print TMP_FILE "$file_context\n";
    close(TMP_FILE);

    open(TMP_FILE, "< $tmp_file_name") || die("do_one_src_file: cannot write to $tmp_file_name\n");
    while( <TMP_FILE> )
    {
        if( $_ =~ /^($def_func_ret)\s+($def_func_name)\s*\((.*)\)/ )
        {
            &do_one_func_proto($1, $2, $3);
        }
    }
    close(TMP_FILE);
}

###############################################################################################################################
# do_one_func_proto($func_ret, $func_name, $func_para_list)
###############################################################################################################################
sub do_one_func_proto
{
    my $func_ret;
    my $func_name;
    my $func_para_list;

    my @func_paras;
    my $func_para;

    my %func_proto;
    my $index;

    my $func_para_direction;
    my $func_para_type;

    ($func_ret, $func_name, $func_para_list) = @_;

    # when func ptr in par list, give up handling
    if( $func_para_list =~ /\(/ )
    {
        #print "$func_ret, $func_name, $func_para_list\n";
        return ;
    }    

    @func_paras = split(',', $func_para_list);
    
    #if( 0 == scalar( @func_paras ) )
    #{
    #    print "$func_ret, $func_name, $func_para_list\n";
    #}

    print STDOUT "$func_ret $func_name($func_para_list)\n";

    $func_proto{"func_ret"} = $func_ret;
    $func_proto{"func_name"} = $func_name;
    $func_proto{"func_para_num"} = scalar( @func_paras );

    for( $index = 0; $index < scalar( @func_paras ); $index ++ )
    {
        ($func_para_direction,$func_para_type) = &do_one_func_para($func_paras[ $index ]);
        $func_proto{"func_para"}[ $index ]{"direction"} = $func_para_direction;
        $func_proto{"func_para"}[ $index ]{"type"} = $func_para_type;
    }

    &print_one_func_proto(\%func_proto);
}

###############################################################################################################################
# print_one_func_proto(%func_proto)
###############################################################################################################################
sub print_one_func_proto
{
    my $func_proto;

    my $func_ret;
    my $func_name;
    my $func_para_num;
    my $func_para_direction;
    my $func_para_type;
    my $index;

    my $str;

    ($func_proto) = @_;

    $func_ret = $$func_proto{"func_ret"};
    $func_name = $$func_proto{"func_name"};
    $func_para_num = $$func_proto{"func_para_num"};

    $str = "$func_ret $func_name (";

    for($index = 0; $index < $func_para_num; $index ++ )
    {
        $func_para_direction = $$func_proto{"func_para"}[ $index ]{"direction"};
        $func_para_type = $$func_proto{"func_para"}[ $index ]{"type"};

        $str .= "$func_para_direction $func_para_type,";
    }
    $str .= ")";
    
    print "$str\n";
}

###############################################################################################################################
# do_one_func_para($func_para)
###############################################################################################################################
sub do_one_func_para
{
    my $func_para;

    my $def_var_type = "[a-zA-Z_][a-zA-Z0-9_]*";
    my $def_var_name = "[a-zA-Z_][a-zA-Z0-9_]*";

    my $var_type;
    my $var_name;
    my $var_ptr;

    my $func_para_tmp;

    ($func_para) = @_;

    $func_para_tmp = $func_para;
    $func_para_tmp =~ s/[^\*]//g;
    $var_ptr = $func_para_tmp;
    $var_ptr =~ s/\*/_ptr/g;

    $func_para_tmp = $func_para;
    $func_para_tmp =~ s/\*/ /g;
    

    if( $func_para_tmp =~ /const\s+($def_var_type)\s*$def_var_name/ )
    {
        $var_type = "e_dbg"."$1"."$var_ptr";
        return ("E_DIRECT_IN", $var_type);
    }

    if( $func_para_tmp =~ /($def_var_type)\s*$def_var_name/ )
    {
        $var_type = "e_dbg"."$1"."$var_ptr";
        return ("E_DIRECT_OUT", $var_type);
    }
}

###############################################################################################################################
# skip_big_brace($src_file)
###############################################################################################################################
sub skip_big_brace
{
    my $src_file;

    ($src_file) = @_;

    while( <$src_file> )
    {
        if( $_ =~ /\{.*\}/ )
        {
            next;
        }
        if( $_ =~ /^\{/ )
        {
            &skip_big_brace($src_file);
            next;
        }
        if( $_ =~ /^\}/ ) 
        {
            last;
        }
    }
}

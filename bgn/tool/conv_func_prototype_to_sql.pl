#! /usr/bin/perl -w

use strict;

my $g_dbg_type;
my $g_md_prio;
my $g_para_dir;
my $g_func_index_enum;

my $usage;

$usage = "$0 <func proto type file>";

$g_dbg_type = {};
$g_md_prio  = {};
$g_para_dir = {};

&main(shift);

#######################################################################################################################
# main()
#######################################################################################################################
sub main
{
    my $func_proto_type_file = shift;
#    $func_proto_type = "void print_ebgn_z_status();";
#    $func_proto_type = "UINT32 ebgn_z_alloc_bgn(const EBGNZ_MD_ID ebgnz_md_id, BIGINT **ppbgn);";

#    &parse_one_func_proto_type($func_proto_type);

    &config_dbg_type($g_dbg_type);
    &config_md_prio($g_md_prio);
    &config_para_dir($g_para_dir);

    #&parse_one_file("ebgnz_func_list.txt");
    #&parse_one_file("func_proto_type_list.txt");
    $g_func_index_enum = 0;
    &parse_one_file($func_proto_type_file);

    #printf STDOUT ("e_dbg_POLY_ptr = %d\n",  $$g_dbg_type{"e_dbg_POLY_ptr"});
}

#######################################################################################################################
# config_dbg_type(%dbg_type)
#######################################################################################################################
sub config_dbg_type 
{
    my $dbg_type;
    my $enum_num;

    ($dbg_type) = @_;

    $enum_num = 0;

    $$dbg_type{"e_dbg_UINT32"}                          = $enum_num ++;
    $$dbg_type{"e_dbg_UINT16"}                          = $enum_num ++;
    $$dbg_type{"e_dbg_UINT8"}                           = $enum_num ++;
    $$dbg_type{"e_dbg_int"}                             = $enum_num ++;
    $$dbg_type{"e_dbg_void"}                            = $enum_num ++;
    $$dbg_type{"e_dbg_BGNZ_MD_ID"}                      = $enum_num ++;
    $$dbg_type{"e_dbg_EBGNZ_MD_ID"}                     = $enum_num ++;
    $$dbg_type{"e_dbg_BGNZ2_MD_ID"}                     = $enum_num ++;
    $$dbg_type{"e_dbg_BGNF2N_MD_ID"}                    = $enum_num ++;
    $$dbg_type{"e_dbg_BGNZN_MD_ID"}                     = $enum_num ++;
    $$dbg_type{"e_dbg_BGNFP_MD_ID"}                     = $enum_num ++;
    $$dbg_type{"e_dbg_ECF2N_MD_ID"}                     = $enum_num ++;
    $$dbg_type{"e_dbg_ECCF2N_MD_ID"}                    = $enum_num ++;
    $$dbg_type{"e_dbg_ECFP_MD_ID"}                      = $enum_num ++;
    $$dbg_type{"e_dbg_ECCFP_MD_ID"}                     = $enum_num ++;
    $$dbg_type{"e_dbg_CONV_MD_ID"}                      = $enum_num ++;
    $$dbg_type{"e_dbg_POLYZ_MD_ID"}                     = $enum_num ++;
    $$dbg_type{"e_dbg_POLYZN_MD_ID"}                    = $enum_num ++;
    $$dbg_type{"e_dbg_POLYZ2_MD_ID"}                    = $enum_num ++;
    $$dbg_type{"e_dbg_POLYFP_MD_ID"}                    = $enum_num ++;
    $$dbg_type{"e_dbg_POLYF2N_MD_ID"}                   = $enum_num ++;
    $$dbg_type{"e_dbg_SEAFP_MD_ID"}                     = $enum_num ++;
    $$dbg_type{"e_dbg_TASK_MD_ID"}                      = $enum_num ++;
    $$dbg_type{"e_dbg_FUNC_RAND_GEN"}                   = $enum_num ++;
    $$dbg_type{"e_dbg_FUNC_HASH"}                       = $enum_num ++;
    $$dbg_type{"e_dbg_EC_BOOL"}                         = $enum_num ++;
    $$dbg_type{"e_dbg_UINT32_ptr"}                      = $enum_num ++;
    $$dbg_type{"e_dbg_UINT16_ptr"}                      = $enum_num ++;
    $$dbg_type{"e_dbg_UINT8_ptr"}                       = $enum_num ++;
    $$dbg_type{"e_dbg_int_ptr"}                         = $enum_num ++;
    $$dbg_type{"e_dbg_void_ptr"}                        = $enum_num ++;
    $$dbg_type{"e_dbg_BIGINT_ptr"}                      = $enum_num ++;
    $$dbg_type{"e_dbg_EBGN_ptr"}                        = $enum_num ++;
    $$dbg_type{"e_dbg_EBGN_ITEM_ptr"}                   = $enum_num ++;
    $$dbg_type{"e_dbg_EC_CURVE_POINT_ptr"}              = $enum_num ++;
    $$dbg_type{"e_dbg_EC_CURVE_AFF_POINT_ptr"}          = $enum_num ++;
    $$dbg_type{"e_dbg_ECF2N_CURVE_ptr"}                 = $enum_num ++;
    $$dbg_type{"e_dbg_ECFP_CURVE_ptr"}                  = $enum_num ++;
    $$dbg_type{"e_dbg_ECC_KEYPAIR_ptr"}                 = $enum_num ++;
    $$dbg_type{"e_dbg_ECC_SIGNATURE_ptr"}               = $enum_num ++;
    $$dbg_type{"e_dbg_POLY_ptr"}                        = $enum_num ++;
    $$dbg_type{"e_dbg_POLY_ITEM_ptr"}                   = $enum_num ++;
    $$dbg_type{"e_dbg_DEGREE_ptr"}                      = $enum_num ++;
    $$dbg_type{"e_dbg_FUNC_RAND_GEN_ptr"}               = $enum_num ++;
    $$dbg_type{"e_dbg_FUNC_HASH_ptr"}                   = $enum_num ++;
    $$dbg_type{"e_dbg_MOD_MGR_ptr"}                     = $enum_num ++;
    $$dbg_type{"e_dbg_DMATRIXR_ptr"}                    = $enum_num ++;
    $$dbg_type{"e_dbg_VMATRIXR_ptr"}                    = $enum_num ++;
    $$dbg_type{"e_dbg_CSTRING_ptr"}                     = $enum_num ++;
    $$dbg_type{"e_dbg_TASKC_MGR_ptr"}                   = $enum_num ++;
    $$dbg_type{"e_dbg_LOG_ptr"}                         = $enum_num ++;
    $$dbg_type{"e_dbg_CFILE_SEG_ptr"}                   = $enum_num ++;
    $$dbg_type{"e_dbg_CFILE_SEG_VEC_ptr"}               = $enum_num ++;
    $$dbg_type{"e_dbg_CFILE_NODE_ptr"}                  = $enum_num ++;
    $$dbg_type{"e_dbg_KBUFF_ptr"}                       = $enum_num ++;
    $$dbg_type{"e_dbg_CDIR_SEG_ptr"}                    = $enum_num ++;
    $$dbg_type{"e_dbg_CDIR_NODE_ptr"}                   = $enum_num ++;
    $$dbg_type{"e_dbg_CMON_OBJ_ptr"}                    = $enum_num ++;
    $$dbg_type{"e_dbg_CMON_OBJ_VEC_ptr"}                = $enum_num ++;
    $$dbg_type{"e_dbg_CSOCKET_CNODE_ptr"}               = $enum_num ++;
    $$dbg_type{"e_dbg_TASKC_NODE_ptr"}                  = $enum_num ++;
    $$dbg_type{"e_dbg_CSYS_CPU_STAT_ptr"}               = $enum_num ++;
    $$dbg_type{"e_dbg_MM_MAN_OCCUPY_NODE_ptr"}          = $enum_num ++;
    $$dbg_type{"e_dbg_MM_MAN_LOAD_NODE_ptr"}            = $enum_num ++;
    $$dbg_type{"e_dbg_MM_MOD_NODE_ptr"}                 = $enum_num ++;
    $$dbg_type{"e_dbg_CPROC_MODULE_STAT_ptr"}           = $enum_num ++;
    $$dbg_type{"e_dbg_CRANK_THREAD_STAT_ptr"}           = $enum_num ++;
    $$dbg_type{"e_dbg_CSYS_ETH_STAT_ptr"}               = $enum_num ++;
    $$dbg_type{"e_dbg_CSYS_DSK_STAT_ptr"}               = $enum_num ++;
    $$dbg_type{"e_dbg_TASK_REPORT_NODE_ptr"}            = $enum_num ++;
    $$dbg_type{"e_dbg_type_end"}                        = $enum_num ++;
}

#######################################################################################################################
# config_md_prio(%md_prio)
#######################################################################################################################
sub config_md_prio
{
    my $md_prio;

    ($md_prio) = @_;

    $$md_prio{"PRIO_BEG"}          = 0x00000000;    
    $$md_prio{"PRIO_MD_BGNZ"}      = 0x00000001;
    $$md_prio{"PRIO_MD_EBGNZ"}     = 0x00000002;
    $$md_prio{"PRIO_MD_BGNZ2"}     = 0x00000003;
    $$md_prio{"PRIO_MD_CONV"}      = 0x00000004;
    $$md_prio{"PRIO_MD_BGNZN"}     = 0x00000010;
    $$md_prio{"PRIO_MD_BGNF2N"}    = 0x00000020;
    $$md_prio{"PRIO_MD_BGNFP"}     = 0x00000100;
    $$md_prio{"PRIO_MD_ECF2N"}     = 0x00001000;
    $$md_prio{"PRIO_MD_ECFP"}      = 0x00002000;
    $$md_prio{"PRIO_MD_ECCF2N"}    = 0x00010000;
    $$md_prio{"PRIO_MD_ECCFP"}     = 0x00020000;
    $$md_prio{"PRIO_MD_POLYZ"}     = 0x00100000;
    $$md_prio{"PRIO_MD_POLYZN"}    = 0x00200000;
    $$md_prio{"PRIO_MD_POLYZ2"}    = 0x00400000;
    $$md_prio{"PRIO_MD_POLYFP"}    = 0x01000000;
    $$md_prio{"PRIO_MD_POLYF2N"}   = 0x02000000;
    $$md_prio{"PRIO_MD_SEAFP"}     = 0x04000000;
    $$md_prio{"PRIO_MD_SUPER"}     = 0x08000000;
    $$md_prio{"PRIO_MD_VMM"}       = 0x10000000;
    $$md_prio{"PRIO_MD_MATRIXR"}   = 0x20000000;
    $$md_prio{"PRIO_MD_DMATRIXR"}  = 0x40000000;
    $$md_prio{"PRIO_MD_VMATRIXR"}  = 0x80000000;
    $$md_prio{"PRIO_MD_CFILE"}     = 0x90000000;
    $$md_prio{"PRIO_MD_CDIR"}      = 0x90000001;
    $$md_prio{"PRIO_MD_CMON"}      = 0x90000002;
    $$md_prio{"PRIO_MD_TBD"}       = 0x90000003;
    $$md_prio{"PRIO_MD_CRUN"}      = 0x90000004;
    $$md_prio{"PRIO_END"}          = 0xFFFFFFFF;
}

#######################################################################################################################
# config_para_dir(%para_dir)
#######################################################################################################################
sub config_para_dir
{
    my $para_dir;

    ($para_dir) = @_;

    $$para_dir{"E_DIRECT_IN"}  = 0x01;
    $$para_dir{"E_DIRECT_OUT"} = 0x02;
    $$para_dir{"E_DIRECT_IO"}  = 0x03;
    $$para_dir{"E_DIRECT_END"} = 0x04;
}

#######################################################################################################################
# parse_one_file($file_name)
#######################################################################################################################
sub parse_one_file 
{
    my $file_name;

    ($file_name) = @_;

    open(FUNC_LIST_FILE, "< $file_name") || die("cannot open $file_name");

    while( <FUNC_LIST_FILE> )
    {
        if( $_ =~ /^\s+$/ )
        {
            next;
        }
        &parse_one_func_proto_type($_);
    }

    close(FUNC_LIST_FILE);
}

#######################################################################################################################
# parse_one_func_proto_type($func_proto_type)
#######################################################################################################################
sub parse_one_func_proto_type 
{
    my $func_proto_type;

    my $func_node;
    my $ret;

    ($func_proto_type) = @_;

    $func_proto_type =~ s/^\s+//g;
    $func_proto_type =~ s/\s+$//g;

    # not support static function
    if( $func_proto_type =~ /^static\s+/ )
    {
        return "";
    }

    # not support function with undetermined paras 
    if( $func_proto_type =~ /\.\.\./ )
    {
        return "";
    }

    $func_node = {};    

    if( $func_proto_type =~ /^(\w+)\s+(\w+)\s*\((.*)\)/ )
    {
        $$func_node{"proto_type"} = $func_proto_type;

        $$func_node{"ret_type"} = "e_dbg_$1";
        $$func_node{"name"} = $2;
        
        $$func_node{"priority"} = &parse_func_priority($2); 

        $$func_node{"paras"} = [];
        $ret = &parse_func_paras($3, $$func_node{"paras"});
        if( 0 != $ret && 1 != $ret )
        {
            printf STDERR ("error: not supported func proto type: %s\n", $func_proto_type);
            exit;
        }

        #&print_func_index_enum(\*STDOUT, $func_node);    
        &print_func_node(\*STDOUT, $func_node);    
        #&print_func_node_to_sql(\*STDOUT, $func_node);    
        return ;
    }

    printf STDERR ("error: invalid func proto type format: %s\n", $func_proto_type);
    exit;
}

#######################################################################################################################
# parse_func_priority($func_name)
#######################################################################################################################
sub parse_func_priority 
{
    my $func_name;

    my $func_prio;
    my $module;

    ($func_name) = @_;

    $func_prio = "PRIO_END";
    if( $func_name =~ /^print_([a-zA-Z0-9]+_[a-zA-Z0-9]+)_status$/
     || $func_name =~ /^print_conv_status$/ )
    {
        $func_prio = "PRIO_BEG";
        return $func_prio;
    }

    if( $func_name =~ /^conv_.*/ )
    {
        $func_prio = "PRIO_MD_CONV";
        return $func_prio;
    }
    if( $func_name =~ /^([a-zA-Z0-9]+_[a-zA-Z0-9]+).*/ )
    {
        $module = $1;
        $module =~ s/_//g;
        $module = uc($module);

        $func_prio = "PRIO_MD_$module";
        return $func_prio;
    }

    return $func_prio;
}

#######################################################################################################################
# parse_func_paras($func_paras_line, @func_paras_tbl)
#######################################################################################################################
sub parse_func_paras 
{
    my $func_paras_line;
    my $func_paras_tbl; 
    
    my @func_paras;
    my $func_paras_num;
    my $index;

    my $pointer_level;

    ($func_paras_line, $func_paras_tbl) = @_;

    if( $func_paras_line =~ /^\s+$/ )
    {
        return 0;
    }

    @func_paras = split(',', $func_paras_line);    
    $func_paras_num = scalar(@func_paras);
    #printf STDOUT ("func paras num: %d\n", $func_paras_num);

    for( $index = 0; $index < $func_paras_num; $index ++ )
    {
        $$func_paras_tbl[ $index ] = {};
        $pointer_level = &parse_one_func_para($func_paras[ $index ], $$func_paras_tbl[ $index ]);
        if( 0 != $pointer_level && 1 != $pointer_level )
        {
            return $pointer_level;
        }
    }
    return 0; # OK
}

#######################################################################################################################
# parse_one_func_para($func_para_line, %func_para_tbl)
#######################################################################################################################
sub parse_one_func_para 
{
    my $func_para_line;
    my $func_para_tbl; 
    
    my $pointers;

    ($func_para_line, $func_para_tbl) = @_;

    $func_para_line =~ s/^\s+//g;
    $func_para_line =~ s/\s+$//g;
    
    # para IN
    if( $func_para_line =~ /^const/ )
    {
        $$func_para_tbl{"direction"} = "E_DIRECT_IN";
        # pointer type
        if( $func_para_line =~ /^const\s+([a-zA-Z0-9_]+)\s*([\*]+)\s*(\w+)/ )
        {
            $$func_para_tbl{"type"} = "e_dbg_$1_ptr";
            $pointers = $2;            

            $$func_para_tbl{"type"} =~ s/\s+//g;
            $pointers =~ s/\s+//g;

            $$func_para_tbl{"pointers"} = length($pointers);
            
            return $$func_para_tbl{"pointers"};
        }

        # non-pointer type
        if( $func_para_line =~ /^const\s+(\w+)\s+(\w+)/ )
        {
            $$func_para_tbl{"type"} = "e_dbg_$1";
            $$func_para_tbl{"pointers"} = 0; 

            $$func_para_tbl{"type"} =~ s/\s+//g;

            return $$func_para_tbl{"pointers"};
        }
    }    
    # para OUT 
    else
    {
        $$func_para_tbl{"direction"} = "E_DIRECT_OUT";
        # pointer type
        if( $func_para_line =~ /([a-zA-Z0-9_]+)([\*\s]+)(\w+)/ )
        {
            $$func_para_tbl{"type"} = "e_dbg_$1_ptr";
            $pointers = $2;            

            $$func_para_tbl{"type"} =~ s/\s+//g;
            $pointers =~ s/\s+//g;

            $$func_para_tbl{"pointers"} = length($pointers);
            
            return $$func_para_tbl{"pointers"};
        }

        # non-pointer type
        if( $func_para_line =~ /(\w+)\s+(\w+)/ )
        {
            $$func_para_tbl{"type"} = "e_dbg_$1";
            $$func_para_tbl{"pointers"} = 0; 

            $$func_para_tbl{"type"} =~ s/\s+//g;

            return $$func_para_tbl{"pointers"};
        }
    }

    return -1;
}

#######################################################################################################################
# print_func_node_to_sql($fp, %func_node)
#######################################################################################################################
sub print_func_node_to_sql 
{
    my $fp;
    my $func_node;

    my $func_paras_ref;
    my $func_paras_num;
    my $func_paras_max_num;
    my $index;

    my $prefix;
    my $value;

    ($fp, $func_node) = @_;

    $func_paras_max_num = 16;

    $prefix = "insert into func_para (module_priority,func_name,func_ret_type,func_para_num,func_para_direction_0,func_para_direction_1,func_para_direction_2,func_para_direction_3,func_para_direction_4,func_para_direction_5,func_para_direction_6,func_para_direction_7,func_para_direction_8,func_para_direction_9,func_para_direction_10,func_para_direction_11,func_para_direction_12,func_para_direction_13,func_para_direction_14,func_para_direction_15,func_para_type_0,func_para_type_1,func_para_type_2,func_para_type_3,func_para_type_4,func_para_type_5,func_para_type_6,func_para_type_7,func_para_type_8,func_para_type_9,func_para_type_10,func_para_type_11,func_para_type_12,func_para_type_13,func_para_type_14,func_para_type_15)";

    $func_paras_ref = $$func_node{"paras"};
    $func_paras_num = scalar(@$func_paras_ref);

    if( ! defined($$func_node{"priority"}) )
    {
        printf STDERR ("priority not defined: %s\n", $$func_node{"proto_type"});
        exit;
    }
    if( ! defined($$g_md_prio{ $$func_node{"priority"} }) )
    {
        printf STDERR ("md_prio not defined: priority = %s, %s\n", $$func_node{"priority"}, $$func_node{"proto_type"});
        exit;
    }

    if( ! defined($$func_node{"ret_type"}) )
    {
        printf STDERR ("ret_type not defined: %s\n", $$func_node{"ret_type"});
        exit;
    }
    if( ! defined($$g_dbg_type{ $$func_node{"ret_type"} }) )
    {
        printf STDERR ("dbg_type not defined: ret_type = %s, %s\n", $$func_node{"ret_type"}, $$func_node{"proto_type"});
        exit;
    }

    $value = sprintf("%d,\"%s\",%d,%d", 
            $$g_md_prio{ $$func_node{"priority"} }, 
            $$func_node{"name"}, 
            $$g_dbg_type{ $$func_node{"ret_type"} },
            $func_paras_num);

    # directions
    for( $index = 0; $index < $func_paras_num; $index ++ )
    {
        if( ! defined($$func_paras_ref[ $index ]{"direction"}) )
        {
            printf STDERR ("direction not defined: index = %d, func_name = %s\n", 
                    $index, 
                    $$func_node{"name"});
            exit;
        }
        if( ! defined($$g_para_dir{ $$func_paras_ref[ $index ]{"direction"} }) )
        {
            printf STDERR ("para dir not defined: index = %d, func_name = %s, type = %s\n",     
                    $index, 
                    $$func_node{"name"},
                    $$func_paras_ref[ $index ]{"direction"});
            exit;
        }

        $value .= ",".$$g_para_dir{ $$func_paras_ref[ $index ]{"direction"} };
    }
    for( ; $index < $func_paras_max_num; $index ++ )
    {
        $value .= ",0";
    }

    # types; 
    for( $index = 0; $index < $func_paras_num; $index ++ )
    {
        if( ! defined($$func_paras_ref[ $index ]{"type"}) )
        {
            printf STDERR ("type not defined: index = %d, func_name = %s\n", 
                    $index, 
                    $$func_node{"name"});
            exit;
        }
        if( ! defined($$g_dbg_type{ $$func_paras_ref[ $index ]{"type"} }) )
        {
            printf STDERR ("dbg type not defined: index = %d, func_name = %s, type = %s\n",     
                    $index, 
                    $$func_node{"name"},
                    $$func_paras_ref[ $index ]{"type"});
            exit;
        }

        $value .= ",".$$g_dbg_type{ $$func_paras_ref[ $index ]{"type"} };
    }
    for( ; $index < $func_paras_max_num; $index ++ )
    {
        $value .= ",0";
    }

    #printf $fp ("-- %s\n", $$func_node{"proto_type"});
    #printf $fp ("value: %s\n", $value);

    printf $fp ("%s values (%s);\n", $prefix, $value);
    printf $fp ("\n");
}

#######################################################################################################################
# print_func_node($fp, %func_node)
#######################################################################################################################
sub print_func_node 
{
    my $fp;
    my $func_node;

    my $func_paras_ref;
    my $space = "    ";

    ($fp, $func_node) = @_;

    $func_paras_ref = $$func_node{"paras"};

    printf $fp ("{\n");
    printf $fp ("/* -- %s -- */\n", $$func_node{"proto_type"});
    printf $fp ("/*func priority   */%s %s,\n"     , $space, $$func_node{"priority"});
    printf $fp ("/*func logic addr */%s (UINT32)%s,\n", $space, $$func_node{"name"});
    printf $fp ("/*func beg addr   */%s 0,\n"     , $space);
    printf $fp ("/*func end addr   */%s 0,\n"     , $space);
    printf $fp ("/*func addr offset*/%s 0,\n"     , $space);
    printf $fp ("/*func name       */%s \"%s\",\n"     , $space, $$func_node{"name"});
    printf $fp ("/*func index      */%s FI_%s,\n"     , $space, $$func_node{"name"});    
    printf $fp ("/*func ret type   */%s %s,\n"     , $space, $$func_node{"ret_type"});
    printf $fp ("/*func para num   */%s %d,\n"     , $space, scalar(@$func_paras_ref));
    printf $fp ("/*func para direct*/%s %s\n"     , $space, &encap_func_para_directions($func_paras_ref));
    printf $fp ("/*func para type  */%s %s\n"     , $space, &encap_func_para_types($func_paras_ref));
    printf $fp ("/*func para val   */%s 0, 0, {0},\n", $space);
    printf $fp ("},\n");

    #&print_func_paras($fp, $$func_node{"paras"});
}

#######################################################################################################################
# print_func_index_enum($fp, %func_node)
#######################################################################################################################
sub print_func_index_enum 
{
    my $fp;
    my $func_node;

    my $func_name;
    my $func_index;
    my $mod_name;
    my $space = "    ";

    ($fp, $func_node) = @_;

    $func_name = $$func_node{"name"};
    if( $func_name =~ /^([a-zA-Z0-9]+)_([a-zA-Z0-9]+).*/ )
    {
        $mod_name = "MD_".uc($1).uc($2);
        $func_index = "FI_".$func_name;

        $g_func_index_enum ++;
        printf $fp ("%64s = ((%s << (WORDSIZE/2)) + %4d),\n", $func_index, $mod_name, $g_func_index_enum);
    }    

    #&print_func_paras($fp, $$func_node{"paras"});
}

#######################################################################################################################
# encap_func_para_directions(@func_paras)
#######################################################################################################################
sub encap_func_para_directions
{
    my $func_paras;

    my $index;
    my $func_paras_num;

    my $para_dir;
    my $para_directions;

    ($func_paras) = @_;

    $func_paras_num = scalar(@$func_paras);

    if( 0 == $func_paras_num )
    {
        return "{E_DIRECT_END,},";
    }

    $para_directions = "{";
    for( $index = 0; $index < $func_paras_num; $index ++ )
    {
        $para_dir = $$func_paras[ $index ]{"direction"};
        $para_directions .= "$para_dir,"; 
    }
    $para_directions .= "},";

    return $para_directions;
}

#######################################################################################################################
# encap_func_para_types(@func_paras)
#######################################################################################################################
sub encap_func_para_types
{
    my $func_paras;

    my $index;
    my $func_paras_num;

    my $para_type;
    my $para_types;

    ($func_paras) = @_;

    $func_paras_num = scalar(@$func_paras);

    if( 0 == $func_paras_num )
    {
        return "{e_dbg_type_end,},";
    }

    $para_types= "{";
    for( $index = 0; $index < $func_paras_num; $index ++ )
    {
        $para_type = $$func_paras[ $index ]{"type"};
        $para_types .= "$para_type,"; 
    }
    $para_types .= "},";

    return $para_types;
}

#######################################################################################################################
# print_func_paras($fp, @func_paras)
#######################################################################################################################
sub print_func_paras
{
    my $fp;
    my $func_paras;

    my $index;
    my $func_paras_num;

    ($fp, $func_paras) = @_;

    $func_paras_num = scalar(@$func_paras);
    for( $index = 0; $index < $func_paras_num; $index ++ )
    {
        &print_func_para($fp, $$func_paras[ $index ]);
    }
}

#######################################################################################################################
# print_func_para($fp, %func_para)
#######################################################################################################################
sub print_func_para
{
    my $fp;
    my $func_para;

    ($fp, $func_para) = @_;

    #printf STDOUT ("func para dir : %s\n", $$func_para{"direction"});
    #printf STDOUT ("func para ptr : %s\n", $$func_para{"pointers"});
    #printf STDOUT ("func para type: %s\n", $$func_para{"type"});

    printf STDOUT ("func para type: %s %s %s\n", $$func_para{"direction"}, $$func_para{"pointers"}, $$func_para{"type"});
}

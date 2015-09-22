use File::Find;

$argc = $#ARGV;
if($argc == -1)
{
    print "\n";
    print "Arguments: <simName> <execute_flag>\n";
    print "\n";
    print "simName      The simName used when comitting\n";
    print "execute_flag 0: show switches, do not actually switch\n";
    print "             1: switch jobs to sim_high\n";
    print "\n";
    exit;
}


#############################
# List of the 20 longest jobs
#############################
@LONG_JOBS =
(
# [ 'ld_loco_',  'BQTerrace_1920x1080_60qp27'] ,                       #  Total Time:    52346.310 sec.
# [ 'ra_loco_',  'BQTerrace_1920x1080_60qp22'] ,                       #  Total Time:    52682.450 sec.
 [ 'ra_',       'SteamLocomotiveTrain_2560x1600_60_10bit_cropqp27'] , #  Total Time:    52756.070 sec.
 [ 'ld_',       'BasketballDrive_1920x1080_50qp37'] ,                 #  Total Time:    53319.490 sec.
 [ 'ld_',       'BasketballDrive_1920x1080_50qp32'] ,                 #  Total Time:    55872.800 sec.
 [ 'ra_',       'BasketballDrive_1920x1080_50qp22'] ,                 #  Total Time:    56070.320 sec.
 [ 'ld_',       'BQTerrace_1920x1080_60qp32'] ,                       #  Total Time:    56111.720 sec.
 [ 'ld_',       'BQTerrace_1920x1080_60qp37'] ,                       #  Total Time:    58132.830 sec.
 [ 'ld_',       'Cactus_1920x1080_50qp22'] ,                          #  Total Time:    58213.310 sec.
 [ 'ra_',       'Nebuta_2560x1600_60_10bit_cropqp32'] ,               #  Total Time:    59292.710 sec.
 [ 'ld_',       'BasketballDrive_1920x1080_50qp27'] ,                 #  Total Time:    60547.470 sec.
 [ 'ra_',       'SteamLocomotiveTrain_2560x1600_60_10bit_cropqp22'] , #  Total Time:    62016.400 sec.
 [ 'ra_loco_',  'Nebuta_2560x1600_60_10bit_cropqp27'] ,               #  Total Time:    63256.940 sec.
 [ 'ld_',       'BQTerrace_1920x1080_60qp27'] ,                       #  Total Time:    65372.300 sec.
 [ 'ld_',       'BasketballDrive_1920x1080_50qp22'] ,                 #  Total Time:    65585.870 sec.
 [ 'ra_',       'BQTerrace_1920x1080_60qp22'] ,                       #  Total Time:    67675.630 sec.
 [ 'ld_loco_',  'BQTerrace_1920x1080_60qp22'] ,                       #  Total Time:    67967.050 sec.
 [ 'ra_',       'Nebuta_2560x1600_60_10bit_cropqp27'] ,               #  Total Time:    79089.520 sec.
 [ 'ld_',       'BQTerrace_1920x1080_60qp22'] ,                       #  Total Time:    87913.300 sec.
 [ 'ra_loco_',  'Nebuta_2560x1600_60_10bit_cropqp22'] ,               #  Total Time:    92162.010 sec.
 [ 'ra_',       'Nebuta_2560x1600_60_10bit_cropqp22'] ,               #  Total Time:   104637.180 sec.
);

$simname = @ARGV[0];

$dir_for_list = 'jobs';
loadFiles();

foreach $file (@FILES)
{
    if($file =~ /$simname/)
    {
        foreach $el (@LONG_JOBS)
        {
            $condition = $el->[0];
            $coding_case = $el->[1];
            
            if($file =~ /$coding_case/)
            {
                # dont do loco jobs when the condition is not loco
                if((not $condition =~ /loco/) and ($file =~ /loco/)) {} else
                {
                    if($file =~ /$condition/)
                    {
                        open(FILE, $file);
                        while(<FILE>)
                        {
                            chop;
                            $id = $_;
                        }
                        close(FILE);
                        $cmd = "bswitch sim_high $id";
                        print "$cmd\n";
                        if($argc == 1 and @ARGV[1] eq '1') {
                            system($cmd);
                        }
                    }
                }
            }
        }
    }
}
if($argc == 1 and @ARGV[1] eq '1') {} else
{
    print "\n";
    print "Arguments: <simName> <execute_flag>\n";
    print "\n";
    print "simName      The simName used when comitting\n";
    print "execute_flag 0: show switches, do not actually switch\n";
    print "             1: switch jobs to sim_high\n";
    print "\n";
}


sub loadFiles()
{
  find(\&mySub,"$dir_for_list"); #custom subroutine find, parse $dir_for_list
}

# following gets called recursively for each file in $dir_for_list, check $_ to see if you want the file!
sub mySub()
{
    push @FILES, $File::Find::name if(/lsfJobStatus/i); # modify the regex as per your needs or pass it as another arg
}

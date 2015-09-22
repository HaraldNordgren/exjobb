
$argc = $#ARGV;
if($argc != 0)
{
    print "Supply a sleep argument\n";
    exit;
}

$sleeptime = @ARGV[0];

while(1)
{
    $time = localtime;
    $time =~ s/\t/_/g;
    $time =~ s/:/_/g;
    $time =~ s/ /_/g;
    
    print "$time\n";
    system("df -h . \| grep G");
#    print "Disk usage:     ";
#    system("du -B M --max-depth 0");
    system("bjobs > bjobs.txt");
    print "Jobs running:   ";
    system("cat bjobs.txt \| grep RUN \| wc -l");
    
    print "     Of which in 24h queue:   ";
    system('cat bjobs.txt | grep 24h | grep RUN | wc -l');
    print "     Of which in 15min queue: ";
    system('cat bjobs.txt | grep 15min | grep RUN | wc -l');
    print "     Of which in 30min queue: ";
    system('cat bjobs.txt | grep 30min | grep RUN | wc -l');
    print "     Of which in sim_high:    ";
    system('cat bjobs.txt | grep "sim_high " | grep RUN | wc -l');
    print "     Of which in sim queue:   ";
    system('cat bjobs.txt | grep "sim " | grep RUN | wc -l');
    
    print "Jobs waiting:   ";
    system("cat bjobs.txt \| grep PEND \| wc -l");
    print "     Of which in 24h queue:   ";
    system('cat bjobs.txt | grep 24h | grep PEND | wc -l');
    print "     Of which in 15min queue: ";
    system('cat bjobs.txt | grep 15min | grep PEND | wc -l');
    print "     Of which in 30min queue: ";
    system('cat bjobs.txt | grep 30min | grep PEND | wc -l');
    print "     Of which in sim_high:    ";
    system('cat bjobs.txt | grep "sim_high " | grep PEND | wc -l');
    print "     Of which in sim queue:   ";
    system('cat bjobs.txt | grep "sim " | grep PEND | wc -l');
    


    print "Jobs suspended: ";
    system("cat bjobs.txt \| grep SSUSP \| wc -l");
    print "     Of which in 24h queue:   ";
    system('cat bjobs.txt | grep 24h | grep SSUSP | wc -l');
    print "     Of which in 15min queue: ";
    system('cat bjobs.txt | grep 15min | grep SSUSP | wc -l');
    print "     Of which in 30min queue: ";
    system('cat bjobs.txt | grep 30min | grep SSUSP | wc -l');
    print "     Of which in sim_high:    ";
    system('cat bjobs.txt | grep "sim_high " | grep SSUSP | wc -l');
    print "     Of which in sim queue:   ";
    system('cat bjobs.txt | grep "sim " | grep SSUSP | wc -l');

    sleep $sleeptime;
    sleep 1;
    
}



sub isNumber {
        !/\d/ ? 0 : $_ == 0 ? 1 : $_ * 1
}

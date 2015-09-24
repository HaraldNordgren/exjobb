
# this file tests:
# foreman	gcif 	1 1 tile 	300 frames 	qp 49 GOP: 0
# vidyo1	720p 	3 2 tiles	100 frames 	qp 16 GOP: 0

use Time::HiRes qw( time );

$enc = 'c65_app.exe';
$dec = '..\bin\d65_dec_D.exe';

if(!-e $dec)
{
    printf("Decoder $dec does not exist\n");
    exit;
}
if(!-e $enc)
{
    printf("Encoder $enc does not exist\n");
    exit;
}


&Run;

sub Run
{
    for($i = 0; $i < 2; $i++)
    {
        $v = 1;
        $intraSize = 0;
        $bucketSize = 0;
        $rc = 0;
        $bitrate = 0;
        $fps = 25;
        $speed = 1;
        $speedParameter = 3;
        if($i == 0){
            $input = 'c:\seqs\foreman.qcif';
            $width = 176;
            $height = 144;
            $horiz_tiles = 1;
            $vert_tiles = 1;
            $frames = 300;
            $qp = 49;
            $gop = 0;
        }
        if($i == 1){
            $input = 'c:\seqs\vidyo1.yuv';
            $width = 1280;
            $height = 720;
            $horiz_tiles = 3;
            $vert_tiles = 2;
            $frames = 100;
            $qp = 16;
            $gop = 0;
        }
        
        if(1) {
            $cmd = "$enc ";
            $cmd = $cmd . "-q $qp ";
            $cmd = $cmd . "-w $width ";
            $cmd = $cmd . "-h $height ";
            $cmd = $cmd . "-f $frames ";
            $cmd = $cmd . "-i $input ";
            $cmd = $cmd . "-o test.265 ";
            $cmd = $cmd . "-r rec.yuv ";
            $cmd = $cmd . "-T $horiz_tiles $vert_tiles ";
            $cmd = $cmd . "-RC $rc ";
            $cmd = $cmd . "-b $bitrate ";
            $cmd = $cmd . "-I $intraSize ";
            $cmd = $cmd . "-B $bucketSize ";
            $cmd = $cmd . "-FPS $fps ";
            $cmd = $cmd . "-S $speed $speedParameter ";
            $cmd = $cmd . "-g $gop ";
            print "# Enc #\n$cmd\n";
            my $start = time();
            if($v >= 3)  { system("$cmd"); }
            if($v < 3) { system("$cmd > enc.txt"); }	
            my $end = time(); 
            printf("Total time: %.2f s\n", ($end - $start));	
            printf("Total speed: %.2f fps\n", $frames /($end - $start));			
        }
        
        $cmd = "$dec -i test.265 -o dec.yuv";
        print "# Dec #\n$cmd\n";
        my $start = time();
        if($v >= 2)  { system("$cmd"); }
        if($v < 2) { system("$cmd > dec.txt"); }
        my $end = time(); 
        printf("Total decoding time: %.2f s\n", ($end - $start));	
        printf("Total decoding speed: %.2f fps\n", $frames /($end - $start));	
        
        $cmd = "cmp_seqs dec.yuv rec.yuv > junk.txt";
        print "$cmd\n";
        system($cmd);
        $size = -s "junk.txt";
        if($size == 0) { print "Decoded sequence matches encoded sequence\n\n"; } else { print "NOT OK NOT OK NOT OK Decoded sequence does not match encoded sequence NOT OK NOT OK NOT OK\n\n"; }
    }
}




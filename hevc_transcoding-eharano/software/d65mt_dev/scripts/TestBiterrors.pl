use POSIX ();

$decoder = '..\bin\d65_dec.exe';

$applyerrors = 'C:\proj\c65_d65\d65\scripts\robustness\applybiterrors\Release\applybiterrors.exe';

$errorbitstream = "error.265";

$infileoffset = 0;
$loop = 10;
$loop = 10000;
$loop = 800;
$loop = 4000;
$loop = 6100;

$errorfile = "18681.3";
&DecodeSequences;
$errorfile = "18681.4";
&DecodeSequences;
$errorfile = "wcdma_128kb_3kph_5e-04.bin";
&DecodeSequences;
$errorfile = "wcdma_128kb_50kph_2e-04.bin";
&DecodeSequences;
$errorfile = "wcdma_64kb_3kph_5e-04.bin";
&DecodeSequences;
$errorfile = "wcdma_64kb_50kph_2e-04.bin";
&DecodeSequences;


sub DecodeSequences
{
    printf("\n##### Errorfile: %s #####\n", $errorfile);

    $x = localtime();
    printf("%s\n", $x);

    $bitstream = "test_0.265";
    &TestBitstream();
}




sub TestBitstream
{
    $errorfileoffset = 0;

    $bytes_bitstream = -s $bitstream;
    printf("bitstreamsize: %i\n", $bytes_bitstream);

    $bytes_errorfile = -s $errorfile;
    printf("errorfilesize: %i\n", $bytes_errorfile);


    $increment = POSIX::floor($bytes_errorfile/$loop);

    printf("%s\n", $bitstream);
    printf("for loop starts at %i and ends at %i\n", $errorfileoffset, ($bytes_errorfile-$increment));
    printf("Increment: %i\n", $increment);
    $counter = 0;
    while($errorfileoffset < $bytes_errorfile)
    {
        $counter++;
        if(($counter % 100) == 0)
        {
            printf("%i", $errorfileoffset);
            $x = localtime();
            printf(" %s ", $x);
        }
        printf("$applyerrors $bitstream $errorbitstream $errorfile $errorfileoffset $infileoffset\n");

        system("$applyerrors $bitstream $errorbitstream $errorfile $errorfileoffset $infileoffset");
        $res = system("$decoder -i $errorbitstream > junk");
        if($res != 0)
        {
            $x = localtime();
            printf("%s\n", $x);
            printf("########## %i ##########\n", $errorfileoffset);
            exit;
        }
        else
        {
            printf(".");
        }
        $errorfileoffset += $increment;

    }
    $x = localtime();
    printf("%s\n", $x);
}



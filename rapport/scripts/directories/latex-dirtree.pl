#!/usr/bin/perl -w
use strict;
use File::Find;

my $max_depth = 10;

my $top = shift @ARGV;
die "specify top directory\n" unless defined $top;
chdir $top or die "cannot chdir to $top: $!\n";

printf "\\dirtree{%%\n";
find( {
    preprocess => \&preprocess,
    wanted => \&wanted
}, '.');
print "}\n";


sub preprocess {
    my $depth = $File::Find::dir =~ tr[/][];
    return @_ if $depth < $max_depth;
    return grep { not -d } @_ if $depth == $max_depth;
    return;
}

sub wanted {
    local $_ = $File::Find::name;
    $_ =~ s/_/{\\_}/g;
    my @F = split '/';
    printf "\t.%d %s.\n", scalar @F, @F==1 ? $top : $F[-1];
}
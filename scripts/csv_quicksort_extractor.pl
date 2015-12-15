
use strict;

my($line);
my($size);

print "Version Size Threads Time\n" ;
while($line=<>) {
    chomp $line;
    if($line =~/^Size: ([\d\.]*)$/) {
        $size = $1;
        next;
    } 
    if($line =~/^(.*) quicksort for (.*) elements using (.*) threads.*: ([\d\.]*) sec.$/) {
        print "$1 $2 $3 $4\n" ;
        next;
    } 
}

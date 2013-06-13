#!/usr/bin/perl

# @@@LICENSE
#
#      Copyright (c) 2010-2013 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# LICENSE@@@

# **********************************************************
# FileName: skValidator.pl
# Author  : Dang Le (WebOS QA)
# Desc    : validate the integrity of SmartKey Service. 
# **************************************************************************************


use strict;
use Getopt::Std;

use vars qw(%opts);

# ******************************
# Predefined Variables
# ***********************************************
my $EXACTMATCH = 1;
my $FUZZYMATCH = 2;
my $ADAPTIVETST = 3;
my $MEMTST = 4;

# *********************
# Cmdline arguements and their default values
# ************************************************
my $tstMethod = 1;				# -t  1=exactMatch  2=fuzzyMatch 3=adaptive 4=memoryTest
my $DictFile = "key_dict_us_1-21";		# -D  
my $nEntry = 20;				# -n
my $wordlen = 5;				# -l
my $modLevel = 0;				# -m  # of characters replace by its equivalent.

my $strOpts = 'hD:l:m:n:t:';
getopts("$strOpts",\%opts) or helpMsg();	#Usage is called if there are invalid input.
helpMsg() if( $opts{h} || $opts{H});





$tstMethod = $opts{t} if($opts{t});		#override default settings
$DictFile = $opts{D} if($opts{D});
$nEntry = $opts{n} if($opts{n});
$wordlen = $opts{l} if($opts{l});
$modLevel = $opts{m} if($opts{m});

die("\n\n### ERROR ###: Can not support options m > l; use -h for detail. \n\n") if($modLevel > $wordlen);
die("\n\n### ERROR ###: Available test methods: 1=exactMatch  2=fuzzyMatch 3=adaptive 4=memoryTest; Use -h for detail.\n\n") if($tstMethod > 4 || $tstMethod < 1);
die("\n\n### ERROR ###: Valid word length is 2>=x<6; use -h for detail.\n\n") if($wordlen <2  || $wordlen >=6);


#  **************
# global vars
# ****************
my %g_KeyEquivTable;				# holds keyvalues pair that define the key_equiv file.
my @g_orderedWordList;				# full dictionary list sorts  by word rank.
my @g_origSubset;			  		# selected subset of @orderedWordList
my @g_noisedWordList;				# list of modified words from @origSubset. 


# ***********************
# statistic data: use for generating report results
# ********************************************************************
my $tstName = "Exact Match";
my $g_totaltst = 0;
my $g_totalpass = 0;
my $g_totalfail = 0;
my $starttime;
my $endtime;

$starttime = getCurrentTime();
cleanUpPreviousRun();

createEquivTable(\%g_KeyEquivTable);			#bld equiv table.
orderDictByRank(\@g_orderedWordList);
@g_origSubset = getTestList(\@g_orderedWordList, $nEntry, $wordlen);  
genNoisedWordList(\@g_origSubset,$modLevel,\@g_noisedWordList) if($tstMethod == $FUZZYMATCH);	#only bld noisedwords for fuzzy match.

my $tstResult = "tmpFailedResult.txt";
smartKeyValidator(\@g_origSubset, \@g_origSubset, $tstResult) if($tstMethod == $EXACTMATCH);

smartKeyValidator(\@g_origSubset, \@g_noisedWordList, $tstResult) if($tstMethod == $FUZZYMATCH);
adaptiveTest() if($tstMethod == $ADAPTIVETST);
memoryTest() if($tstMethod == $MEMTST);

getTestName(\$tstName,$tstMethod);
$endtime = getCurrentTime();
print "\nStart time: $starttime";
print "\nEnd time: $endtime";
print "\nTest Method:  $tstName for the top $nEntry words with length >= $wordlen";
print "\nCharacter replaced: $modLevel" if($tstMethod == $FUZZYMATCH);
print "\nTotal # of tests: $g_totaltst";
print "\nTotal # of passed: $g_totalpass";
print "\nTotal # of failed: $g_totalfail\n\n";



open(INFILE,"$tstResult") || die("\n\nCan not open for reading: $tstResult\n\n");
my $entry = 0;
my $tstkey;
my $expectedkey;
my $actualkey;

while(<INFILE>)
{
   s/^\s+|\s+$//g;
   next if(length($_) == 0);
   $entry += 1;
   ($tstkey, $expectedkey, $actualkey) = split(/,/, $_);
   write();
} #while

format STDOUT_TOP =
Entry   TestKey        ExpectedKey        ActualKey return from luna-send
=====   =======        ===========        ================================
.

format STDOUT =
@<<<<   @<<<<<<        @<<<<<<<<<<        @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
$entry $tstkey        $expectedkey       $actualkey
.


#close(OFILE);




# ================== End of Main =============================


# *****************************
# Desc: Create lookup table for key_equiv file; a helper function
# **********************************************************************
sub createEquivTable()
{
   my $KeyEquiv = "key_equiv";
   my $ref_var = shift;

   open(MAPFILE,$KeyEquiv) || die("could not open file: $KeyEquiv");

   #Build hashtable for KeyEquiv data
   while(<MAPFILE>)
   {
      my ($letter,$equiv)=split(/=/,$_);
      chomp($equiv);
      $$ref_var{$letter} = $equiv;
   }
} #createEquivTable


# **********************
# Desc: Input words that are unknown to dictionary will get added as new items
# ***********************************************************************************
sub adaptiveTest()
{
   print "\n\n ******* TBA in next release ******** \n\n";
   exit;
}#adaptiveTest


# ****************************
# Desc: Test for memory leak
# **************************************************
sub memoryTest()
{
   print "\n\n ******* TBA in next release ******** \n\n";
   exit;

} #memoryTest


# *************************************
# Desc: Testing the integrity of the dictionary; All input words are expected to be matched. write result to a file.
#    INPUT: origlist, tstlist, outputfile
#    OUTPUT: none
#    Possible  return message from luna-send command:
#      
#       luna-send -n 1 palm://com.palm.smartKey/search '{"query":"trst"}'
#       ** Message: serviceResponse Handling: 2, { "returnValue": true, "match": "test" }    <=== exact match; word correcte
#
#       luna-send -n 1 palm://com.palm.smartKey/search '{"query":"kdjjsljlsjld"}'
#       ** Message: serviceResponse Handling: 2, { "returnValue": true, "match": "" }        <=== bad match
# **********************************************************************************************
sub smartKeyValidator()
{
   my $ret1 = 0;
   my $ret2 = 0;
   my $reslt1 = 0;
   my $reslt2 = 0;
   my $tmpstr = "";
   my $tmp2 = "";
   my $count = 0;

   my ($ref_origlist, $ref_tstlist, $tmpfailed) = @_;		
   open(OUTFAILED,">$tmpfailed") || die("\n\n ### ERROR ###:  CAN NOT OPEN FOR WRITING:   $tmpfailed\n\n");

   $g_totaltst = 0;
   print "\n Test in progress for $nEntry entries....\n\n";
   foreach (@$ref_tstlist)
   {
       #chomp($_);
       my $json = "\"{\\\"query\\\":\\\"$_\\\"}\"";             #forming json string: "{\"query\":\"trst\"}"
       $tmpstr = `luna-send -n 1 palm://com.palm.smartKey/search $json  2>&1`;
       $tmp2 = substr($tmpstr,index($tmpstr,'returnValue'));    # $tmp2 = returnValue": true, "match": "test" }
       $tmp2 =~ s/"//g;				
       $tmp2 =~ s/ //g;
       $tmp2 =~ s/}//g;
       chomp($tmp2);

       my ($fld1, $fld2) = split(/,/, $tmp2);
       ($ret1, $ret2) = split(/:/, $fld1);              # result of luna-send call
       ($reslt1, $reslt2) = split(/:/, $fld2);          # result of query; if $reslt2 is empty then did not find a match.
       $ret2 = lc($ret2);               		#make all lowercase

       if($ret2 eq 'true' && length($reslt2) )		
       {
    
          if(($$ref_origlist[$g_totaltst] eq $reslt2))
          {
                print ".";
                $g_totalpass += 1
          }
          else
          {    # luna-send succeed but the return value is different
                print "\*";
                $g_totalfail += 1;
                print( OUTFAILED "\n$_,$$ref_origlist[$g_totaltst],$reslt2");
          }
       }
       else
       {
         print "\*";
         $g_totalfail += 1;
         print( OUTFAILED "\n$_,$$ref_origlist[$g_totaltst],$reslt2");
       }
       $g_totaltst += 1;
    } #while loop

    print (OUTFAILED "\n");
    close(OUTFAILED);
}#smartKeyValidator




# ****************************
# Desc: Alter the list-words with its equivelant key based on key_equiv file
#       INPUT: list-words, alteration level, outputarray
# **********************************************************************************
sub genNoisedWordList()
{
   my ($ref_origlist, $level, $ref_outArray) = @_;
   my $noisedfile = "NoisedWords.txt";
   my @lookupChar; 
   my @tmpar;
   my $cnt = 0; 

   open(OUTFILE,">$noisedfile") || die("\n\nCan not open for writing: $noisedfile\n\n");
   @$ref_outArray = @$ref_origlist;		#assign to outArray so we can manipulate and get instant update. 
   for($cnt=0;$cnt<$level;++$cnt)		#loop to change the number of characters in a given word.
   {
      my $indx = 0;
      foreach(@$ref_outArray)
      {
          my $lcase = lc($_);					# convert lowercase so that Htable don't fail due to capital letter.
          my @tmpword = convertStr2Array($lcase);		#make all lowercase
          my $eqKey = $g_KeyEquivTable{$tmpword[$cnt]};
          my @arrEQ = convertStr2Array($eqKey); 
          $tmpword[$cnt] = $arrEQ[0];				#for now always use first equivalent key.
          my $newstr = join("",@tmpword);
          $tmpar[$indx] = $newstr;
          $indx += 1;
      } #foreach
     
      @$ref_outArray = @tmpar;
   } #for

   my $tstflag = 0;
   foreach (@$ref_outArray)
   {
       if($tstflag > 0)
       {
           print OUTFILE "\n$_";
       }
       else
       {
           print OUTFILE "$_";
           $tstflag = 1;
       }
   }
   
   print OUTFILE "\n";
   close(OUTFILE);
}#genNoisedWordList 


# ****************************************
# Desc: convert string to array.
# ****************************************************
sub convertStr2Array()
{
   my $str1 = shift;       # get the argument
   my @array = split("",$str1);
   return @array;
}#convertStr2Array



# *********************
# Desc: return type of test is running
#       INPUT:  testname & testvalue.
# *********************************************
sub getTestName()
{
    my ($ref_name, $tstmethod) = @_;
    $$ref_name = "Exact Match" if($tstmethod == 1);
    $$ref_name = "Fuzzy Match" if($tstmethod == 2);
    $$ref_name = "Adaptive Test" if($tstmethod == 3);
    $$ref_name = "Memory Test" if($tstmethod == 4);
    
} #getTestName

# *****************************
# Desc: return current time as format: 07:12:22
# ************************************************************
sub getCurrentTime()
{
    my $starttime = `date 2<&1`;            # suppress stderr & stdout to variable
    $starttime =~ s/\s+/ /g;
    my @tmparray = split(/ /,$starttime);

    $starttime = $tmparray[3];
    chomp($starttime);
    return $starttime;
} #getCurrentTime


# *******************************************************
# Desc: create files of length 5,4,3,2 characters; Each file contains N entries
#       INPUT: Array of sorted-ranked words
#	       N for number of entries to test.
#	       M minimum length in words
#	       
#	Return: a new array with N entries and minimun length of M.
# *******************************************************************************************
sub getTestList()
{
    my $tstlist = "SubSetList.txt";
    my ($ref,$Nentries,$Mlen ) = @_;
    my $cnt = 0;
    my @arr;
 
    open(OUTFILE,">$tstlist") || die("\n\nCan not open for writing: $tstlist \n\n");
    foreach( @$ref )
    {
       my ($str1, $str2) = split(/ /,$_);

       if( (length($str2) >= $Mlen) )		#list of words with Mlen or greater length
       {
          last if($cnt >= $Nentries);
          $arr[$cnt] = $str2;
          $cnt += 1;
       }
    } #foreach 

    my $tmpflag = 0;
    foreach (@arr)
    {
        if($tmpflag > 0)
	{
	   print OUTFILE "\n$_";

 	}
	else
	{
	    print OUTFILE "$_";
	    $tmpflag = 1;
	} 
    }

    close(OUTFILE);
    return @arr;
} #getTestList



# Desc: Reorder dictionary by rank (Descending order)
#       INPUT: reference to array  
# *****************************************************************
sub orderDictByRank()
{
   my $sortedlist = "SortedWordsByRanks.txt";
   my $dictfile = "key_dict_us_1-21";
   my $fld1;
   my $fld2;
   my $index = 0;
   my @arr;
   my $reference = shift;

   open(DICTFILE,$dictfile) || die("\n\n### ERROR###: Can not open dictionary: $dictfile\n\n");
   open(OUTFILE,">$sortedlist") || die("\n\n ### ERROR ###: Can not open for writing: $sortedlist\n\n"); 

   while(<DICTFILE>)
   {
  	chomp();
        s/^\s+|\s+$//g;		#remove leading & trailing space
 	next unless(length($_));
  	($fld1,$fld2) = split(/ /,$_);
        my $newstr = join(' ',$fld2, $fld1);		# swap column so we can compare the rank later.
        $arr[$index] = $newstr;				#store string back to array for sorting later.
	$index += 1;    
   }   
   
   @$reference = sort {$b <=> $a} @arr;			# assigned back to reference which points to passed in array

   my $tstflag = 0;
   foreach (@$reference)
   {
        if($tstflag > 0)
        { 
            print OUTFILE "\n$_";
        }
        else
	{
	   print OUTFILE "$_";
           $tstflag = 1;
	}
   }
   close(DICTFILE);
   close(OUTFILE);
} #orderDictByRank


# *****************
# Desc: display help message on how to use this scripts and its options.
# ******************************************************************************
sub helpMsg()
{

print STDERR << "HELPMSG";


Desc: helps validate the integrity of SmartKey Service.  It can generate tst data from SmartkeyService's dictionary.  
      display statistic result at the end of test. 

Options:
	-h	Display this help message.
 	-D	SmartKeyService's dictionary.
	-l	specify minimum word length use for testing.
	-m	specify how many characters to be replaced using equivalent keys from key_equiv file.
	-n	specify number of words use for testing.
	-t	specify (as a numberical value) which test method to use: 1=exactMatch  2=fuzzyMatch 3=adaptive 4=memoryTest.

Example: 
	$0 -D key_dict_us_1-21 -n 20 -l 5 -m 1 -t 1

(Note: the above example is the default settings if user provides no arguments upon invoking the script. )

HELPMSG
exit;
} #helpMsg



# ****************************
# Desc: delete files that got generated in previous run.
# ***************************************************************
sub cleanUpPreviousRun()
{
   `rm -rf SortedWordsByRanks.txt`;
   `rm -rf SubSetList.txt`;
   `rm -rf NoisedWords.txt`;
   `rm -rf tmpFailedResult.txt`;
} #cleanUpPreviousRun

#!/usr/bin/perl

use strict;

# /V13a[URL]*[FixedURL]*[DocumentTitle]*[Referrer]*[JavaSupport]/[ScreenSize]/[ColorDepth]/[AccountName]/[CountryCode]/[EncType]/

my $max_lines = 375000; # - 2million /hr
my $urls  = "urls.txt";
my $uas   = "useragents.txt";
my $lang  = "acceptlang.txt";
my $proxy = "HTTP/1.0 webcache[C0020179] (Traffic-Server/5.2.1-51986 [uScM]) a08677b8e86d86744576cc4707781f54";
my $time  = "1151265600000000"; 
my $ip    = "192.168.1.192"  ;
my $cid   = "a08677b8e86d86744576cc4707781f54";
my $sid   = "dd116eb060d77476df33f91981ff5e15";
my @uri_ary;
my @ref_ary;
my @ua_ary;
my @lang_ary;
#/V13bhttp://www.mail.ru/commutable/palt/man06.htm?disinfeudation=Taiping&tmsec=mail_search***http://www.planterly.com/monostele/overconsumption/scoffingstock.htm?dingleberry=lobbyman&autodidactic=Johnsmas*true/1280x1024/32/mail_ru/ru/CP1251/
open (U, "<$urls");
while (<U>)
{
   chomp;
   chop;
   my ($key, $uri, $ref) = split("\t");
   #my ($a, $b, $c, $d, $e) = split("\\*", $uri); 
   #my ($ja, $sc, $col, $acc, $cc, $enc) = split("/", $e); 
   #print $acc . "\n";
   push(@uri_ary, $uri);
   push(@ref_ary, $ref);
}
close (U);

open (UA, "<$uas");
while (<UA>)
{
   chomp;
   my ($key, $ua) = split("\t");
   push(@ua_ary, $ua);
}
open (L, "<$lang");
while (<L>)
{
   chomp;
   my ($key, $lang) = split("\t");
   push(@lang_ary, $lang);
}

for (my $i = 1; $i <= $max_lines; $i++)
{
   my $index_uri = rand @uri_ary;
   my $index_ref = rand @ref_ary;
   my $index_ua  = rand @ua_ary;
   my $index_la  = rand @lang_ary;
   print $uri_ary[$index_uri] 	. "\t";
   print $time++	 	. "\t";  
   print $ref_ary[$index_ref] 	. "\t";
   print $ip		 	. "\t";
   print $ua_ary[$index_ua] 	. "\t";
   print $lang_ary[$index_la]	. "\t";
   print $proxy 		. "\t";
   print $sid			. "\t";;
   print $cid 			;
   print  "\n";
}

=start


for (my $i = 1; $i <= 45000; $i++)
{
   print   $log_array{'REQUEST_URI'}		. "\t";
   print ++$log_array{'TIME'}			. "\t";
   print   $log_array{'HTTP_REFERER'}		. "\t";
   print   $log_array{'REMOTE_ADDRESS'}		. "\t";
   print   $log_array{'HTTP_USER_AGENT'}	. "\t";
   print   $log_array{'HTTP_ACCEPT_LANGUAGE'}	. "\t";
   print   $log_array{'HTTP_VIA'}		. "\t";
   print   $log_array{'COOKIE_Sid'}		. "\t";
   print   $log_array{'COOKIE_Cid'}	 	;	
   print  "\n";
}
=cut

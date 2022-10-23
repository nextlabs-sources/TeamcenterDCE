#!/usr/bin/perl

#

# DESCRIPTION

#	This script extracts Perforce workspace information from a Hudson job config file.



use strict;

use warnings;



print "NextLabs Get Perforce Workspace Setting Script\n";





#

# Arguements

#



# -----------------------------------------------------------------------------
# Print usage

sub printUsage
{
	print "usage: getPerforceWorkspace.pl <config file>\n";

	print "  config file  - Path to a Hudson job config file.\n";
}



# Print usage

my	$argCount = scalar(@ARGV);



if (($argCount < 1 || $argCount > 1) || $ARGV[0] eq "-h" || $ARGV[0] eq "--help")

{

	printUsage;

	exit 1;

}



# Collect parameters

my	$configFile = $ARGV[0];



# Print parameters

print "Parameters:\n";

print "  Config file       = $configFile\n";





#

# Check for errors

#



if (! -f $configFile)

{

	print "ERROR: $configFile does not exist\n";

	exit 1;

}





#

# Read config file

#



local $/ = undef;

open FILE, $configFile || die "Error opening config file $configFile";

my	$buf = <FILE>;

close FILE;



#print "\nSource Data:\n----------------\n$buf\n\n";



#

# Extract info

#



print "\nINFO: Parse config data\n";



if ($buf =~ /<p4Client>([^<]+)<\/p4Client>/)

{

	print "[BUILD MANIFEST] Perforce Workspace Name: $1\n";

}



if ($buf =~ /<projectPath>([^<]+)<\/projectPath>/s)

{

	print "[BUILD MANIFEST] Perforce Workspace View: $1\n";

}



print "[BUILD MANIFEST] Perforce Label         : $ENV{BUILD_TAG}\n";



print "[BUILD MANIFEST] Hudson Build URL       : $ENV{BUILD_URL}\n";

print "[BUILD MANIFEST] Hudson Build Number    : $ENV{BUILD_NUMBER}\n";

print "[BUILD MANIFEST] Hudson Build Id        : $ENV{BUILD_ID}\n";

print "[BUILD MANIFEST] Hudson Workspace Path  : $ENV{WORKSPACE}\n";



if (defined $ENV{HUDSON_USER})

{

	print "[BUILD MANIFEST] Hudson User            : $ENV{HUDSON_USER}\n";

}



if ($buf =~ /<hudson\.tasks\.Shell>[^<]*<command>([^<]+)<\/command>[^<]*<\/hudson\.tasks\.Shell>/s)

{

	my	$tmpBuf = $1;

	

	$tmpBuf =~ s/\n/; /g;

	

	print "[BUILD MANIFEST] Hudson Build Script    : $tmpBuf\n";

}



exit 0;


#!/usr/bin/perl -w
#
# This script runs the OpenSSL Configure script, then processes the
# resulting file list into our local OpensslLib[Crypto].inf and also
# takes copies of opensslconf.h and dso_conf.h.
#
# This only needs to be done once by a developer when updating to a
# new version of OpenSSL (or changing options, etc.). Normal users
# do not need to do this, since the results are stored in the EDK2
# git repository for them.
#
# Due to the script wrapping required to process the OpenSSL
# configuration data, each native architecture must be processed
# individually by the maintainer (in addition to the standard version):
#   ./process_files.pl
#   ./process_files.pl X64
#   ./process_files.pl [Arch]
#
# Follow the command below to update the INF file:
# 1. OpensslLib.inf ,OpensslLibCrypto.inf and OpensslLibFull.inf
# ./process_files.pl
# 2. OpensslLibAccel.inf and OpensslLibFullAccel.inf
# ./process_files.pl X64
# ./process_files.pl X64Gcc
# ./process_files.pl IA32
# ./process_files.pl IA32Gcc

use strict;
use Cwd;
use File::Copy;
use File::Basename;
use File::Path qw(make_path remove_tree);
use Text::Tabs;

my $comment_character;

#
# OpenSSL perlasm generator script does not transfer the copyright header
#
sub copy_license_header
{
    my $source = $_[1];          #Source file is second (after "perl")
    my $target = $_[0];          #Target file is always last

    my $temp_file_name = "license.tmp";
    open (my $target_file, "<" . $target) || die $target;
    open (my $temp_file, ">" . $temp_file_name) || die $temp_file_name;

    #Add "generated file" warning
    print ($temp_file "$comment_character WARNING: do not edit!\r\n");
    print ($temp_file "$comment_character Generated from $source\r\n");
    print ($temp_file "$comment_character\r\n");
    print ($temp_file "$comment_character Copyright 2004-2022 The OpenSSL Project Authors. All Rights Reserved.\r\n");
    print ($temp_file "$comment_character\r\n");
    print ($temp_file "$comment_character Licensed under the Apache License 2.0 (the \"License\").  You may not use\r\n");
    print ($temp_file "$comment_character this file except in compliance with the License.  You can obtain a copy\r\n");
    print ($temp_file "$comment_character in the file LICENSE in the source distribution or at\r\n");
    print ($temp_file "$comment_character https://www.openssl.org/source/license.html\r\n");

    print ($temp_file "\r\n");
    #Retrieve generated assembly contents
    while (my $line = <$target_file>) {
        $line =~ s/\s+$/\r\n/;      #Trim trailing whitepsace, fixup line endings
        print ($temp_file expand ($line));  #expand() replaces tabs with spaces
    }

    close ($target_file);
    close ($temp_file);

    move ($temp_file_name, $target) ||
        die "Cannot replace \"" . $target . "\"!";
}

#
# Find the openssl directory name for use lib. We have to do this
# inside of BEGIN. The variables we create here, however, don't seem
# to be available to the main script, so we have to repeat the
# exercise.
#
my $inf_file;
my $OPENSSL_PATH;
my $uefi_config;
my $extension;
my $compile;
my $arch;
my @inf;
my @asmfilelist;

BEGIN {
    $inf_file = "OpensslLib.inf";
    $uefi_config = "UEFI";
    $arch = shift;

    if (defined $arch) {
        if (uc ($arch) eq "X64") {
            $arch = "X64";
            $uefi_config = "UEFI-x86_64";
            $extension = "nasm";
            $compile = "MSFT";
            $comment_character = ";";
            @asmfilelist = ("x86_64cpuid.s",
                            "aes/vpaes-x86_64.s", "aes/aesni-x86_64.s", "aes/aesni-sha1-x86_64.s ", "aes/aesni-sha256-x86_64.s", "aes/aesni-mb-x86_64.s",
                            "sha/sha1-x86_64.s", "sha/sha256-x86_64.s", "sha/sha512-x86_64.s", "sha/sha1-mb-x86_64.s", "sha/sha256-mb-x86_64.s",
                            "modes/ghash-x86_64.s", "modes/aesni-gcm-x86_64.s"
                            );
        } elsif (uc ($arch) eq "X64GCC") {
            $arch = "X64Gcc";
            $uefi_config = "UEFI-x86_64-GCC";
            $extension = "S";
            $compile = "GCC";
            $comment_character = "#";
            @asmfilelist = ("x86_64cpuid.s",
                            "aes/vpaes-x86_64.s", "aes/aesni-x86_64.s", "aes/aesni-sha1-x86_64.s", "aes/aesni-sha256-x86_64.s", "aes/aesni-mb-x86_64.s",
                            "sha/sha1-x86_64.s", "sha/sha256-x86_64.s", "sha/sha512-x86_64.s", "sha/sha1-mb-x86_64.s", "sha/sha256-mb-x86_64.s",
                            "modes/ghash-x86_64.s", "modes/aesni-gcm-x86_64.s"
                            );
        } elsif (uc ($arch) eq "IA32") {
            $arch = "IA32";
            $uefi_config = "UEFI-x86";
            $extension = "nasm";
            $compile = "MSFT";
            $comment_character = ";";
            @asmfilelist = ("x86cpuid.S",
                            "aes/vpaes-x86.S", "aes/aesni-x86.S",
                            "sha/sha1-586.S", "sha/sha256-586.S", "sha/sha512-586.S",
                            "modes/ghash-x86.S"
                            );
        } elsif (uc ($arch) eq "IA32GCC") {
            $arch = "IA32Gcc";
            $uefi_config = "UEFI-x86-GCC";
            $extension = "S";
            $compile = "GCC";
            $comment_character = "#";
            @asmfilelist = ("x86cpuid.S",
                            "aes/vpaes-x86.S", "aes/aesni-x86.S",
                            "sha/sha1-586.S", "sha/sha256-586.S", "sha/sha512-586.S",
                            "modes/ghash-x86.S"
                            );
        } else {
            die "Unsupported architecture \"" . $arch . "\"!";
        }
        $inf_file = "OpensslLibAccel.inf";
        if ($extension eq "nasm") {
            if (`nasm -v 2>&1`) {
                #Presence of nasm executable will trigger inclusion of AVX instructions
                die "\nCannot run assembly generators with NASM in path!\n\n";
            }
        }

        # Prepare assembly folder
        if (-d $arch) {
            opendir my $dir, $arch ||
                die "Cannot open assembly folder \"" . $arch . "\"!";
            while (defined (my $file = readdir $dir)) {
                if (-d "$arch/$file") {
                    next if $file eq ".";
                    next if $file eq "..";
                    remove_tree ("$arch/$file", {safe => 1}) ||
                       die "Cannot clean assembly folder \"" . "$arch/$file" . "\"!";
                }
            }

        } else {
            mkdir $arch ||
                die "Cannot create assembly folder \"" . $arch . "\"!";
        }
        mkdir $arch."/crypto" ||
            die "Cannot create assembly folder \"" . $arch."/crypto" . "\"!";
        mkdir $arch."/crypto/aes" ||
            die "Cannot create assembly folder \"" . $arch."/crypto/aes" . "\"!";
        mkdir $arch."/crypto/modes" ||
            die "Cannot create assembly folder \"" . $arch."/crypto/modes" . "\"!";
        mkdir $arch."/crypto/sha" ||
            die "Cannot create assembly folder \"" . $arch."/crypto/sha" . "\"!";
    }

    # Read the contents of the inf file
    open( FD, "<" . $inf_file ) ||
        die "Cannot open \"" . $inf_file . "\"!";
    @inf = (<FD>);
    close(FD) ||
        die "Cannot close \"" . $inf_file . "\"!";

    foreach (@inf) {
        if (/DEFINE\s+OPENSSL_PATH\s*=\s*([a-z]+)/) {

            # We need to run Configure before we can include its result...
            $OPENSSL_PATH = $1;

            my $basedir = getcwd();

            chdir($OPENSSL_PATH) ||
                die "Cannot change to OpenSSL directory \"" . $OPENSSL_PATH . "\"";

            # Configure UEFI
            system(
                "./Configure",
                "--config=../UefiAsm.conf",
                "$uefi_config",
                "no-afalgeng",
                "no-aria",
                "no-async",
                "no-autoerrinit",
                "no-autoload-config",
                "no-bf",
                "no-blake2",
                "no-camellia",
                "no-capieng",
                "no-cast",
                "no-chacha",
                "no-cmac",
                "no-cmp",
                "no-cms",
                "no-ct",
                "no-deprecated",
                "no-des",
                "no-dgram",
                "no-dsa",
                "no-dso",
                "no-dtls",
                "no-dtls1-method",
                "no-dtls1_2-method",
                "no-dynamic-engine",
                "no-ec2m",
                "no-engine",
                "no-err",
                "no-filenames",
                "no-gost",
                "no-hw",
                "no-idea",
                "no-ktls",
                "no-makedepend",
                "no-module",
                "no-md4",
                "no-mdc2",
                "no-multiblock",
                "no-nextprotoneg",
                "no-pic",
                "no-psk",
                "no-ocb",
                "no-ocsp",
                "no-padlockeng",
                "no-poly1305",
                "no-posix-io",
                "no-rc2",
                "no-rc4",
                "no-rc5",
                "no-rfc3779",
                "no-rmd160",
                "no-scrypt",
                "no-seed",
                "no-shared",
                "no-siphash",
                "no-siv",
                "no-sm4",
                "no-sock",
                "no-srp",
                "no-srtp",
                "no-ssl",
                "no-ssl3-method",
                "no-ssl-trace",
                "no-static-engine",
                "no-stdio",
                "no-threads",
                "no-ts",
                "no-ui",
                "no-whirlpool",
                "disable-legacy",
                # OpenSSL1_1_1b doesn't support default rand-seed-os for UEFI
                # UEFI only support --with-rand-seed=none
                "--with-rand-seed=none",
                "--api=1.1.1"
                ) == 0 ||
                    die "OpenSSL Configure failed!\n";

            # Generate files per config data
            system("make build_all_generated");

            chdir($basedir) ||
                die "Cannot change to base directory \"" . $basedir . "\"";

            push @INC, $1;
            last;
        }
    }
}

#
# Retrieve file lists from OpenSSL configdata
#
use configdata qw/%unified_info/;
use configdata qw/%config/;
use configdata qw/%target/;

my @cryptofilelist = ();
my @sslfilelist = ();
my @ecfilelist = ();

foreach my $product ((@{$unified_info{libraries}},
                      @{$unified_info{engines}})) {
    my @objs = @{$unified_info{sources}->{$product}};
    while (my $o = pop @objs) {
        if ($o =~ m/\.a$/) {
            push @objs, @{$unified_info{sources}->{$o}};
            next;
        }
        foreach my $s (@{$unified_info{sources}->{$o}}) {
            # No need to add unused files in UEFI.
            # So it can reduce porting time, compile time, library size.
            next if $s =~ "crypto/rand/randfile.c";
            next if $s =~ "crypto/store/";
            next if $s =~ "crypto/err/err_all.c";
            next if $s =~ "crypto/aes/aes_ecb.c";
            next if $s =~ "providers/implementations/storemgmt/";
            next if $s =~ "ssl/ssl_txt.c";
            # Will use UEFI own provider.
            next if $s =~ "crypto/provider_predefined.c";
            next if $s =~ "providers/defltprov.c";

            #Filter out all EC related files.
            if ($s =~ "/ec/" || $s =~ "/sm2/" ||
                $s =~ "/evp/ec_support.c" ||
                $s =~ "/evp/ec_ctrl.c" ||
                $s =~ "/signature/sm2_sig.c" ||
                $s =~ "/signature/eddsa_sig.c" ||
                $s =~ "/signature/ecdsa_sig.c" ||
                $s =~ "/keymgmt/ecx_kmgmt.c" ||
                $s =~ "/keymgmt/ec_kmgmt.c" ||
                $s =~ "/exchange/ecx_exch.c" ||
                $s =~ "/exchange/ecdh_exch.c" ||
                $s =~ "/encode_decode/encode_key2blob.c" ||
                $s =~ "/asymciphers/sm2_enc.c" ||
                $s =~ "/der/der_sm2_sig.c" ||
                $s =~ "/der/der_sm2_key.c" ||
                $s =~ "/der/der_ecx_key.c" ||
                $s =~ "/der/der_ec_sig.c" ||
                $s =~ "/der/der_ec_key.c") {
                push @ecfilelist, '  $(OPENSSL_PATH)/' . $s . "\r\n";
                next;
            }
            if ($product =~ "libssl") {
                push @sslfilelist, '  $(OPENSSL_PATH)/' . $s . "\r\n";
                next;
            }
            push @cryptofilelist, '  $(OPENSSL_PATH)/' . $s . "\r\n";
        }
    }
}


#
# Update the perl script to generate the missing header files
#
my @dir_list = ();
for (sort keys %{$unified_info{dirinfo}}){
  push @dir_list,$_;
}

my $dir = getcwd();
my @files = ();
my @headers = ();
chdir ("openssl");
foreach(@dir_list){
  @files = glob($_."/*.h");
  push @headers, @files;
}
chdir ($dir);

foreach (@headers){
  next if $_ =~ "crypto/buildinf.h";
  if(/ssl/){
    push @sslfilelist, '  $(OPENSSL_PATH)/' . $_ . "\r\n";
    next;
  }
  if ($_ =~ "/ec/" || $_ =~ "/sm2/") {
    push @ecfilelist, '  $(OPENSSL_PATH)/' . $_ . "\r\n";
    next;
  }
  push @cryptofilelist, '  $(OPENSSL_PATH)/' . $_ . "\r\n";
}

#
# Update OpensslLib.inf with autogenerated file list
#
my @new_inf = ();
my $subbing = 0;
print "\n--> Updating $inf_file ... ";
foreach (@inf) {
    if ( $_ =~ "# Autogenerated files list starts here" ) {
        push @new_inf, $_, @cryptofilelist, @sslfilelist;
        $subbing = 1;
        if (defined $arch) {
            #3.0 can not push aes_core.c aes_cbc.c to src list auto, dont know why...
            push @new_inf, '  $(OPENSSL_PATH)/crypto/aes/aes_core.c' . "\r\n";
            push @new_inf, '  $(OPENSSL_PATH)/crypto/aes/aes_cbc.c' . "\r\n";
        }
        next;
    }
    if (defined $arch) {
        my $arch_asmfile_flag = "# Autogenerated " . $arch . " files list starts here";
        if ($_ =~ $arch_asmfile_flag) {
            push @new_inf, $_;
            for my $s (@asmfilelist) {
                my $file = $s;
                if ($extension =~ "nasm") {
                    $file =~ s|\.S|.nasm|i;
                } else {
                    $file =~ s|\.s|.S|;
                }
                push @new_inf, "  $arch/crypto/$file  |$compile\r\n";
            }
            $subbing = 1;
            next;
        }
    }
    if ( $_ =~ "# Autogenerated files list ends here" ) {
        push @new_inf, $_;
        $subbing = 0;
        next;
    }

    push @new_inf, $_
        unless ($subbing);
}

my $new_inf_file = $inf_file . ".new";
open( FD, ">" . $new_inf_file ) ||
    die $new_inf_file;
print( FD @new_inf ) ||
    die $new_inf_file;
close(FD) ||
    die $new_inf_file;
rename( $new_inf_file, $inf_file ) ||
    die "rename $inf_file";
print "Done!";

if (!defined $arch) {
    #
    # Update OpensslLibCrypto.inf with auto-generated file list (no libssl)
    #
    $inf_file = "OpensslLibCrypto.inf";

    # Read the contents of the inf file
    @inf = ();
    @new_inf = ();
    open( FD, "<" . $inf_file ) ||
        die "Cannot open \"" . $inf_file . "\"!";
    @inf = (<FD>);
    close(FD) ||
        die "Cannot close \"" . $inf_file . "\"!";

    $subbing = 0;
    print "\n--> Updating OpensslLibCrypto.inf ... ";
    foreach (@inf) {
        if ( $_ =~ "# Autogenerated files list starts here" ) {
            push @new_inf, $_, @cryptofilelist;
            $subbing = 1;
            next;
        }
        if ( $_ =~ "# Autogenerated files list ends here" ) {
            push @new_inf, $_;
            $subbing = 0;
            next;
        }

        push @new_inf, $_
            unless ($subbing);
    }

    $new_inf_file = $inf_file . ".new";
    open( FD, ">" . $new_inf_file ) ||
        die $new_inf_file;
    print( FD @new_inf ) ||
        die $new_inf_file;
    close(FD) ||
        die $new_inf_file;
    rename( $new_inf_file, $inf_file ) ||
        die "rename $inf_file";
    print "Done!";
}

#
# Update OpensslLibFull.inf with autogenerated file list
#
if (!defined $arch) {
    $inf_file = "OpensslLibFull.inf";
} else {
    $inf_file = "OpensslLibFullAccel.inf";
}
# Read the contents of the inf file
@inf = ();
@new_inf = ();
open( FD, "<" . $inf_file ) ||
    die "Cannot open \"" . $inf_file . "\"!";
@inf = (<FD>);
close(FD) ||
    die "Cannot close \"" . $inf_file . "\"!";
$subbing = 0;
print "\n--> Updating $inf_file ... ";
foreach (@inf) {
    if ( $_ =~ "# Autogenerated files list starts here" ) {
        push @new_inf, $_, @cryptofilelist, @sslfilelist, @ecfilelist;
        if (defined $arch) {
            #3.0 can not push aes_core.c aes_cbc.c to src list auto, dont know why...
            push @new_inf, '  $(OPENSSL_PATH)/crypto/aes/aes_core.c' . "\r\n";
            push @new_inf, '  $(OPENSSL_PATH)/crypto/aes/aes_cbc.c' . "\r\n";
        }
        $subbing = 1;
        next;
    }
    if (defined $arch) {
        my $arch_asmfile_flag = "# Autogenerated " . $arch . " files list starts here";
        if ($_ =~ $arch_asmfile_flag) {
            push @new_inf, $_;
            for my $s (@asmfilelist) {
                my $file = $s;
                if ($extension =~ "nasm") {
                    $file =~ s|\.S|.nasm|i;
                } else {
                    $file =~ s|\.s|.S|;
                }
                push @new_inf, "  $arch/crypto/$file  |$compile\r\n";
            }
            $subbing = 1;
            next;
        }
    }
    if ( $_ =~ "# Autogenerated files list ends here" ) {
        push @new_inf, $_;
        $subbing = 0;
        next;
    }

    push @new_inf, $_
        unless ($subbing);
}

$new_inf_file = $inf_file . ".new";
open( FD, ">" . $new_inf_file ) ||
    die $new_inf_file;
print( FD @new_inf ) ||
    die $new_inf_file;
close(FD) ||
    die $new_inf_file;
rename( $new_inf_file, $inf_file ) ||
    die "rename $inf_file";
print "Done!";

#
# Copy generated files
#
for my $file (map { s/\.in//; $_ } glob($OPENSSL_PATH . "/include/*/*.h.in")) {
    my $dest = $file;
    $dest =~ s|.*/include/|./OpensslGen/|;
    print "\n--> Duplicating $file into $dest ... ";
    system("perl -pe 's/\\n/\\r\\n/' < $file > $dest") == 0
        or die "Cannot copy $file !";
}

my @hdrs = ();
push @hdrs, glob($OPENSSL_PATH . "/providers/common/include/prov/[a-z]*.h");
push @hdrs, glob($OPENSSL_PATH . "/providers/implementations/include/prov/[a-z]*.h");
for my $file (@hdrs) {
    my $dest = $file;
    $dest =~ s|.*/include/|./OpensslGen/|;
    print "\n--> Duplicating $file into $dest ... ";
    system("perl -pe 's/\\n/\\r\\n/' < $file > $dest") == 0
        or die "Cannot copy $file !";
}

for my $file (map { s/\.in//; $_ } glob($OPENSSL_PATH . "/providers/common/der/*.c.in")) {
    next unless -f $file;
    my $dest = $file;
    $dest =~ s|.*/|./OpensslGen/|;
    print "\n--> Duplicating $file into $dest ... ";
    system("perl -pe 's/\\n/\\r\\n/' < $file > $dest") == 0
        or die "Cannot copy $file !";
}

for my $file (@asmfilelist) {
    my $dest = "./".$arch."/crypto/".$file;
    my $source = $file;
    $source =~ s|.*/||g;
    $source =~ s|\.S|.pl|i;
    copy_license_header("./openssl/crypto/".$file, $source);
    if ($extension =~ "nasm") {
        $dest =~ s|\.S|.nasm|i;
    } else {
        $dest =~ s|\.s|.S|;
    }
    print "\n--> Duplicating $file into $dest ... ";
    system("perl -pe 's///' < './openssl/crypto/'$file > $dest") == 0
        or die "Cannot copy $file !";
}

print "\nProcessing Files Done!\n";

# cleanup
system("make -C $OPENSSL_PATH clean");

exit(0);


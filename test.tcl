puts "Shell [info name] version [info patchlevel]"
puts "[concat $::tcl_platform(os) $::tcl_platform(osVersion)]"

if {[info command memory] ne ""} {
    puts "Memory validation mode"
    memory init on
    memory validate on
}

package require tcltest
namespace import tcltest::*

package require registry

# WinNT3.1 - HKEY_CURRENT_USER\\Printers\\Connections
set k {HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Print\Printers}
set printerNames [lsort [registry keys $k]]
for {set i 0} {true} {incr i} {
    set unknownName printer$i
    if {[lsearch -exact $printerNames $unknownName] < 0} break
}

#set k {HKEY_CURRENT_USER\Printers\Settings}
set k {HKEY_CURRENT_USER\Software\Microsoft\Windows NT\CurrentVersion\Windows}
set defaultDevice [lindex [split [registry get $k Device] ","] 0]
set defaultDriver [lindex [split [registry get $k Device] ","] 1]
set defaultPort [lindex [split [registry get $k Device] ","] 2]
#set defaultName [registry keys $k]

#set k {HKEY_CURRENT_USER\Software\Microsoft\Windows NT\CurrentVersion\Devices}
#set defaultName [lindex [registry keys $k] 0]
set testingDevice "Microsoft XPS Document Writer"
#set testingDevice "Microsoft Print to PDF"
#set testingDevice $defaultDevice
set testingOutput [file join [tcltest::temporaryDirectory] test.tmp]
set testingFont Courier

# fconfigure stdout -encoding cp866
puts "Registry devices: $printerNames"
puts "Registry default: $defaultDevice $defaultDriver $defaultPort"
puts "Using device: $testingDevice"

unset k

set lib {}
if {[catch {load $lib tclprinter}]} {
    foreach lib {tclprinter10g.dll tclprinter10.dll tclprinter10tg.dll tclprinter10t.dll} {
        if {[file exists $lib]} {
            if {[catch {load $lib tclprinter} err]} {
                puts "Loading library '$lib': $err"
            } else {
                puts "Using library $lib"
                break
            }
        } 
    }
} else {
    puts "Using static library"
}

set version [package present tclprinter]
if {$version eq ""} {
    puts "Cant load any tclprinter library"
    exit 1
} else {
    puts "Using tclprinter library version $version"
}

proc striperror {errorcode errormessage} {
   list [lrange $errorcode 0 1] [string range $errormessage 0 [string first ":" $errormessage]]
}

proc reopen {command {force false}} {
   set reopen $force
   if {$::tcl_platform(os) eq "Windows NT"} {
        if {$::tcl_platform(osVersion) > 6.1} {
            set reopen true
        }
   }
   if {$reopen} {
       set n [$command info name]
       set p [$command info protocol] 
       $command close
       printer open -$p -name $n $command
   }
   return $command
}

set nodialogs 0
set noxpscheck [catch {package require zipvfs; package require tdom}]

proc viewXps {file} {
    set l {}
    if {$::noxpscheck == 0} {
        vfs::zip::Mount $file $file
        dom parse [tDOM::xmlReadFile $file/Documents/1/Pages/1.fpage] doc
        vfs::unmount $file
        $doc documentElement root
        foreach a [$root selectNodes //@UnicodeString] {
            set l [concat $l [lindex $a 1]]
        }
        $doc delete
    }
    return $l
}

debug 0
verbose tbe
testConstraint raw 1
testConstraint gdi 1
testConstraint xps [expr {!$noxpscheck}]
testConstraint job 1

if {$tcl_version >= 8.6} {
    set codeWrongArgs {TCL WRONGARGS}
} else {
    set codeWrongArgs {NONE}
}

if {$tcl_version >= 8.5} {
    set codeValueNumber {TCL VALUE NUMBER}
    proc codeLookupIndex args {concat TCL LOOKUP INDEX $args}
} else {
    set codeValueNumber {NONE}
    proc codeLookupIndex args {return NONE}
}

if {$nodialogs} {
    set printerUsage [list $codeWrongArgs {wrong # args: should be "printer open ?-gdi|-raw? ?-default|-name printername? ?command?"}]
} else {
    set printerUsage [list $codeWrongArgs {wrong # args: should be "printer open ?-gdi|-raw? ?-default|-printdialog|-setupdialog|-name printername? ?command?"}]
}

catch {file delete $testingOutput}

test test-1.1.0 {no params at all} {
    catch {printer} result
    list $errorCode $result
} [list $codeWrongArgs {wrong # args: should be "printer command"}]

test test-1.1.1 {unknown param} {
    catch {printer unknown} result
    list $errorCode $result
} [list [codeLookupIndex subcommand unknown] {bad subcommand "unknown": must be default, names, print, write, or open}]

test test-1.1.2 {default printer (compare to registry)} {
    printer default
} $defaultDevice

test test-1.1.3 {known printers (compare to registry)} {
    lsort [printer names]
} $printerNames

test test-1.2.1 {open syntax error} {gdi} {
    catch {printer open -default xxx yyy} result
    list $errorCode $result
} $printerUsage

test test-1.2.2 {open syntax error} {gdi} {
    catch {printer open -name xxx yyy zzz} result
    list $errorCode $result
} $printerUsage

test test-1.2.3 {open syntax error} {gdi} {
    catch {printer open -name} result
    list $errorCode $result
} {NONE {printer name required for -name}}

test test-1.2.4 {open command with -} {gdi} {
    catch {printer open -prn} result
    list $errorCode $result
} $printerUsage

test test-1.2.5 {open command with -} {gdi} {
    catch {printer open -default -prn} result
    list $errorCode $result
} $printerUsage

test test-1.2.6 {open/close default successfully} {gdi} {
    list [printer open -default prn] [prn info name] [prn close]
} [list prn $defaultDevice {}]

test test-1.2.7 {open/close by name} {gdi} {
    list [printer open -name $testingDevice prn] [prn info name] [prn close]
} [list prn $testingDevice {}]

test test-1.2.8 {open/close by name, autonamed} {gdi} {
    set p [printer open -name $testingDevice]
    $p close
    set p
} {printer0}

test test-1.2.9 {open/close by name, check command name} {gdi} {
    proc printer2 {} {}
    set l [list \
        [printer open -name $testingDevice] \
        [printer open -name $testingDevice]]
    foreach c $l {$c close}
    rename printer2 {}
    set l
} {printer1 printer3}

test test-1.2.10 {open unknown error} {gdi} {
    catch {printer open -name $unknownName} result
    striperror $errorCode $result
} {{WINDOWS 1801} {can't open printer:}}

test test-1.3.0 {open by name} {gdi} {
    list [printer open -name $testingDevice prn] [info commands prn]
} {prn prn}

test test-1.3.1 {command noargs error} {gdi} {
    catch {prn} result
    list $errorCode $result
} [list $codeWrongArgs {wrong # args: should be "prn subcommand"}]

test test-1.3.2 {command unknown} {gdi} {
    catch {prn unknown} result
    list $errorCode $result
} [list [codeLookupIndex subcommand unknown] {bad subcommand "unknown": must be info, start, end, startpage, endpage, place, print, frame, fill, rectangle, round, ellipse, close, or abort}]

test test-1.3.3 {command info unknown} {gdi} {
    catch {prn info} result
    list $errorCode $result
} [list $codeWrongArgs {wrong # args: should be "prn info name"}]

test test-1.3.4 {command info name} {gdi} {
    prn info name
} $testingDevice

test test-1.3.5 {x resolution} {gdi} {
    expr [prn info width] / [prn info pwidth]
} 23

test test-1.3.6 {y resolution} {gdi} {
    expr [prn info height] / [prn info pheight]
} 23

test test-1.4.0.1 {open/start/end/close command} {gdi} {
   list [printer open -name $testingDevice prn] [prn start -output $testingOutput] [prn end] [prn close]
} {prn {} {} {}}

test test-1.4.0.2 {open/start/end command} {gdi} {
   list [printer open -name $testingDevice prn] [prn start -output $testingOutput] [prn end] 
} {prn {} {}}

test test-1.4.0.3 {reopen/start/end command - check for reopen works}  {gdi} {
   list [reopen prn] [prn start -output $testingOutput] [prn end]
} {prn {} {}}

test test-1.4.0.4 {open command for testing} {gdi} {
  printer open -name $testingDevice prn
} {prn}

test test-1.4.1 {command start job bad arg} {gdi} {
    catch {prn start unknown} result
    list $errorCode $result
} [list $codeWrongArgs {wrong # args: should be "prn start ?-document name? ?-output name?"}]

test test-1.4.2 {command start job bad args number} {gdi} {
    catch {prn start -document} result
    list $errorCode $result
} {NONE {missing option value for -document}}

test test-1.4.3 {command start job bad args number} {gdi} {
    catch {prn start -output} result
    list $errorCode $result
} {NONE {missing option value for -output}}

test test-1.4.4.1 {command stop job not started} {gdi} {
    catch {prn end} result
    list $errorCode $result
} {NONE {document is not started}}

test test-1.4.4.2 {command abort job not started} {gdi} {
    list [prn abort] [info commands prn]
} {{} {}}

test test-1.4.4.3 {reopen command for testing after abort} {gdi} {
  printer open -name $testingDevice prn
} {prn}

test test-1.4.5 {command stop job bad arg} {gdi} {
    reopen prn
    prn start -output $testingOutput
    catch {prn end unknown} result
    prn end
    list $errorCode $result
} [list $codeWrongArgs {wrong # args: should be "prn end"}]

test test-1.4.6 {command double start job} {gdi} {
    reopen prn
    prn start -output $testingOutput
    catch {prn start  -output $testingOutput} result
    prn end
    list $errorCode $result
} {NONE {document is already started}}

test test-1.5.1 {command start page bad arg} {gdi} {
    reopen prn
    prn start -output $testingOutput
    catch {prn startpage unknown} result
    prn end
    list $errorCode $result
} [list $codeWrongArgs {wrong # args: should be "prn startpage"}]

test test-1.5.2 {command endpage w/o startpage} {gdi} {
    reopen prn
    prn start -output $testingOutput
    catch {prn endpage} result
    prn end
    list $errorCode $result
} {NONE {page is not started}}

test test-1.5.3 {command stop page bad arg} {gdi} {
    reopen prn
    prn start -output $testingOutput
    prn startpage
    catch {prn endpage unknown} result
    prn end
    list $errorCode $result
} [list $codeWrongArgs {wrong # args: should be "prn endpage"}]

test test-1.6.1 {command start/stop job succesfully} {gdi} {
    list \
        [reopen prn] \
        [prn start  -document TEST -output $testingOutput] \
        [prn end] \
        [file delete $testingOutput] \
} [list prn {} {} {}]

test test-1.6.2 {command start/abort job succesfully} {gdi} {
    list \
        [reopen prn] \
        [prn start  -document TEST -output $testingOutput] \
        [prn abort] \
        [file delete $testingOutput] \
        [info commands prn]
} [list prn {} {} {} {}]

test test-1.6.2.1 {reopen printer} {gdi} {
    printer open -name $testingDevice prn
} {prn}

test test-1.6.3 {command start/stop unicode named job succesfully} {gdi} {
    list [reopen prn] \
        [prn start -document ТЕСТ -output $testingOutput] \
        [prn end] \
        [file delete $testingOutput]
} [list prn {} {} {}]

test test-1.6.4 {command start/stop job duplicated} {gdi} {
    list [reopen prn] \
       [prn start -output $testingOutput] \
       [catch {prn start -output $testingOutput} result] \
       $result \
       [prn end]
} {prn {} 1 {document is already started} {}}

test test-1.6.5 {command endpage w/o start in job} {gdi} {
    reopen prn
    prn start -output $testingOutput
    catch {prn endpage} result
    prn end
    list $errorCode $result
} {NONE {page is not started}}

test test-1.6.6 {command start/stop job/page succesfully} {gdi} {
    list [reopen prn] [prn start -output $testingOutput] [prn startpage] [prn endpage] [prn end] 
} {prn {} {} {} {}}

test test-1.7.1 {start page for print tests} {gdi} {
    reopen prn
    prn start -output $testingOutput
    prn startpage
} {}

test test-1.7.2.1 {print syntax error} {gdi} {
    catch {prn print} result
    list $errorCode $result
} [list $codeWrongArgs {wrong # args: should be "prn print ?-margins rectspec? ?-font fontspec? ?-wordwrap? text"}]

test test-1.7.2.2 {print -margins missing syntax error} {gdi} {
    catch {prn print -margins} result
    list $errorCode $result
} {NONE {missing option value for -margins}}

test test-1.7.2.3 {print -margins nonint syntax error} {gdi} {
    catch {prn print -margins x test} result
    list $errorCode $result
} [list $codeValueNumber {expected integer but got "x", invalid option value for -margins}]

test test-1.7.2.4 {print -margins syntax error} {gdi} {
    catch {prn print -margins {1 2 3 4 5}} result
    list $errorCode $result
} {NONE {too many elements, invalid option value for -margins}}

test test-1.7.2.5 {print -font missing syntax error} {gdi} {
    catch {prn print -font} result
    list $errorCode $result
} {NONE {missing option value for -font}}

test test-1.7.2.6 {print -font unknown} {gdi} {
    catch {prn print -font {unknown} data} result
    list $errorCode 
} {NONE}

test test-1.7.2.7 {print -font syntax error} {gdi} {
    catch {prn print -font {{} unknown} data} result
    list $errorCode $result
} {NONE {unknown property "unknown", invalid option value for -font}}

test test-1.7.2.8 {print -margins -font syntax error} {gdi} {
    catch {prn print -margins -font} result
    list $errorCode $result
} [list $codeValueNumber {expected integer but got "-font", invalid option value for -margins}]

test test-1.7.3.1 {place syntax error} {gdi} {
    catch {prn place} result
    list $errorCode $result
} [list $codeWrongArgs {wrong # args: should be "prn place ?-rect rectspec? ? ?-align flags? ?-font fontspec? ?-calcrect? text"}]

test test-1.7.3.2 {place -rect missing syntax error} {gdi} {
    catch {prn place -rect} result
    list $errorCode $result
} {NONE {missing option value for -rect}}

test test-1.7.3.3 {place -rect nonint syntax error} {gdi} {
    catch {prn place -rect x test} result
    list $errorCode $result
} [list $codeValueNumber {expected integer but got "x", invalid option value for -rect}]

test test-1.7.3.4 {place -rect syntax error} {gdi} {
    catch {prn place -rect {1 2 3 4 5}} result
    list $errorCode $result
} {NONE {too many elements, invalid option value for -rect}}

test test-1.7.3.5 {place -font missing syntax error} {gdi} {
    catch {prn place -font} result
    list $errorCode $result
} {NONE {missing option value for -font}}

test test-1.7.3.6 {place -font syntax error} {gdi} {
    catch {prn place -font {unknown} data} result
    list $errorCode 
} {NONE}

test test-1.7.3.7 {place -font syntax error} {gdi} {
    catch {prn place -font {{} unknown} data} result
    list $errorCode $result
} {NONE {unknown property "unknown", invalid option value for -font}}

test test-1.7.3.8 {place -font syntax error} {gdi} {
    catch {prn place -font} result
    list $errorCode $result
} {NONE {missing option value for -font}}

test test-1.7.3.9 {place -rect -font syntax error} {gdi} {
    catch {prn place -rect -font} result
    list $errorCode $result
} [list $codeValueNumber {expected integer but got "-font", invalid option value for -rect}]

test test-1.7.9 {end print testing page} {gdi} {
    prn endpage
    prn end
} {}

set s blablabla

test test-1.8.1 {document write} {gdi} {
    list \
        [reopen prn] \
        [prn start -document test -output $testingOutput] \
        [prn startpage] \
        [prn print -font $testingFont $s] \
        [prn endpage] \
        [prn end]
} [list prn {} {} [string length $s] {} {}]

test test-1.8.2 {document check} {gdi xps} {
    viewXps $testingOutput
} $s

unset s

set s ЙЦУКЕНйцукен№ЁёІіЇї

test test-1.8.3 {document write unicode} {gdi} {
    list \
        [reopen prn] \
        [prn start -document test -output $testingOutput] \
        [prn startpage] \
        [prn print -font $testingFont $s] \
        [prn endpage] \
        [prn end]
} [list prn {} {} [string length $s] {} {}]

test test-1.8.4 {document check} {gdi xps} {
   viewXps $testingOutput
} $s

unset s

set s blablabla

test test-1.8.5 {document write} {gdi} {
    printer print -name $testingDevice -document test -output $testingOutput $s
} [string length $s]

test test-1.8.6 {document check} {gdi xps} {
    viewXps $testingOutput
} $s

unset s

set s ЙЦУКЕНйцукен№ЁёІіЇї

test test-1.7.4 {document write unicode} {gdi} {
    printer print -name $testingDevice -document test -output $testingOutput -font $testingFont $s
} [string length $s]

test test-1.8.7 {document check unicode} {gdi xps} {
    viewXps $testingOutput
} $s

unset s

foreach {h w} {{}} break

test test-1.9.1.0 {get default font metrics} {gdi} {
    prn start -output nul
    prn startpage
    foreach {x1 y1 x2 y2} [prn place -calcrect x] break
    set h [expr ($y2-$y1)]
    set w [expr ($x2-$x1)]
    prn endpage
    prn end
} {}

test test-1.9.1.1 {check default font metrics} {gdi} {
  list [prn info cheight] [prn info cwidth]
} [list $h $w]

unset h w

foreach {h w} {{}} break

test test-1.9.2.0 {get courier font metrics} {gdi} {
    prn start -output nul
    prn startpage
    foreach {x1 y1 x2 y2} [prn place -font courier -calcrect x] break
    set h [expr ($y2-$y1)]
    set w [expr ($x2-$x1)]
    prn endpage
    prn end
} {}

test test-1.9.2.1 {check courier font metrics} {gdi} {
  list [prn info cheight] [prn info cwidth]
} [list $h $w]

unset h w

foreach {h w} {{}} break

test test-1.9.3.0 {get arial 20x5 font metrics} {gdi} {
    prn start -output nul
    prn startpage
    foreach {x1 y1 x2 y2} [prn place -font {arial 20 5 bold italic} -calcrect x] break
    set h [expr ($y2-$y1)]
    set w [expr ($x2-$x1)]
    prn endpage
    prn end
} {}

test test-1.9.3.1 {check courier font metrics} {gdi} {
  list [prn info cheight] [prn info cwidth]
} [list $h $w]

unset h w

test test-2.9 {} {gdi} {
    list [prn close] [info commands prn]
} {{} {}}

test test-3.0 {} {gdi} {
    list [printer open -default prn1] [printer open -default prn2]
} {prn1 prn2}

test test-3.1 {} {gdi} {
    list [prn1 info name] [prn2 info name]
} [list $defaultDevice $defaultDevice]

test test-3.9 {} {gdi} {
    list [prn1 close] [prn2 close]
} {{} {}}

test test-5.2.1 {raw open syntax error} {raw} {
    catch {printer open -raw -default xxx yyy} result
    list $errorCode $result
} $printerUsage

test test-5.2.2 {raw open syntax error} {raw} {
    catch {printer open -raw -name xxx yyy zzz} result
    list $errorCode $result
} $printerUsage

test test-5.2.3 {raw open syntax error} {raw} {
    catch {printer open -raw -name} result
    list $errorCode $result
} {NONE {printer name required for -name}}

test test-5.2.4 {raw open unknown error} {raw} {
    catch {printer open -raw -name $unknownName} result
    striperror $errorCode $result
} {{WINDOWS 1801} {can't open printer:}}

test test-5.2.6 {raw open/close default successfully} {raw} {
    list [printer open -raw -default prn] [info commands prn] [prn info name] [prn close]
} [list prn prn $defaultDevice {}]

test test-5.2.7 {raw open/close by name} {raw} {
    list [printer open -raw -name $testingDevice prn] [info commands prn] [prn close]
} {prn prn {}}

test test-5.3.0 {raw open by name} {raw} {
    list [printer open -raw -name $testingDevice prn] [info commands prn]
} {prn prn}

test test-5.3.1 {raw command noargs error} {raw} {
    catch {prn} result
    list $errorCode $result
} [list $codeWrongArgs {wrong # args: should be "prn subcommand"}]

test test-5.3.2 {raw command unknown} {raw} {
    catch {prn unknown} result
    list $errorCode $result
} [list [codeLookupIndex subcommand unknown] {bad subcommand "unknown": must be info, status, document, start, end, startpage, endpage, write, close, or abort}]

test test-5.3.3 {raw command info unknown} {raw} {
    catch {prn info} result
    list $errorCode $result
} [list $codeWrongArgs {wrong # args: should be "prn info name"}]

test test-5.3.4 {raw command info name} {raw} {
    prn info name
} $testingDevice

test test-5.4.0.0 {raw open/start/end/close command} {raw} {
   list [printer open -raw -name $testingDevice prn] [prn start -output $testingOutput] [prn end] [prn close] [info commands prn] [file delete $testingOutput]
} {prn {} {} {} {} {}}

test test-5.4.0.1 {raw open/start/abort command} {raw} {
   list [printer open -raw -name $testingDevice prn] [prn start -output $testingOutput] [prn abort] [info commands prn] [file delete $testingOutput]
} {prn {} {} {} {}}

test test-5.4.0.2 {raw open/start/end command} {raw} {
   list [printer open -raw -name $testingDevice prn] [prn info protocol] [prn start -output $testingOutput] [prn end] [file delete $testingOutput]
} {prn raw {} {} {}}

test test-5.4.0.3 {raw reopen/start/end command - check for reopen works} {raw} {
   list [reopen prn] [prn info protocol] [prn start -output $testingOutput] [prn end] [file delete $testingOutput]
} {prn raw {} {} {}}

test test-5.4.0.4 {raw open command for testing} {raw} {
  printer open -raw -name $testingDevice prn
} {prn}

test test-5.4.1 {raw command start job bad arg} {raw} {
    catch {prn start unknown} result
    list $errorCode $result
} [list $codeWrongArgs {wrong # args: should be "prn start ?-document name? ?-output name?"}]

test test-5.4.2 {raw command start job bad args number} {raw} {
    catch {prn start -document} result
    list $errorCode $result
} {NONE {missing option value for -document}}

test test-5.4.3 {raw command start job bad args number} {raw} {
    catch {prn start -output} result
    list $errorCode $result
} {NONE {missing option value for -output}}

test test-5.4.4.1 {raw command stop job not started} {raw} {
    catch {prn end} result
    list $errorCode $result
} {NONE {document is not started}}

#test test-5.4.4.2 {raw command abort job not started} {
#    catch {prn abort} result
#    list $errorCode $result
#} {NONE {document is not started}}

test test-5.4.5 {raw command stop job bad arg} {raw} {
    reopen prn
    prn start -output $testingOutput
    catch {prn end unknown} result
    prn end
    file delete $testingOutput
    list $errorCode $result
} [list $codeWrongArgs {wrong # args: should be "prn end"}]

test test-5.4.6 {raw command double start job} {raw} {
    reopen prn
    prn start -output $testingOutput
    catch {prn start  -output $testingOutput} result
    prn end
    file delete $testingOutput
    list $errorCode $result
} {NONE {document is already started}}

test test-5.5.1 {raw command start page bad arg} {raw} {
    reopen prn
    prn start -output $testingOutput
    catch {prn startpage unknown} result
    prn end
    file delete $testingOutput
    list $errorCode $result
} [list $codeWrongArgs {wrong # args: should be "prn startpage"}]

test test-5.5.2 {raw command endpage w/o startpage} {raw} {
    reopen prn
    prn start -output $testingOutput
    catch {prn endpage} result
    prn end
    file delete $testingOutput
    list $errorCode $result
} {NONE {page is not started}}

test test-5.5.3 {raw command stop page bad arg} {raw} {
    reopen prn
    prn start -output $testingOutput
    prn startpage
    catch {prn endpage unknown} result
    prn end
    file delete $testingOutput
    list $errorCode $result
} [list $codeWrongArgs {wrong # args: should be "prn endpage"}]

test test-5.6.1 {raw command start/stop job succesfully} {raw} {
    list \
        [reopen prn] \
        [prn start  -document TEST -output $testingOutput] \
        [prn end] \
        [file delete $testingOutput]
} [list prn {} {} {}]

test test-5.6.2 {raw command start/abort job succesfully} {raw} {
    list \
        [reopen prn] \
        [prn start -document TEST -output $testingOutput] \
        [prn abort] \
        [file delete $testingOutput] \
        [info commands prn]
} [list prn {} {} {} {}]

test test-5.6.2.1 {reopen printer} {raw} {
    printer open -raw -name $testingDevice prn
} {prn}

test test-5.6.3 {raw command start/stop unicode named job succesfully} {raw} {
    list [reopen prn] \
        [prn start -document ТЕСТ -output $testingOutput] \
        [prn end] \
        [file delete $testingOutput]
} [list prn {} {} {}]

test test-5.6.4 {command start/stop job duplicated} {raw} {
    list [reopen prn] \
        [prn start -output $testingOutput] \
        [catch {prn start -output $testingOutput} result] \
        $result \
        [prn end] \
        [file delete $testingOutput]
} {prn {} 1 {document is already started} {} {}}

test test-5.6.5 {raw command endpage w/o start in job} {raw} {
    reopen prn
    prn start -output $testingOutput
    catch {prn endpage} result
    prn end
    file delete $testingOutput
    list $errorCode $result
} {NONE {page is not started}}

test test-5.6.6 {-raw command start/stop job/page succesfully} {raw} {
    list [reopen prn] [prn start -output $testingOutput] [prn startpage] [prn endpage] [prn end] [file delete $testingOutput]
} {prn {} {} {} {} {}}

test test-5.7.1 {raw write} {raw} {
    reopen prn
    catch {prn write blablabla} result
    striperror $errorCode $result
} {{WINDOWS 3003} {error writing data:}}

set s blablabla

test test-5.7.2 {raw document write} {raw} {
    list \
        [reopen prn] \
        [prn start -document test -output $testingOutput] \
        [prn write $s] \
        [prn end]
} [list prn {} [string length $s] {}]

test test-5.7.2.1 {raw document check} {raw} {
    viewFile $testingOutput
} $s

unset s

set s ЙЦУКЕНйцукен№ЁёІіЇї

test test-5.7.2.2 {raw document write unicode} {raw} {
    list \
        [reopen prn] \
        [prn start -document test -output $testingOutput] \
        [prn write [encoding convertto $s]] \
        [prn end]
} [list prn {} [string length $s] {}]

test test-5.7.2.3 {raw document check} {raw} {
    viewFile $testingOutput
} $s

unset s

test test-5.7.2.4 {close printer} {raw} {
    prn close
} {}

set s blablabla

test test-5.7.3 {raw document write} {raw} {
    printer write -name $testingDevice -document test -output $testingOutput $s
} [string length $s]

test test-5.7.3.1 {raw document check} {raw} {
    viewFile $testingOutput
} $s

unset s

set s ЙЦУКЕНйцукен№ЁёІіЇї

test test-5.7.4 {raw document write unicode} {raw} {
    printer write -name $testingDevice -document test -output $testingOutput [encoding convertto $s]
} [string length $s]

test test-5.7.4.1 {raw document check unicode} {raw} {
    viewFile $testingOutput
} $s

unset s

test test-6.0.1 {create printer} {job} {
    printer open -raw -name $testingDevice prn
} {prn}

test test-6.0.2 {clear queue} {job} {
    foreach i [prn document select id -document test-6.0.1] {
        if {[catch {prn document delete $i}]} {
            prn document cancel $i
        }
    }
    prn document select document -document test-6.0.1
} {}

test test-6.0.3 {create document, check id} {job} {
    prn start -document test-6.0.1 -output nul
    lsearch [prn document select id -document test-6.0.1] [prn document id]
} {0}

test test-6.0.4 {pause document, check status} {job} {
    prn document pause
    lsearch [prn document status] "paused"
} {0}

test test-6.0.5 {cleanup} {job} {
    if {[catch {prn document delete}]} {
        prn document cancel
    }
    prn abort
} {}

test test-6.1.1 {create monitor} {job} {
    printer open -raw -name $testingDevice pmon
} {pmon}

test test-6.1.2 {clear queue} {job} {
    foreach i [pmon document select id -document test-6.1.3-?] {
        pmon document delete $i
    }
    pmon document select document
} {}

test test-6.1.3 {create queue} {job} {
    printer open -name $testingDevice prn1
    printer open -name $testingDevice prn2
    prn1 start -document test-6.1.3-1 -output nul
    prn2 start -document test-6.1.3-2 -output nul
    lsort [pmon document select document -document test-6.1.3-?]
} {test-6.1.3-1 test-6.1.3-2}

test test-6.1.4 {pause queue} {job} {
    set l {}
    foreach i [pmon document select id -document test-6.1.*] {
        pmon document pause $i        
    }
    foreach i [pmon document select id -document test-6.1.*] {
        lappend l [expr {[lsearch -exact [pmon document status $i] "paused"] >= 0}]
    }
    set l
} {1 1}

test test-6.1.5 {clear queue} {job} {
    foreach i [pmon document select id -document test-6.1.*] {
        pmon document delete $i
    }
    prn1 abort
    prn2 abort
    after 1000
    pmon document select document -document test-6.1.3-?
} {}

test test-6.1.6.1 {invalid paramener for monitor} {job} {
    catch {pmon document xxx} result
    list $errorCode $result
} [list [codeLookupIndex request xxx] {bad request "xxx": must be id, status, cancel, delete, pause, resume, restart, retain, or release}]

test test-6.1.6.2 {invalid id for monitor} {job} {
    pmon document id 1
} {1}

test test-6.1.6.3 {empty id for monitor} {job} {
    catch {pmon document id {}} result
    list $errorCode $result
} [list $codeValueNumber {expected integer but got ""}]

test test-6.1.6.4 {negative id for monitor} {job} {
    pmon document id -1
} {-1}

test test-6.1.6.5 {long id for monitor} {job} {
    pmon document id 1234567890
} {1234567890}

test test-6.1.6.6 {get id for not started document} {job} {
    pmon document id
} {0}

test test-6.1.6.6 {invalid id for monitor} {job} {
    catch {pmon document status -1} result
    striperror $errorCode $result
} {{WINDOWS 87} {error processing document:}}

test test-6.1.6.7 {invalid id for monitor} {job} {
    catch {pmon document status 1234567890} result
    striperror $errorCode $result
} {{WINDOWS 87} {error processing document:}}

test test-6.1.6.8 {invalid id for monitor} {job} {
    catch {pmon document pause -1} result
    striperror $errorCode $result
} {{WINDOWS 87} {error processing document:}}

test test-6.1.6.9 {invalid id for monitor} {job} {
    catch {pmon document pause 1234567890} result
    striperror $errorCode $result
} {{WINDOWS 87} {error processing document:}}

catch {file delete $testingOutput}

cleanupTests 

puts "end of tests"

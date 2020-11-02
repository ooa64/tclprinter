puts [info name]

puts [concat $::tcl_platform(os) $::tcl_platform(osVersion)]

if {1 && [info command memory] ne ""} {
    puts "memory validation mode"
    memory init on
    memory validate on
}

package require tcltest
namespace import tcltest::*

#debug 3

package require registry

# WinNT3.1 - HKEY_CURRENT_USER\\Printers\\Connections
set k {HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Print\Printers}
set printerNames [lsort [registry keys $k]]

#set k {HKEY_CURRENT_USER\Printers\Settings}
set k {HKEY_CURRENT_USER\Software\Microsoft\Windows NT\CurrentVersion\Windows}
set defaultDevice [lindex [split [registry get $k Device] ","] 0]
set defaultDriver [lindex [split [registry get $k Device] ","] 1]
set defaultPort [lindex [split [registry get $k Device] ","] 2]
#set defaultName [registry keys $k]

#set k {HKEY_CURRENT_USER\Software\Microsoft\Windows NT\CurrentVersion\Devices}
#set defaultName [lindex [registry keys $k] 0]
set testingDevice "Microsoft XPS Document Writer"
#et testingDevice $defaultDevice
set testingOutput [file join [tcltest::temporaryDirectory] test.tmp]

fconfigure stdout -encoding cp866
puts "registry devices: $printerNames"
puts "registry default: $defaultDevice $defaultDriver $defaultPort"
puts "testing device: $testingDevice"

unset k

set lib {}
if {[catch {load $lib Tclprinter} err]} {
    puts "loading '$lib': $err"
    set lib tclprinter10g.dll
    if {[catch {load $lib Tclprinter}]} {
        puts "loading '$lib': $err"
        set lib tclprinter10.dll
        if {[catch {load $lib Tclprinter}]} {
            puts "loading '$lib': $err"
            exit 1
        }
    }
}

proc reopen {command {force false}} {
   set reopen $force
   if {$::tcl_platform(os) eq "Windows NT"} {
        switch $::tcl_platform(osVersion) {
            6.1 {
                set reopen true
            }
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

puts "loaded '$lib'"

catch {file delete $testingOutput}

set printerUsage {NONE {wrong # args: should be "printer open ?-gdi|-raw? ?-default|-select|-name printername? ?command?"}}

test test-1.1.0 {no params at all} {
    catch {printer} result
    list $errorCode $result
} {NONE {wrong # args: should be "printer command"}}

test test-1.1.1 {unknown param} {
    catch {printer unknown} result
    list $errorCode $result
} {NONE {bad command "unknown": must be names, print, write, or open}}

test test-1.1.2 {known printers (compare to registry)} {
    lsort [printer names]
} $printerNames

test test-1.2.1 {open syntax error} {
    catch {printer open -default xxx yyy} result
    list $errorCode $result
} $printerUsage

test test-1.2.2 {open syntax error} {
    catch {printer open -name xxx yyy zzz} result
    list $errorCode $result
} $printerUsage

test test-1.2.3 {open syntax error} {
    catch {printer open -name} result
    list $errorCode $result
} {NONE {printer name required for -name}}

#test test-1.2.4 {open syntax error} {
#    catch {printer open -unknown} result
#    list $errorCode $result
#} $printerUsage

test test-1.2.5 {open command with -} {
    list [printer open -prn] [-prn close]
} {-prn {}}

test test-1.2.6 {open/close default successfully} {
    list [printer open -default prn] [info commands prn] [prn close]
} {prn prn {}}

test test-1.2.7 {open/close by name} {
    list [printer open -name $testingDevice prn] [info commands prn] [prn close]
} {prn prn {}}

test test-1.2.8 {open/close by name, autonamed} {
    set p [printer open -name $testingDevice]
    $p close
    set p
} {printer0}

test test-1.2.9 {open/close by name, check command name} {
    proc printer2 {} {}
    set l [list \
        [printer open -name $testingDevice] \
        [printer open -name $testingDevice]]
    foreach c $l {$c close}
    rename printer2 {}
    set l
} {printer1 printer3}

test test-1.3.0 {open by name} {
    list [printer open -name $testingDevice prn] [info commands prn]
} {prn prn}

test test-1.3.1 {command noargs error} {
    catch {prn} result
    list $errorCode $result
} {NONE {wrong # args: should be "prn command"}}

test test-1.3.2 {command unknown} {
    catch {prn unknown} result
    list $errorCode $result
} {NONE {bad command "unknown": must be info, start, abort, end, startpage, endpage, place, print, frame, fill, rectangle, round, ellipse, or close}}

test test-1.3.3 {command info unknown} {
    catch {prn info} result
    list $errorCode $result
} {NONE {wrong # args: should be "prn info name"}}

test test-1.3.4 {command info name} {
    prn info name
} $testingDevice

test test-1.4.0.1 {open/start/end/close command} {
   list [printer open -name $testingDevice prn] [prn start -output $testingOutput] [prn end] [prn close]
} {prn {} {} {}}

test test-1.4.0.2 {open/start/end command} {
   list [printer open -name $testingDevice prn] [prn start -output $testingOutput] [prn end] 
} {prn {} {}}

test test-1.4.0.3 {reopen/start/end command - check for reopen works} {
   list [reopen prn] [prn start -output $testingOutput] [prn end]
} {prn {} {}}

test test-1.4.0.4 {open command for testing} {
  printer open -name $testingDevice prn
} {prn}

test test-1.4.1 {command start job bad arg} {
    catch {prn start unknown} result
    list $errorCode $result
} {NONE {wrong # args: should be "prn start ?-document name? ?-output name?"}}

test test-1.4.2 {command start job bad args number} {
    catch {prn start -document} result
    list $errorCode $result
} {NONE {missing option value for -document}}

test test-1.4.3 {command start job bad args number} {
    catch {prn start -output} result
    list $errorCode $result
} {NONE {missing option value for -output}}

test test-1.4.4.1 {command stop job not started} {
    catch {prn end} result
    list $errorCode $result
} {NONE {document is not started}}

test test-1.4.4.2 {command abort job not started} {
    catch {prn abort} result
    list $errorCode $result
} {NONE {document is not started}}

test test-1.4.5 {command stop job bad arg} {
    reopen prn
    prn start -output $testingOutput
    catch {prn end unknown} result
    prn end
    list $errorCode $result
} {NONE {wrong # args: should be "prn end"}}

test test-1.4.6 {command double start job} {
    reopen prn
    prn start -output $testingOutput
    catch {prn start  -output $testingOutput} result
    prn end
    list $errorCode $result
} {NONE {document is already started}}

test test-1.5.1 {command start page bad arg} {
    reopen prn
    prn start -output $testingOutput
    catch {prn startpage unknown} result
    prn end
    list $errorCode $result
} {NONE {wrong # args: should be "prn startpage"}}

test test-1.5.2 {command endpage w/o startpage} {
    reopen prn
    prn start -output $testingOutput
    catch {prn endpage} result
    prn end
    list $errorCode $result
} {NONE {page is not started}}

test test-1.5.3 {command stop page bad arg} {
    reopen prn
    prn start -output $testingOutput
    prn startpage
    catch {prn endpage unknown} result
    prn end
    list $errorCode $result
} {NONE {wrong # args: should be "prn endpage"}}

test test-1.6.1 {command start/stop job succesfully} {
    list \
        [reopen prn] \
        [prn start  -document TEST -output $testingOutput] \
        [prn end] \
        [file delete $testingOutput] \
} [list prn {} {} {}]


test test-1.6.2 {command start/abort job succesfully} {
    list \
        [reopen prn] \
        [prn start  -document TEST -output $testingOutput] \
        [prn abort] \
        [file delete $testingOutput] \
} [list prn {} {} {}]

test test-1.6.2.1 {known bug: start dont work after abort, reopen printer} {
    set n [prn info name]
    prn close
    printer open -name $n prn
} {prn}

test test-1.6.3 {command start/stop unicode named job succesfully} {
    list [reopen prn] \
        [prn start -document ТЕСТ -output $testingOutput] \
        [prn end] \
        [file delete $testingOutput]
} [list prn {} {} {}]

test test-1.6.4 {command start/stop job duplicated} {
    list [reopen prn] \
       [prn start -output $testingOutput] \
       [catch {prn start -output $testingOutput} result] \
       $result \
       [prn end]
} {prn {} 1 {document is already started} {}}

test test-1.6.5 {command endpage w/o start in job} {
    reopen prn
    prn start -output $testingOutput
    catch {prn endpage} result
    prn end
    list $errorCode $result
} {NONE {page is not started}}

test test-1.6.6 {command start/stop job/page succesfully} {
    list [reopen prn] [prn start -output $testingOutput] [prn startpage] [prn endpage] [prn end] 
} {prn {} {} {} {}}

test test-1.7.1 {start page for print tests} {
    reopen prn
    prn start -output $testingOutput
    prn startpage
} {}

test test-1.7.2.1 {print syntax error} {
    catch {prn print} result
    list $errorCode $result
} {NONE {wrong # args: should be "prn print ?-margins rectspec? ?-font fontspec? ?-wordwrap? text"}}

test test-1.7.2.2 {print -margins missing syntax error} {
    catch {prn print -margins} result
    list $errorCode $result
} {NONE {missing option value for -margins}}

test test-1.7.2.3 {print -margins nonint syntax error} {
    catch {prn print -margins x test} result
    list $errorCode $result
} {NONE {expected integer but got "x", invalid option value for -margins}}

test test-1.7.2.4 {print -margins syntax error} {
    catch {prn print -margins {1 2 3 4 5}} result
    list $errorCode $result
} {NONE {too many elements, invalid option value for -margins}}

test test-1.7.2.5 {print -font missing syntax error} {
    catch {prn print -font} result
    list $errorCode $result
} {NONE {missing option value for -font}}

test test-1.7.2.6 {print -font unknown} {
    catch {prn print -font {unknown} data} result
    list $errorCode 
} {NONE}

test test-1.7.2.7 {print -font syntax error} {
    catch {prn print -font {{} unknown} data} result
    list $errorCode $result
} {NONE {unknown property "unknown", invalid option value for -font}}

test test-1.7.2.8 {print -margins -font syntax error} {
    catch {prn print -margins -font} result
    list $errorCode $result
} {NONE {expected integer but got "-font", invalid option value for -margins}}

test test-1.7.3.1 {place syntax error} {
    catch {prn place} result
    list $errorCode $result
} {NONE {wrong # args: should be "prn place ?-rect rectspec? ? ?-align flags? ?-font fontspec? ?-calcrect? text"}}

test test-1.7.3.2 {place -rect missing syntax error} {
    catch {prn place -rect} result
    list $errorCode $result
} {NONE {missing option value for -rect}}

test test-1.7.3.3 {place -rect nonint syntax error} {
    catch {prn place -rect x test} result
    list $errorCode $result
} {NONE {expected integer but got "x", invalid option value for -rect}}

test test-1.7.3.4 {place -rect syntax error} {
    catch {prn place -rect {1 2 3 4 5}} result
    list $errorCode $result
} {NONE {too many elements, invalid option value for -rect}}

test test-1.7.3.5 {place -font missing syntax error} {
    catch {prn place -font} result
    list $errorCode $result
} {NONE {missing option value for -font}}

test test-1.7.3.6 {place -font syntax error} {
    catch {prn place -font {unknown} data} result
    list $errorCode 
} {NONE}

test test-1.7.3.7 {place -font syntax error} {
    catch {prn place -font {{} unknown} data} result
    list $errorCode $result
} {NONE {unknown property "unknown", invalid option value for -font}}

test test-1.7.3.8 {place -font syntax error} {
    catch {prn place -font} result
    list $errorCode $result
} {NONE {missing option value for -font}}

test test-1.7.3.9 {place -rect -font syntax error} {
    catch {prn place -rect -font} result
    list $errorCode $result
} {NONE {expected integer but got "-font", invalid option value for -rect}}

test test-1.7.9 {end print testing page} {
    prn endpage
    prn end
} {}

test test-2.9 {} {
    list [prn close] [info commands prn]
} {{} {}}

test test-3.0 {} {
    list [printer open -default prn1] [printer open -default prn2]
} {prn1 prn2}

test test-3.1 {} {
    list [prn1 info name] [prn2 info name]
} [list $defaultDevice $defaultDevice]

test test-3.9 {} {
    list [prn1 close] [prn2 close]
} {{} {}}

test test-5.2.1 {-raw open syntax error} {
    catch {printer open -raw -default xxx yyy} result
    list $errorCode $result
} $printerUsage

test test-5.2.2 {-raw open syntax error} {
    catch {printer open -raw -name xxx yyy zzz} result
    list $errorCode $result
} $printerUsage

test test-5.2.3 {-raw open syntax error} {
    catch {printer open -raw -name} result
    list $errorCode $result
} {NONE {printer name required for -name}}

test test-5.2.6 {-raw open/close default successfully} {
    list [printer open -raw -default prn] [info commands prn] [prn close]
} {prn prn {}}

test test-5.2.7 {-raw open/close by name} {
    list [printer open -raw -name $testingDevice prn] [info commands prn] [prn close]
} {prn prn {}}

test test-5.3.0 {-raw open by name} {
    list [printer open -name $testingDevice prn] [info commands prn]
} {prn prn}

test test-5.3.1 {-raw command noargs error} {
    catch {prn} result
    list $errorCode $result
} {NONE {wrong # args: should be "prn command"}}

test test-5.3.2 {-raw command unknown} {
    catch {prn unknown} result
    list $errorCode $result
} {NONE {bad command "unknown": must be info, start, abort, end, startpage, endpage, place, print, frame, fill, rectangle, round, ellipse, or close}}

test test-5.3.3 {-raw command info unknown} {
    catch {prn info} result
    list $errorCode $result
} {NONE {wrong # args: should be "prn info name"}}

test test-5.3.4 {-raw command info name} {
    prn info name
} $testingDevice

test test-5.4.0.1 {-raw open/start/end/close command} {
   list [printer open -name $testingDevice prn] [prn start -output $testingOutput] [prn end] [prn close]
} {prn {} {} {}}

test test-5.4.0.2 {-raw open/start/end command} {
   list [printer open -name $testingDevice prn] [prn start -output $testingOutput] [prn end] 
} {prn {} {}}

test test-5.4.0.3 {-raw reopen/start/end command - check for reopen works} {
   list [reopen prn] [prn start -output $testingOutput] [prn end]
} {prn {} {}}

test test-5.4.0.4 {-raw open command for testing} {
  printer open -name $testingDevice prn
} {prn}

test test-5.4.1 {-raw command start job bad arg} {
    catch {prn start unknown} result
    list $errorCode $result
} {NONE {wrong # args: should be "prn start ?-document name? ?-output name?"}}

test test-5.4.2 {-raw command start job bad args number} {
    catch {prn start -document} result
    list $errorCode $result
} {NONE {missing option value for -document}}

test test-5.4.3 {-raw command start job bad args number} {
    catch {prn start -output} result
    list $errorCode $result
} {NONE {missing option value for -output}}

test test-5.4.4.1 {-raw command stop job not started} {
    catch {prn end} result
    list $errorCode $result
} {NONE {document is not started}}

test test-5.4.4.2 {-raw command abort job not started} {
    catch {prn abort} result
    list $errorCode $result
} {NONE {document is not started}}

test test-5.4.5 {-raw command stop job bad arg} {
    reopen prn
    prn start -output $testingOutput
    catch {prn end unknown} result
    prn end
    list $errorCode $result
} {NONE {wrong # args: should be "prn end"}}

test test-5.4.6 {-raw command double start job} {
    reopen prn
    prn start -output $testingOutput
    catch {prn start  -output $testingOutput} result
    prn end
    list $errorCode $result
} {NONE {document is already started}}

test test-5.5.1 {-raw command start page bad arg} {
    reopen prn
    prn start -output $testingOutput
    catch {prn startpage unknown} result
    prn end
    list $errorCode $result
} {NONE {wrong # args: should be "prn startpage"}}

test test-5.5.2 {-raw command endpage w/o startpage} {
    reopen prn
    prn start -output $testingOutput
    catch {prn endpage} result
    prn end
    list $errorCode $result
} {NONE {page is not started}}

test test-5.5.3 {-raw command stop page bad arg} {
    reopen prn
    prn start -output $testingOutput
    prn startpage
    catch {prn endpage unknown} result
    prn end
    list $errorCode $result
} {NONE {wrong # args: should be "prn endpage"}}

test test-5.6.1 {-raw command start/stop job succesfully} {
    list \
        [reopen prn] \
        [prn start  -document TEST -output $testingOutput] \
        [prn end] \
        [file delete $testingOutput] \
} [list prn {} {} {}]

test test-5.6.2 {-raw command start/abort job succesfully} {
    list \
        [reopen prn] \
        [prn start  -document TEST -output $testingOutput] \
        [prn abort] \
        [file delete $testingOutput] \
} [list prn {} {} {}]

test test-5.6.2.1 {known bug: start dont work after abort, reopen printer} {
    set n [prn info name]
    prn close
    printer open -raw -name $n prn
} {prn}

test test-5.6.3 {-raw command start/stop unicode named job succesfully} {
    list [reopen prn] \
        [prn start -document ТЕСТ -output $testingOutput] \
        [prn end] \
        [file delete $testingOutput]
} [list prn {} {} {}]

test test-5.6.4 {command start/stop job duplicated} {
    list [reopen prn] \
        [prn start -output $testingOutput] \
        [catch {prn start -output $testingOutput} result] \
        $result \
        [prn end]
} {prn {} 1 {document is already started} {}}

test test-5.6.5 {-raw command endpage w/o start in job} {
    reopen prn
    prn start -output $testingOutput
    catch {prn endpage} result
    prn end
    list $errorCode $result
} {NONE {page is not started}}

test test-5.6.6 {-raw command start/stop job/page succesfully} {
    list [reopen prn] [prn start -output $testingOutput] [prn startpage] [prn endpage] [prn end] 
} {prn {} {} {} {}}

test test-5.7.1 {-raw write} {
    reopen prn
    catch {prn write blablabla} result
    list [lrange $errorCode 0 1] [lrange $result 0 2]
} {{WINDOWS 3003} {error writing data:}}

test test-5.7.2 {-raw document write} {
    set s blablabla
    reopen prn
    prn start -document test -output $testingOutput
    prn write $s
    prn end
} {}

test test-5.7.2.1 {-raw document check} {
    set s
} [viewFile $testingOutput]

test test-5.7.3 {-raw document write unicode} {
    set s ЙЦУКЕНйцукен№ЁёІіЇї
    reopen prn
    prn start -document test -output $testingOutput
    prn write [encoding convertto $s]
    prn end
} {}

test test-5.7.3.1 {-raw document check unicode} {
    set s
} [viewFile $testingOutput]

catch {file delete $testingOutput}

cleanupTests 

puts "end of tests"

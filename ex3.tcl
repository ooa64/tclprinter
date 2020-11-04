#
# Print nonascii symbols.
#
# Note: raw printing fails
#

if {[catch {
    set libname tclprinter10
    if {[info exists ::tcl_platform(debug)]} {
        append libname g
    } 
    append libname .dll
    load $libname tclprinter
} result]} {
    puts "can't load printer library $libname: $result"
    exit 1
}

proc viewfile {filename} {
    if {[catch {
	package require registry
        set ext [file extension $filename]
	set app [registry get HKEY_CLASSES_ROOT\\$ext {}]
	set cmd [registry get HKEY_CLASSES_ROOT\\$app\\shell\\open\\command {}]
	regsub -all {%1} $cmd $filename cmd
	set cmd "[auto_execok start] [regsub -all {\\} $cmd {\\\\}]"
	puts $cmd
	eval exec $cmd
    } result]} {
	puts stderr "open $filename - $result"
    }
}

set printer {}
set printertype {}
set printerfont {}
set printermargins {}
set printeroutput {}
set printertimeout {30}
set printencoding {utf-8}

if 1 {
set printer {Microsoft Print to PDF}
set printertype {gdi}
set printerfont {{} 10 russian draft fixed}
set printerfont {{Lucida Console} 12 4 russian}
set printerfont {Courier 10}
set printermargins {100 100 100 100}
set printeroutput [file join [pwd] testprinter.pdf]
}

if 0 {
set printer {Microsoft Print to PDF}
set printertype {raw}
set printeroutput [file join [pwd] testprinter.pdf]
}

if 0 {
set printer {HP LaserJet Professional P1606dn}
set printer {OKI MB472(PCL6) (Копия 1)}
set printertype {gdi}
set printerfont {Courier 10}
set printermargins {100 100 100 100}
}

if 0 {
set printer {OKI MB472(PCL6)}
set printertype {raw}
set printencoding cp866
}

if 0 {
set printer {EPSON LX-300+ /II}
set printertype {gdi}
set printerfont {Courier 10 draft}
}

if 0 {
set printer {EPSON LX-300+ /II}
set printertype {raw}
}

set report {}
set line 1
for {set i 1} {$i <= 80} {incr i} {
    if {$i < 10} {
        append report $i
    } elseif {($i % 10) == 0} {
        append report $i
        incr i [string length $i]
        incr i -1 
    } else {
        append report " "
    }
}
append report \n; incr line
for {} {$line <= 10} {incr line} {
    append report $line \n
}
append report \n; incr line 
append report [format "type '%s', name '%s', font '%s'" $printertype $printer $printerfont] \n; incr line 
append report \n; incr line 
append report "А Б В Г Д Е Ж З И Й К Л М Н О П Р С Т У Ф Х Ц Ч Ш Щ Ъ Ы Ь Э Ю Я" \n; incr line
append report "а б в г д е ж з и й к л м н о п р с ф х ш щ ц ч ш щ ъ ы ь э ю я" \n; incr line
append report ". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ." \n; incr line
append report "# e E b b i I i I e E r r" \n; incr line
append report "№ ё Ё ъ Ъ ї Ї і І є Є ґ Ґ" \n; incr line
append report \n; incr line 
for {} {$line <= 70} {incr line} {
    if {($line % 10) == 0} {
        append report $line \n
    } else { 
        append report \n
    }
}

if {$printertype eq "gdi"} {

    if {$printeroutput ne ""} {
        printer print -name $printer -output $printeroutput -margins $printermargins -font $printerfont $report
        viewfile $printeroutput
    } else {
        set d [format %s%s testprinter [clock seconds]]
        printer open -gdi -name $printer p
        p start -document $d
        p startpage
        p print -margins $printermargins -font $printerfont $report
        p endpage
        p end
        printer open -raw -name $printer m
        set j [m document select id -document $d] 
        if {[llength $j] == 1} {
            set s {}
            set i 0
            set j [lindex $j 0] 
            puts "monitor start: $j $d"
            while {true} {
                if {$printertimeout ne "" && $i == $printertimeout} {
                    puts "\nmonitor timeout"
                    catch {m document delete $j}
                }
                if {[catch {m document status $j} result]} {
                    puts "\nmonitor error: $errorCode"
                    break;
                } elseif {$s != $result} {
                    puts "\nmonitor status: $result"
                    set s $result
                }
                after 1000
                puts -nonewline "."; flush stdout
                incr i
            }
        }
        m close
        p close
    }

} elseif {$printertype eq "raw"} {

    if {$printeroutput ne ""} {
        printer write -name $printer -output $printeroutput $report
        viewfile $printeroutput
    } else {
        set d [format %s%s testprinter [clock seconds]]
        printer open -raw -name $printer p
        p start -document $d
        p startpage
        p write [encoding convertto $printencoding $report]
        p endpage
        p end
        
        set s {}
        set i 0
        puts "monitor start: [p document id] $d"
        while {true} {
            if {$printertimeout ne "" && $i == $printertimeout} {
                puts "\nmonitor timeout"
                catch {p document delete}
            }
            if {[catch {p document status} result]} {
                puts "\nmonitor error: $errorCode"
                break;
            } elseif {$s != $result} {
                puts "\nmonitor status: $result"
                set s $result
            }
            after 1000
            puts -nonewline "."; flush stdout
            incr i
        }
        p close
    }

} else {

}

puts "\nPress Enter..."; gets stdin

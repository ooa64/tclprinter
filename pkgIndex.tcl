if {[info exists ::tcl_platform(debug)]} {
    package ifneeded tclprinter 1.0 \
        [list load [file join $dir tclprinter10g.dll] tclprinter] 
} else {
    package ifneeded tclprinter 1.0 \
        [list load [file join $dir tclprinter10.dll] tclprinter] 
}
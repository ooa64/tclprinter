# Example 1. Printing.

lappend auto_path .

package require tclprinter

set printerName "Microsoft XPS Document Writer"
set outputName "ex1.xps"
set fileName [file join $tcl_library init.tcl]

proc readFile fileName {
    set h [open $fileName r]
    set s [read $h]
    close $h
    return $s
}

proc viewFile fileName {
    if {[catch {
        package require registry
        set app [registry get {HKEY_CLASSES_ROOT\.xps} {}]
        set cmd [registry get HKEY_CLASSES_ROOT\\$app\\shell\\open\\command {}]
        regsub -all {%1} $cmd $fileName cmd
        set cmd "[auto_execok start] [regsub -all {\\} $cmd {\\\\}]"
        eval exec $cmd
    } result]} {
        puts "viewFile $fileName: $result"
    }
}

puts "About to send $fileName to the $printerName"

set chars [printer print -name $printerName -output $outputName -font Courier [readFile $fileName]]

puts "$chars chars printed"

viewFile $outputName

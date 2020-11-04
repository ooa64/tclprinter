# Example 1. Printing.

lappend auto_path .

package require tclprinter

# Windows 10
set printerName {Microsoft Print to PDF}
set outputName [file join [pwd] ex1.pdf]
# Windows 7
#set printerName "Microsoft XPS Document Writer"
#set outputName "ex1.xps"
set fileName "tclprinter.txt"

proc readFile fileName {
    set h [open $fileName r]
    fconfigure $h -encoding utf-8
    set s [read $h]
    close $h
    return $s
}

proc viewFile fileName {
    if {[catch {
        package require registry
        set app [registry get HKEY_CLASSES_ROOT\\[file extension $::outputName] {}]
        set cmd [registry get HKEY_CLASSES_ROOT\\$app\\shell\\open\\command {}]
        regsub -all {%1} $cmd $fileName cmd
        set cmd "[auto_execok start] [regsub -all {\\} $cmd {\\\\}]"
        eval exec $cmd
    } result]} {
        puts "viewFile $fileName: $result"
    }
}

puts "About to send $fileName to the $printerName"

set chars [printer print -name $printerName -output $outputName -font {Courier -8} [readFile $fileName]]

puts "$chars chars printed"

after 1000

viewFile $outputName

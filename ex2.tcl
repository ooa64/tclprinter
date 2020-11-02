# Example 2. Drawing.

lappend auto_path .

package require tclprinter

set printerName "Microsoft XPS Document Writer"
set outputName "ex2.xps"
set text1 "The 'place' command draws formatted text in the specified rectangle and formats the text according to the specified method (expanding tabs, justifying characters, breaking lines, and so forth). If the function succeeds, the return value is the height of the text in device units. If vcenter or bottom align mode is specified, the return value is the offset from -rect top value to the bottom of the drawn text."
set text2 "wordellipsis truncates any word that does not fit in the rectangle and adds ellipses."
set font1 {Arial 12 bold}
set font2 {Lucida 10}

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

printer open -gdi -name $printerName prn
prn start -output $outputName
prn startpage

set pageHeight [prn info height]
set pageWidth [prn info width]

set rect [list 100 100 [expr {$pageWidth/2 - 100}] [expr {$pageHeight/4 - 100}] ]
prn rect -rect $rect
prn place -rect $rect -align wordbreak -font $font1 $text1

set rect [list 100 [expr {$pageHeight/4}] [expr {$pageWidth/2 - 100}] [expr {$pageHeight/2 - 100}] ]
prn rect -rect $rect
prn place -rect $rect -align {wordellipsis singleline vcenter} -font $font2 $text2]

set y [expr {$pageHeight/2}]
prn rect -rect [list 0 [incr y 100] [expr {$pageWidth/2}] [expr {$y+1}]]
prn rect -rect [list 0 [incr y 100] [expr {$pageWidth/2}] [expr {$y+2}]]
prn rect -rect [list 0 [incr y 100] [expr {$pageWidth/2}] [expr {$y+5}]]
prn rect -rect [list 0 [incr y 100] [expr {$pageWidth/2}] [expr {$y+10}]]
prn rect -rect [list 0 [incr y 100] [expr {$pageWidth/2}] [expr {$y+20}]]  -brush black

prn rect -rect [list [expr {$pageWidth/2}] 0 [expr {$pageWidth/2+1}] $pageHeight]

prn ellipse -rect [list [expr {$pageWidth/2+100}] 100 [expr {$pageWidth-100}] [expr {$pageHeight/4 - 100}]] -brush ltgray

prn endpage
prn end
prn close

viewFile $outputName

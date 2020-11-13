# tclprinter


Tclprinter - Tcl interface to the Windows printing system.


Example 1. List installed printers.
```
puts [printer names]
```

Example 2. Print to the matrix printer directly.
```
printer write -name "EPSON LX-300+ /II" [encoding convertto $text]
```

Example 3. Print to the laser printer via Windows GDI.
```
printer print -name "Canon LBP2900" $text
```

Tcl printer uses two windows printing interfaces: "raw" (Print Spooler API) and "gdi" (GDI Print API).
To create a handle to the API use  'printer open -raw'  or 'printer open -gdi'


Example 4. Draw a page using GDI API
```
# open gdi handle
set h [printer open -gdi -name "Canon LBP2900"]

# start a job
$h start

# start a page
$h startpage

# draw a rectangle
$h rectangle -rect {100 100 2000 1000}

# place a text into rectangle
$h place -rect {100 100 2000 1000} -align {center wordbreak} -font {Courier 10} $text

# draw a gray ellipse
$h ellipse -rect {100 1100 2000 2000} -brush ltgray

# close all
$h endpage
$h end
$h close
```

Example 5. Pause all printer jobs using Spooler API.
```
set h [printer open -raw -name "EPSON LX-300+ /II"]
foreach id [$h document select id] {
    $h document pause $id
}
$h close
```

Example 6. Print and control the job using Spooler API.
```
set h [printer open -raw -name "EPSON LX-300+ /II"]
$h start -document "Example 6"
$h startpage
$h write [encoding convertto $text]
$h endpage
$h end
for {set i 0}  {true}  {incr i} {
    if {[catch {$h document status}]} {
        puts "End of the job"
        $h close
        break
    }
    if {$i >= 10} {
        puts "Timed out"
        $h abort
        break
    }
    after 1000
}
```

Example 7. Print using GDI API and control the job using Spooler API.
```
set document "Example 5 - [clock clicks]"
set printer [printer open -gdi -name "Canon LBP2900"]
$printer start -document $document
$printer startpage
$printer print $text
$printer endpage
$printer end

# GDI API does not provide interface to the printer job, create a "raw" handle
set monitor [printer open -raw -name "Canon LBP2900"]

# Locate the job 
set documentid [lindex [$monitor document select id -document $document] 0]

for {set i 0} {true} {incr i} {
    if {[catch {$monitor document status $documentid} result]} {
        puts "End of the job"
        break
    }
    puts "Current job status: $result"
    if {$i >= 10} {
        $monitor document delete $documentid
        puts "Timed out"
        break
    }
    after 1000
}

$monitor close
$printer close
```

Changes...

    2014-03-04 - start
    2014-03-16 - alpha 1. GDI interface
    2014-03-20 - alpha 2. RAW interface
    2014-03-27 - alpha 3. Job control (rawcommand document)
    2014-04-02 - alpha 4. Compatibility fixes and cleanups
                          Tcl 8.5/8.6 - all tests passed
                          Gcc 4.8.1 (mingw) - all tests passed
                          VC 6.0 / Windows 98 - failed on test 6-1-3/4 and some errorcodes
    04.11.2020            Recompiled and tested on Windows 10, VC 14.0
                          rawcommand tests failed on Windows 10 (passed on Windows 7)
                          new example ex3.tcl (raw printing fails)
    12.11.2020            All tests passed for 32bit and 64bit builds                          

Missing...

1. Good documentation (just a tclptrinter.txt).
2. Advancef error reporting (TclWinConvertError, Tcl_SetErrorCode).
3. Callback for startpage/endpage.
4. Noninteractive printer setup (page form, orientation, copies etc).
5. Image printing.
6. XPS interface?
7. Testing on various Windows versions, and varios printers.

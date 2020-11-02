#include <tcl.h>

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

EXTERN int Tclprinter_Init _ANSI_ARGS_ ((Tcl_Interp *));

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT

int Tclprinter_AppInit(Tcl_Interp *interp)
{
#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, "8.1", 0) == 0L) {
        return TCL_ERROR;
    }
#endif

    if (Tcl_Init(interp) == TCL_ERROR) {
        goto error;
    }

    if (Tclprinter_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }

    Tcl_StaticPackage(interp, "tclprinter", Tclprinter_Init, NULL);
    Tcl_SetVar(interp, "tcl_rcFileName", "~/tclprintershrc.tcl", TCL_GLOBAL_ONLY);
    return TCL_OK;

error:
    return TCL_ERROR;
}

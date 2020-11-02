#ifndef TCLPRINT_H
#define TCLPRINT_H

#include <windows.h>
#include "tclcmd.hpp"

class TclPrintCmd : public TclCmd {
    friend class TclPrinterCmd;

public:

    TclPrintCmd(Tcl_Interp *interp, char *name):
          TclCmd(interp, name),
          lasterrorcode(0),
          pageno(0),
          pageposy(0),
          pageactive(FALSE),
          printerdc(NULL),
          printerfont(NULL),
          printername(NULL) {
        DEBUGLOG("TclPrintCmd::Construct *" << this);
        ZeroMemory(&docinfo, sizeof(docinfo));
        ZeroMemory(&logfont, sizeof(logfont));
    };

    virtual ~TclPrintCmd() {
        Close();
        DEBUGLOG("TclPrintCmd::Destruct *" << this);
    };

    BOOL Opened();
    BOOL Started();
    BOOL PageStarted();

    void Close();
    int Select(BOOL usedefault);
    int Open(TCHAR *name);
    int Configure(LOGFONT *fc);
    int StartDoc(TCHAR *document, TCHAR *output);
    int AbortDoc();
    int EndDoc();
    int StartPage();
    int EndPage();
    int Place(RECT *rect, BOOL calc, UINT align, TCHAR *text);
    int Print(RECT *rect, BOOL wrap, TCHAR *text);
    int PrintDoc(TCHAR *document, TCHAR *output, RECT *rect, BOOL wrap, TCHAR *text);
    int PrintDocObj(TCHAR *document, TCHAR *output, RECT *rect, BOOL wrap, Tcl_Obj *textobj);

    HDC GetDC() {
        return printerdc;
    };

    DWORD GetError() {
        return lasterrorcode;
    }

protected:

    int pageno;
    int pageposy;
    BOOL pageactive;
    HDC printerdc;
    HFONT printerfont;
    TCHAR *printername;
    DOCINFO docinfo;
    LOGFONT logfont;
    DWORD lasterrorcode;

    VOID SetError(DWORD errorcode) {
        lasterrorcode = errorcode;
    }

    int ParseFontArg(struct Tcl_Obj *obj, LOGFONT *lf);
    int ParseAlignArg(struct Tcl_Obj *obj, UINT *format);
    int ParseRectArg(struct Tcl_Obj *obj, RECT *rect);

private:

    virtual int Command(int objc, struct Tcl_Obj *CONST objv[]);
};

#endif

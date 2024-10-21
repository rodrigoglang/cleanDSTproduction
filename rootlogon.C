{
  if(gSystem->Getenv("HESSUSER")) {
    gSystem->SetIncludePath("-I$HESSUSER/include -I$HESSROOT/include -I/usr/include/mysql");
    gROOT->SetMacroPath(".:$HESSUSER:$HESSROOT");
  }
  else {
    gSystem->SetIncludePath("-I$HESSROOT/include -I/usr/include/mysql");
    gROOT->SetMacroPath(".:$HESSROOT");
  }
}


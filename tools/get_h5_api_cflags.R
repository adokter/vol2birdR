if (pkgbuild::has_rtools()) {
  ver<-pkgbuild::rtools_needed()
  CFLAGS<-""
  if (ver != "Rtools 4.0") {
    CFLAGS<-"-DH5_USE_110_API"
  }
  cat(CFLAGS)
  quit("no", status=0)
}
quit("no", status=127)


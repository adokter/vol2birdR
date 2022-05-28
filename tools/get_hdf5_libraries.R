if (pkgbuild::has_rtools()) {
  ver<-pkgbuild::rtools_needed()
  LIBS<-"-lm -ldl -lws2_32 -lwsock32 -lz -lhdf5"
  if (ver == "Rtools 4.0") {
    LIBS<-"-lz -lhdf5"
  }
  cat(LIBS)
  quit("no", status=0)
}
quit("no", status=127)


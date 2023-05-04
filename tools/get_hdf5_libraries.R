if (pkgbuild::has_rtools()) {
  ver<-pkgbuild::rtools_needed()
  LIBS<-"-lhdf5 -lwsock32 -lz -lsz -lm -ldl -lws2_32"
  if (ver == "Rtools 4.0") {
    LIBS<-"-lhdf5 -lz"
  }
  cat(LIBS)
  quit("no", status=0)
}
quit("no", status=127)


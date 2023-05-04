if (pkgbuild::has_rtools()) {
  ver<-pkgbuild::rtools_needed()
  LIBS<-"-lproj -lsqlite3 -lz -ldl -ltiff -lwebp -lsharpyuv -lzstd -llzma -ljpeg -lz -lcurl -lidn2 -lunistring -liconv -lcharset -lssh2 -lgcrypt -lgpg-error -lws2_32 -lgcrypt -lgpg-error -lws2_32 -lz -ladvapi32 -lcrypt32 -lssl -lcrypto -lssl -lz -lws2_32 -lgdi32 -lcrypt32 -lcrypto -lbcrypt -lz -lws2_32 -lgdi32 -lcrypt32 -lgdi32 -lwldap32 -lzstd -lz -lws2_32 -lpthread -lstdc++"
  if (ver == "Rtools 4.0") {
    LIBS<-"-lproj -lsqlite3 -lz -ltiff -ljpeg -lz -lcurl -lnormaliz -lssh2 -lcrypt32 -lgdi32 -lws2_32 -lcrypt32 -lgdi32 -lws2_32 -lssl -lcrypto -lws2_32 -lgdi32 -lcrypt32 -lz -lssl -lcrypto -lssl -lcrypto -lws2_32 -lgdi32 -lcrypt32 -lgdi32 -lcrypt32 -lwldap32 -lz -lws2_32 -lstdc++"
  }
  cat(LIBS)
  quit("no", status=0)
}
quit("no", status=127)    


import regex
import unicode

var unicode_cvt = new unicode.codecvt.utf8
system.out.println(unicode.build_optimize_wregex(unicode_cvt.local2wide("TEST")))

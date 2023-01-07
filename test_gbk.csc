@charset: gbk
import unicode
var cvt = new unicode.codecvt.gbk
var str = cvt.local2wide("ÄãºÃ£¬ÊÀ½ç£¡")
system.out.println(cvt.wide2local(str))
for i = 0, i < str.size(), ++i
    system.out.println(cvt.wide2local(str.at(i).to_wstring()) + " " + cvt.is_identifier(str.at(i)))
end

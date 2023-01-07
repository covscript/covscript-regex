@charset: utf8
import unicode
var cvt = new unicode.codecvt.utf8
var str = cvt.local2wide("你好，世界！")
system.out.println(cvt.wide2local(str))
for i = 0, i < str.size(), ++i
    system.out.println(cvt.wide2local(str.at(i).to_wstring()) + " " + cvt.is_identifier(str.at(i)))
end

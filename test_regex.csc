import regex
var re = regex.build("^.*?(\\w+)\\.(c|cc|cpp|cxx)$")
var sm = re.match("test/regex.cpp")

system.out.println("Matched groups: " + sm.size())
foreach i in range(sm.size())
    system.out.println(sm.str(i))
end

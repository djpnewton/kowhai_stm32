# get symbol tokens
f = open("symbols.txt")
syms = f.read().split("\n")

# strip duplicates and comments
def check_exists(sym, syms):
    for s in syms:
        if s.lower() == sym.lower():
            return True
def is_comment(sym):
    return sym[0] == '#'
syms2 = []
for sym in syms:
    if sym and not check_exists(sym, syms2) and not is_comment(sym):
        syms2.append(sym)
syms = syms2

# write symbols.h
f = open("symbols.h", "w")
f.write("#ifndef _SYMBOLS_H_\n")
f.write("#define _SYMBOLS_H_\n\n")
f.write("char* symbols[] = {\n")
for i in range(len(syms)):
    sym = syms[i]
    f.write("\t\"%s\",\n" % sym)
f.write("};\n\n")
for i in range(len(syms)):
    sym = syms[i]
    f.write("#define SYM_%s\t\t%d\n" % (sym.upper().replace(" ", "_"), i))
f.write("\n#endif\n");

# write Symbols.cs
f = open("Symbols.cs", "w")
f.write("namespace KowhaiSymbols\n{\n")
f.write("    public static class Symbols\n    {\n")
f.write("        public static string[] Strings =\n        {\n")
for sym in syms:
    f.write("            \"%s\",\n" % sym)
f.write("        };\n");
f.write("        public enum Constants\n        {\n")
for sym in syms:
    f.write("            %s,\n" % sym.replace(" ", "_"))
f.write("        };\n    }\n}\n");



# get symbol tokens
f = open("symbols.txt")
syms = f.read().split()

# strip duplicates
def check_exists(sym, syms):
    for s in syms:
        if s.lower() == sym.lower():
            return True
syms2 = []
for sym in syms:
    if not check_exists(sym, syms2):
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
    f.write("#define SYM_%s\t\t%d\n" % (sym.upper(), i))
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
    f.write("            %s,\n" % sym)
f.write("        };\n    }\n}\n");



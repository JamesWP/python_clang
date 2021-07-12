import sys
import importlib
import os

native = "native" in sys.argv
local_shared = "local_shared" in sys.argv

if native:
  # native module
  import python_clang
elif local_shared:
  so = os.environ.get("PYTHON_CLANG_SO")
  print(so)
  spec = importlib.util.spec_from_file_location("python_clang", so)
  python_clang = importlib.util.module_from_spec(spec)

compiler = python_clang.Compiler()

print("compiler", compiler, hex(id(compiler)))

func = compiler.compile("""
  int go() {
    static int thing = 0;
    return ++thing;
  }
""")

print("function", func, hex(id(func)))

output = func()
print(output)
assert output == 1

output = func()
print(output)
assert output == 2

output = func()
print(output)
assert output == 3

import sys
import importlib
import pkg_resources
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
else:
  # python mock
  python_clang = importlib.import_module("python_mock")

a = python_clang.Compiler()

func = a.compile("int go() { return 1+2+3; }")

print("result of compile", func)

#output = func()
#print(output)

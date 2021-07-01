import sys

native = "native" in sys.argv
  

if native:
  # native module
  import python_clang

  a = python_clang.Compiler()
else:
  # python mock
  import python_mock

  a = python_mock.Compiler()

func = a.compile("1+2+3")

output = func()


print(output)

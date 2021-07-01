class Compiler:
  def compile(self, code):
    def go():
      return eval(code)
    return go


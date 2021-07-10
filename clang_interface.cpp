#include "clang_interface.h"

#include "clang/Basic/DiagnosticOptions.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Tool.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"

#include <iostream>

using namespace clang;
using namespace clang::driver;

// This function isn't referenced outside its translation unit, but it
// can't use the "static" keyword because its address is used for
// GetMainExecutable (since some platforms don't support taking the
// address of main, and some platforms can't implement GetMainExecutable
// without being given the address of a function in the main executable).
std::string GetExecutablePath(const char *Argv0, void *MainAddr)
{
  return llvm::sys::fs::getMainExecutable(Argv0, MainAddr);
}

namespace llvm
{
  namespace orc
  {
    class SimpleJIT
    {
    private:
      ExecutionSession ES;
      std::unique_ptr<TargetMachine> TM;
      const DataLayout DL;
      MangleAndInterner Mangle{ES, DL};
      JITDylib &MainJD{ES.createJITDylib("<main>")};
      RTDyldObjectLinkingLayer ObjectLayer{ES, createMemMgr};
      IRCompileLayer CompileLayer{ES, ObjectLayer,
                                  std::make_unique<SimpleCompiler>(*TM)};

      static std::unique_ptr<SectionMemoryManager> createMemMgr()
      {
        return std::make_unique<SectionMemoryManager>();
      }

      SimpleJIT(
          std::unique_ptr<TargetMachine> TM, DataLayout DL,
          std::unique_ptr<DynamicLibrarySearchGenerator> ProcessSymbolsGenerator)
          : TM(std::move(TM)), DL(std::move(DL))
      {
        llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
        MainJD.addGenerator(std::move(ProcessSymbolsGenerator));
      }

    public:
      static Expected<std::unique_ptr<SimpleJIT>> Create()
      {
        auto JTMB = JITTargetMachineBuilder::detectHost();
        if (!JTMB)
          return JTMB.takeError();

        auto TM = JTMB->createTargetMachine();
        if (!TM)
          return TM.takeError();

        auto DL = (*TM)->createDataLayout();

        auto ProcessSymbolsGenerator =
            DynamicLibrarySearchGenerator::GetForCurrentProcess(
                DL.getGlobalPrefix());

        if (!ProcessSymbolsGenerator)
          return ProcessSymbolsGenerator.takeError();

        return std::unique_ptr<SimpleJIT>(new SimpleJIT(
            std::move(*TM), std::move(DL), std::move(*ProcessSymbolsGenerator)));
      }

      const TargetMachine &getTargetMachine() const { return *TM; }

      Error addModule(ThreadSafeModule M)
      {
        return CompileLayer.add(MainJD, std::move(M));
      }

      Expected<JITEvaluatedSymbol> findSymbol(const StringRef &Name)
      {
        return ES.lookup({&MainJD}, Mangle(Name));
      }

      Expected<JITTargetAddress> getSymbolAddress(const StringRef &Name)
      {
        auto Sym = findSymbol(Name);
        if (!Sym)
          return Sym.takeError();
        return Sym->getAddress();
      }
    };

  } // end namespace orc
} // end namespace llvm

int clang_interface_init()
{
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  return 0;
}

int clang_interface_fini()
{
  llvm::llvm_shutdown();
  return 0;
}

int clang_interface_createEnvironment(clang_interface_EnvironmentHandle *h)
{
  if (h == nullptr)
  {
    return 1;
  }

  if (h->handle != nullptr)
  {
    return 1;
  }

  auto jit = llvm::orc::SimpleJIT::Create();

  if (jit.takeError())
  {
    return 1;
  }

  h->handle = reinterpret_cast<void *>(jit->release());

  return 0;
}

int clang_interface_destroyEnvironment(clang_interface_EnvironmentHandle *h)
{
  if (h == nullptr)
  {
    return 1;
  }

  if (h->handle == nullptr)
  {
    return 1;
  }

  auto *jit = reinterpret_cast<llvm::orc::SimpleJIT *>(h->handle);

  delete jit;

  h->handle = nullptr;

  return 0;
}

int clang_interface_compileCode(clang_interface_EnvironmentHandle *h, const char *code, size_t code_size)
{
  if (h == nullptr)
  {
    return 1;
  }

  llvm::ExitOnError ExitOnErr;

  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
  TextDiagnosticPrinter *DiagClient =
      new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);

  IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
  DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

  const std::string TripleStr = llvm::sys::getProcessTriple();
  llvm::Triple T(TripleStr);

  ExitOnErr.setBanner("clang interpreter");

  std::string Path = "/not/actually/clang";

  Driver TheDriver(Path, T.str(), Diags);
  TheDriver.setTitle("clang interpreter");
  TheDriver.setCheckInputsExist(false);

  std::cout << "Compiling going well" << std::endl;

  SmallVector<const char *, 16> Args;
  Args.push_back("clang-interpreter");
  Args.push_back("test.c");
  Args.push_back("-fsyntax-only");
  Args.push_back("-v");
  std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(Args));
  if (!C)
  {
    return 1;
  }

  std::cout << "Compiling going well" << std::endl;

  const driver::JobList &Jobs = C->getJobs();
  if (Jobs.size() != 1 || !isa<driver::Command>(*Jobs.begin()))
  {
    SmallString<256> Msg;
    llvm::raw_svector_ostream OS(Msg);
    Jobs.Print(OS, "; ", true);
    Diags.Report(diag::err_fe_expected_compiler_job) << OS.str();
    return 1;
  }

  std::cout << "Compiling going well" << std::endl;

  driver::Command &Cmd = cast<driver::Command>(*Jobs.begin());

  if (llvm::StringRef(Cmd.getCreator().getName()) != "clang")
  {
    Diags.Report(diag::err_fe_expected_clang_command);
    return 1;
  }

  // Initialize a compiler invocation object from the clang (-cc1) arguments.
  const llvm::opt::ArgStringList &CCArgs = Cmd.getArguments();
  std::unique_ptr<CompilerInvocation> CI(new CompilerInvocation);
  CompilerInvocation::CreateFromArgs(*CI, CCArgs, Diags);

  // Map code filename to a memoryBuffer
  llvm::StringRef testCodeData(code, code_size);
  auto buffer = llvm::MemoryBuffer::getMemBufferCopy(testCodeData);
  CI->getPreprocessorOpts().addRemappedFile("test.c", buffer.release());

  // Show the invocation, with -v.
  if (CI->getHeaderSearchOpts().Verbose)
  {
    llvm::errs() << "clang invocation:\n";
    Jobs.Print(llvm::errs(), "\n", true);
    llvm::errs() << "\n";
  }

  // Create a compiler instance to handle the actual work.
  CompilerInstance Clang;
  Clang.setInvocation(std::move(CI));

  // Create the compilers actual diagnostics engine.
  Clang.createDiagnostics();
  if (!Clang.hasDiagnostics())
  {
    return 1;
  }

  std::cout << "Compiling going well" << std::endl;

  // Create and execute the frontend to generate an LLVM bitcode module.
  std::unique_ptr<CodeGenAction> Act(new EmitLLVMOnlyAction());
  if (!Clang.ExecuteAction(*Act))
  {
    return 1;
  }

  std::cout << "Compiling going well" << std::endl;

  int Res = 255;
  std::unique_ptr<llvm::LLVMContext> Ctx(Act->takeLLVMContext());
  std::unique_ptr<llvm::Module> Module = Act->takeModule();

  auto *J = reinterpret_cast<llvm::orc::SimpleJIT *>(h->handle);

  if (Module)
  {
    ExitOnErr(J->addModule(
        llvm::orc::ThreadSafeModule(std::move(Module), std::move(Ctx))));
  }

  auto Main = (int (*)())ExitOnErr(J->getSymbolAddress("go"));

  h->compiledFunctionHandle = Main;

  return 0;
}

int clang_interface_runCode(clang_interface_EnvironmentHandle *h, int *output)
{
  if (h == nullptr)
  {
    return 1;
  }

  int value = h->compiledFunctionHandle();

  if (output != nullptr)
  {
    *output = value;
  }

  return 0;
}

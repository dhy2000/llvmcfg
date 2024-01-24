#include <llvm/Support/CommandLine.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CFG.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/ToolOutputFile.h>

#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Bitcode/BitcodeWriter.h>

#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Scalar.h>

#include <llvm/IR/Function.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>

#include <queue>

using namespace llvm;
static ManagedStatic<LLVMContext> GlobalContext;
static LLVMContext& getGlobalContext() { return *GlobalContext; }

struct EnableFunctionOptPass : public FunctionPass {
  static char ID;
  EnableFunctionOptPass() :FunctionPass(ID) {}
  bool runOnFunction(Function& F) override {
    if (F.hasFnAttribute(Attribute::OptimizeNone)) {
      F.removeFnAttr(Attribute::OptimizeNone);
    }
    return true;
  }
};

char EnableFunctionOptPass::ID = 0;
    
struct CFGPass : public ModulePass {
  static char ID;
  CFGPass() : ModulePass(ID) {}

  bool runOnModule(Module& M) override {
    for (const Function& func : M.getFunctionList()) {
      if (func.isDeclaration())
        continue;
      outs() << "## " << func.getName() << "\n\n";
      outs() << "```mermaid\ngraph TD;\n";
      std::map<const BasicBlock*, int> bbids;
      int bbid = 0;
      for (const BasicBlock& bb : func.getBasicBlockList()) {
        bbids[&bb] = (++bbid);
        outs() << "bb_" << bbid << "(\"" << bb.getName() << "\")\n";
      }
      std::queue<const BasicBlock*> bfs_bb;
      std::set<const BasicBlock*> vis_bb;
      const BasicBlock* entry = &func.getEntryBlock();
      bfs_bb.push(entry);
      vis_bb.insert(entry);
      while (!bfs_bb.empty()) {
        const BasicBlock* bb = bfs_bb.front();
        bfs_bb.pop();
        for (const BasicBlock* succ: successors(bb)) {
          if (vis_bb.count(succ))
            continue;
          bfs_bb.push(succ);
          vis_bb.insert(succ);
          outs() << "bb_" << bbids.at(bb) << "-->" << "bb_" << bbids.at(succ) << "\n";
        }
      }
      outs() << "```\n\n";
    }
    return false;
  }
};

char CFGPass::ID = 0;
static RegisterPass<CFGPass> X("cfgpass", "Print control-flow graph");

static cl::opt<std::string>
InputFilename(cl::Positional, cl::desc("<filename>.bc"), cl::init(""));

int main(int argc, char **argv) {
  LLVMContext& Context = getGlobalContext();
  SMDiagnostic Err;

  cl::ParseCommandLineOptions(argc, argv, "CFGPass\nLLVM IR");

  outs() << "# " << InputFilename.getValue() << "\n\n";

  std::unique_ptr<Module> M = parseIRFile(InputFilename, Err, Context);
  if (!M) {
    Err.print(argv[0], errs());
    return 1;
  }
  llvm::legacy::PassManager Passes;
  Passes.add(new EnableFunctionOptPass());
  Passes.add(new CFGPass());

  Passes.run(*M.get());

  return 0;
}

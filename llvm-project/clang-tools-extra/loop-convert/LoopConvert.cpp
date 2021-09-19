// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/Refactoring.h"
// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"

using namespace clang::tooling;
using namespace llvm;

using namespace clang;
using namespace clang::ast_matchers;

class ForLoopReplacer : public MatchFinder::MatchCallback {
public :
  virtual void run(const MatchFinder::MatchResult &Result) {
    if (const ForStmt *FS = Result.Nodes.getNodeAs<clang::ForStmt>("forLoop")) {
      std::map<std::string, Replacements> replacements;
      Replacements re;
      auto &SM = *(Result.SourceManager);

      // Get the code from SourceManager
      std::string initCode(SM.getCharacterData(FS->getInit()->getBeginLoc()), SM.getCharacterData(FS->getInit()->getEndLoc())-SM.getCharacterData(FS->getInit()->getBeginLoc()));
      std::string condCode(SM.getCharacterData(FS->getCond()->getBeginLoc()), SM.getCharacterData(FS->getCond()->getEndLoc().getLocWithOffset(2))-SM.getCharacterData(FS->getCond()->getBeginLoc()));
      std::string incCode(SM.getCharacterData(FS->getInc()->getBeginLoc()), SM.getCharacterData(FS->getInc()->getEndLoc().getLocWithOffset(2))-SM.getCharacterData(FS->getInc()->getBeginLoc()));
      std::string whileText = "while (" + condCode + ")";

      // Create one Replacement for each text insertion/replacement
      // Errors must be handled https://llvm.org/doxygen/classllvm_1_1Error.html#details
      if (auto E = re.add(Replacement(SM, FS->getForLoc(), 0, initCode + ";" + "\n")))
      {
        llvm::errs() << "Error " << E << " while adding init code " << initCode << '\n';
        FS->dump();
        return;
      }
      
      if (auto E = re.add(Replacement(SM, CharSourceRange::getCharRange(SourceRange(FS->getForLoc(), FS->getRParenLoc().getLocWithOffset(1))), whileText)))
      {
        llvm::errs() << "Error " << E << " while adding while code " << whileText << '\n';
        FS->dump();
        return;
      }
      
      const Stmt *lastStmt = nullptr;
      for (auto it = FS->getBody()->child_begin(); it != FS->getBody()->child_end(); it ++)
      {
        lastStmt = *it;
      }
      if (lastStmt == nullptr) {
        llvm::errs() << "No statements in loop body\n";
        FS->dump();
        return;
      }
      if (auto E = re.add(Replacement(SM, lastStmt->getEndLoc().getLocWithOffset(2), 0, "\n" + incCode + ";")))
      {
        llvm::errs() << "Error " << E << " while adding increment code " << incCode << '\n';
        FS->dump();
        return;
      }

      // Add a set of replacements for this file
      std::string fileName = SM.getFileEntryForID(SM.getFileID(FS->getBeginLoc()))->getName().str();
      replacements.insert(std::make_pair(fileName, re));
      
      LangOptions DefaultLangOptions;
      Rewriter Rewrite(SM, DefaultLangOptions);
      formatAndApplyAllReplacements(replacements, Rewrite);
      Rewrite.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
      llvm::outs() << '\n';
    }
  }

  static StatementMatcher LoopMatcher;
};

StatementMatcher ForLoopReplacer::LoopMatcher =
    forStmt(hasLoopInit(declStmt(hasSingleDecl(varDecl(
      hasInitializer(integerLiteral(equals(0)))))))).bind("forLoop");

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// From clang/tools/clang-rename/ClangRename.cpp
int main(int argc, const char **argv) {
  tooling::CommonOptionsParser OP(argc, argv, MyToolCategory);
  tooling::ClangTool Tool(OP.getCompilations(), OP.getSourcePathList());
  ForLoopReplacer Replacer;
  MatchFinder ForLoopFinder;
  ForLoopFinder.addMatcher(ForLoopReplacer::LoopMatcher, &Replacer);
  Tool.run(newFrontendActionFactory(&ForLoopFinder).get());
}

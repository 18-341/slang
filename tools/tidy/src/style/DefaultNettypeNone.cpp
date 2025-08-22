//------------------------------------------------------------------------------
// DefaultNettypeNone.cpp
// Check that `default_nettype none` is specified at the beginning of the file
//
// SPDX-FileCopyrightText: Perrin Tong
// SPDX-License-Identifier: MIT
//------------------------------------------------------------------------------

#include "ASTHelperVisitors.h"
#include "TidyDiags.h"

#include "slang/parsing/Token.h"
#include "slang/parsing/TokenKind.h"
#include "slang/syntax/AllSyntax.h"
#include "slang/syntax/SyntaxVisitor.h"

using namespace slang;
using namespace slang::ast;
using namespace slang::syntax;
using namespace slang::parsing;

namespace default_nettype_none {

struct MainVisitor : public TidyVisitor, ASTVisitor<MainVisitor, true, true, false, true> {
    explicit MainVisitor(Diagnostics& diagnostics) : TidyVisitor(diagnostics) {}

    void handle(const RootSymbol& root) {
        // Get all syntax trees and check them
        auto& compilation = root.getCompilation();
        for (const auto& tree : compilation.getSyntaxTrees()) {
            checkSyntaxTree(*tree);
        }
    }

private:
    void checkSyntaxTree(const SyntaxTree& tree) {
        // Get the source text of the file
        auto& sourceManager = tree.sourceManager();
        auto buffer = sourceManager.getSourceText(tree.root().sourceRange().start().buffer());

        // Check if the file contains `default_nettype none at the beginning
        // Look for the directive in the first few lines (allowing for comments)
        std::string_view text = buffer;
        bool hasDefaultNettypeNone = false;

        // Look for `default_nettype none in the file text
        size_t pos = text.find("`default_nettype");
        while (pos != std::string_view::npos) {
            // Look for "none" after the directive
            size_t nonePos = text.find("none", pos);
            if (nonePos != std::string_view::npos &&
                (nonePos - pos) < 20) { // within reasonable distance
                // Make sure there's no other text between directive and "none" (except whitespace)
                std::string_view between =
                    text.substr(pos + 16, nonePos - pos - 16); // 16 = length of "`default_nettype"
                bool onlyWhitespace = true;
                for (char c : between) {
                    if (c != ' ' && c != '\t') {
                        onlyWhitespace = false;
                        break;
                    }
                }
                if (onlyWhitespace) {
                    hasDefaultNettypeNone = true;
                    break;
                }
            }
            pos = text.find("`default_nettype", pos + 1);
        }

        if (!hasDefaultNettypeNone) {
            // Report error at the start of the file
            SourceLocation fileStart = tree.root().sourceRange().start();
            diags.add(diag::DefaultNettypeNone, fileStart);
        }
    }
};
} // namespace default_nettype_none

using namespace default_nettype_none;
class DefaultNettypeNone : public TidyCheck {
public:
    [[maybe_unused]] explicit DefaultNettypeNone(
        TidyKind kind, std::optional<slang::DiagnosticSeverity> severity) :
        TidyCheck(kind, severity) {}

    bool check(const ast::RootSymbol& root, const slang::analysis::AnalysisManager&) override {
        MainVisitor visitor(diagnostics);
        root.visit(visitor);
        return diagnostics.empty();
    }

    DiagCode diagCode() const override { return diag::DefaultNettypeNone; }
    DiagnosticSeverity diagDefaultSeverity() const override { return DiagnosticSeverity::Warning; }
    std::string diagString() const override {
        return "missing `default_nettype none directive at the top of file";
    }
    std::string name() const override { return "DefaultNettypeNone"; }
    std::string description() const override { return shortDescription(); }
    std::string shortDescription() const override {
        return "Enforces that all SystemVerilog files start with `default_nettype none to catch "
               "undeclared signal errors";
    }
};

REGISTER(DefaultNettypeNone, DefaultNettypeNone, TidyKind::Style)

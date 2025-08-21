//------------------------------------------------------------------------------
// ConstantWidthCheck.cpp
// Check that constants fit within their declared bit width
//
// SPDX-FileCopyrightText: Perrin Tong
// SPDX-License-Identifier: MIT
//------------------------------------------------------------------------------
#include "ASTHelperVisitors.h"
#include "TidyDiags.h"
#include <algorithm>
#include <cctype>
#include <regex>
#include <string>

#include "slang/ast/ASTVisitor.h"
#include "slang/syntax/AllSyntax.h"
#include "slang/syntax/SyntaxPrinter.h"

using namespace slang;
using namespace slang::ast;
using namespace slang::syntax;

namespace constant_width_check {

struct MainVisitor : public TidyVisitor, ASTVisitor<MainVisitor, true, true, false, true> {
    explicit MainVisitor(Diagnostics& diagnostics) : TidyVisitor(diagnostics) {}

    void handle(const IntegerLiteral& literal) {
        if (auto syntax = literal.syntax) {
            auto text = SyntaxPrinter().setIncludeTrivia(false).print(*syntax).str();

            std::regex sized_pattern("^([0-9]+)'([bBoOdDhH])(.+)$");
            std::smatch match;

            if (std::regex_match(text, match, sized_pattern)) {
                int declaredWidth = std::stoi(match[1].str());
                char base = std::tolower(match[2].str()[0]);
                std::string valueStr = match[3].str();

                if (declaredWidth >= 64 || declaredWidth <= 0) {
                    return;
                }

                uint64_t originalValue = parseValueByBase(valueStr, base);
                if (originalValue == UINT64_MAX) {
                    return; // Invalid parse
                }

                uint64_t maxVal = (1ULL << declaredWidth) - 1;

                if (originalValue > maxVal) {
                    diags.add(diag::ConstantWidthCheck, literal.sourceRange)
                        << std::string("constant value ") + std::to_string(originalValue) +
                               " in '" + text + "' overflows " + std::to_string(declaredWidth) +
                               "-bit width (max value: " + std::to_string(maxVal) + ")";
                }
            }
        }
    }

private:
    bool isValidForBase(const std::string& valueStr, char base) {
        if (valueStr.empty()) return false;
        
        for (char c : valueStr) {
            if (c == '_') continue; 
            
            switch (base) {
                case 'b':
                    if (c != '0' && c != '1') return false;
                    break;
                case 'o':
                    if (c < '0' || c > '7') return false;
                    break;
                case 'd':
                    if (c < '0' || c > '9') return false;
                    break;
                case 'h':
                    if (!std::isxdigit(c)) return false;
                    break;
                default:
                    return false;
            }
        }
        return true;
    }

    uint64_t parseValueByBase(const std::string& valueStr, char base) {
        if (!isValidForBase(valueStr, base)) {
            return UINT64_MAX;
        }
        
        std::string cleanValue = valueStr;
        cleanValue.erase(std::remove(cleanValue.begin(), cleanValue.end(), '_'), cleanValue.end());
        
        if (cleanValue.empty()) {
            return UINT64_MAX;
        }
        
        uint64_t result = 0;
        int baseValue;
        
        switch (base) {
            case 'b': baseValue = 2; break;
            case 'o': baseValue = 8; break;
            case 'd': baseValue = 10; break;
            case 'h': baseValue = 16; break;
            default: return UINT64_MAX;
        }
        
        for (char c : cleanValue) {
            uint64_t digit;
            if (c >= '0' && c <= '9') {
                digit = c - '0';
            } else if (c >= 'a' && c <= 'f') {
                digit = c - 'a' + 10;
            } else if (c >= 'A' && c <= 'F') {
                digit = c - 'A' + 10;
            } else {
                return UINT64_MAX;
            }
            
            if (digit >= baseValue) {
                return UINT64_MAX;
            }
            
            if (result > (UINT64_MAX - digit) / baseValue) {
                return UINT64_MAX;
            }
            
            result = result * baseValue + digit;
        }
        
        return result;
    }
};
} // namespace constant_width_check

using namespace constant_width_check;
class ConstantWidthCheck : public TidyCheck {
public:
    [[maybe_unused]] explicit ConstantWidthCheck(
        TidyKind kind, std::optional<slang::DiagnosticSeverity> severity) :
        TidyCheck(kind, severity) {}

    bool check(const ast::RootSymbol& root, const slang::analysis::AnalysisManager&) override {
        MainVisitor visitor(diagnostics);
        root.visit(visitor);
        return diagnostics.empty();
    }

    DiagCode diagCode() const override { return diag::ConstantWidthCheck; }
    DiagnosticSeverity diagDefaultSeverity() const override { return DiagnosticSeverity::Error; }
    std::string diagString() const override { return "constant range check: {}"; }
    std::string name() const override { return "ConstantWidthCheck"; }
    std::string description() const override { return shortDescription(); }
    std::string shortDescription() const override {
        return "Checks that constants fit within their declared bit width";
    }
};

REGISTER(ConstantWidthCheck, ConstantWidthCheck, TidyKind::Style)

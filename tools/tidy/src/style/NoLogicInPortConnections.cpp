//------------------------------------------------------------------------------
// NoLogicInPortConnections.cpp
// Check for logic expressions in named port instantiations
//
// SPDX-FileCopyrightText: Perrin Tong
// SPDX-License-Identifier: MIT
//------------------------------------------------------------------------------
#include "ASTHelperVisitors.h"
#include "TidyDiags.h"

#include "slang/syntax/AllSyntax.h"
#include "slang/syntax/SyntaxVisitor.h"

using namespace slang;
using namespace slang::ast;
using namespace slang::syntax;

namespace no_logic_in_port_connections {

struct PortConnectionVisitor : public SyntaxVisitor<PortConnectionVisitor> {
    void handle(const NamedPortConnectionSyntax& port) {
        // Only check connections with expressions (not .port or .port())
        if (port.openParen && port.expr) {
            if (auto expr = extractExpressionFromProperty(*port.expr)) {
                checkExpression(*expr, port.name.valueText());
            }
        }
    }

private:
    // Extract ExpressionSyntax from PropertyExprSyntax hierarchy
    const ExpressionSyntax* extractExpressionFromProperty(const PropertyExprSyntax& property) {
        if (property.kind == SyntaxKind::SimplePropertyExpr) {
            const auto& simpleProperty = property.as<SimplePropertyExprSyntax>();
            return extractExpressionFromSequence(*simpleProperty.expr);
        }
        // Add support for other property types if needed
        return nullptr;
    }

    // Extract ExpressionSyntax from SequenceExprSyntax hierarchy
    const ExpressionSyntax* extractExpressionFromSequence(const SequenceExprSyntax& sequence) {
        if (sequence.kind == SyntaxKind::SimpleSequenceExpr) {
            const auto& simpleSequence = sequence.as<SimpleSequenceExprSyntax>();
            return simpleSequence.expr;
        }
        // Add support for other sequence types if needed
        return nullptr;
    }

    // Check if a SyntaxKind represents an identifier-like expression
    bool isIdentifierLikeExpression(SyntaxKind kind) {
        return kind == SyntaxKind::IdentifierName || kind == SyntaxKind::IdentifierSelectName ||
               kind == SyntaxKind::EmptyIdentifierName;
    }

    // Check if a SyntaxKind is allowed in port connections
    bool isAllowedInPortConnection(SyntaxKind kind) {
        return isIdentifierLikeExpression(kind) || kind == SyntaxKind::IntegerLiteralExpression ||
               kind == SyntaxKind::IntegerVectorExpression ||
               kind == SyntaxKind::UnbasedUnsizedLiteralExpression ||
               kind == SyntaxKind::NullLiteralExpression ||
               kind == SyntaxKind::TimeLiteralExpression ||
               kind == SyntaxKind::WildcardLiteralExpression ||
               kind == SyntaxKind::ElementSelectExpression || kind == SyntaxKind::BitSelect ||
               kind == SyntaxKind::BitType; // BitType can appear in certain port connections
    }
    void checkExpression(const ExpressionSyntax& expr, std::string_view portName) {
        SyntaxKind kind = expr.kind;

        // Allow simple identifiers (both plain identifiers and identifier selects)
        if (isIdentifierLikeExpression(kind)) {
            return;
        }

        // Allow literals and vector literals
        if (kind == SyntaxKind::IntegerLiteralExpression ||
            kind == SyntaxKind::IntegerVectorExpression ||
            kind == SyntaxKind::RealLiteralExpression ||
            kind == SyntaxKind::StringLiteralExpression ||
            kind == SyntaxKind::UnbasedUnsizedLiteralExpression ||
            kind == SyntaxKind::NullLiteralExpression ||
            kind == SyntaxKind::TimeLiteralExpression ||
            kind == SyntaxKind::WildcardLiteralExpression) {
            return;
        }

        // Allow simple element select (e.g., array[0]) and identifier select (e.g., bus[0])
        // Also allow range selects (e.g., bus[3:0])
        if (kind == SyntaxKind::ElementSelectExpression ||
            kind == SyntaxKind::IdentifierSelectName || kind == SyntaxKind::BitSelect ||
            kind == SyntaxKind::BitType) { // BitType can appear in certain port connections
            return;
        }

        // Allow member access (e.g., struct.field)
        if (kind == SyntaxKind::MemberAccessExpression) {
            return;
        }

        // Allow simple concatenations of identifiers/literals
        if (kind == SyntaxKind::ConcatenationExpression) {
            const auto& concat = expr.as<ConcatenationExpressionSyntax>();
            bool allSimple = true;
            for (const auto& element : concat.expressions) {
                if (!isAllowedInPortConnection(element->kind)) {
                    allSimple = false;
                    break;
                }
            }
            if (allSimple) {
                return;
            }
        }

        // All other expressions are considered logic that should be moved outside
        foundPorts.push_back({&expr, std::string(portName)});
    }

public:
    struct LogicPort {
        const ExpressionSyntax* expr;
        std::string portName;
    };
    std::vector<LogicPort> foundPorts;
};

struct MainVisitor : public TidyVisitor, ASTVisitor<MainVisitor, true, true, false, true> {
    explicit MainVisitor(Diagnostics& diagnostics) : TidyVisitor(diagnostics) {}

    void handle(const InstanceBodySymbol& symbol) {
        NEEDS_SKIP_SYMBOL(symbol)
        if (!symbol.getSyntax()) {
            return;
        }

        PortConnectionVisitor visitor;
        symbol.getSyntax()->visit(visitor);

        for (const auto& port : visitor.foundPorts) {
            if (port.expr) {
                diags.add(diag::NoLogicInPortConnections, port.expr->sourceRange())
                    << "logic expression in port connection '" + port.portName +
                           "' (move logic outside the port instantiation for Quartus "
                           "compatibility)";
            }
        }
    }
};

} // namespace no_logic_in_port_connections

using namespace no_logic_in_port_connections;
class NoLogicInPortConnections : public TidyCheck {
public:
    [[maybe_unused]] explicit NoLogicInPortConnections(
        TidyKind kind, std::optional<slang::DiagnosticSeverity> severity) :
        TidyCheck(kind, severity) {}

    bool check(const ast::RootSymbol& root, const slang::analysis::AnalysisManager&) override {
        MainVisitor visitor(diagnostics);
        root.visit(visitor);
        return diagnostics.empty();
    }

    DiagCode diagCode() const override { return diag::NoLogicInPortConnections; }
    DiagnosticSeverity diagDefaultSeverity() const override { return DiagnosticSeverity::Warning; }
    std::string diagString() const override { return "{}"; }
    std::string name() const override { return "NoLogicInPortConnections"; }
    std::string description() const override { return shortDescription(); }
    std::string shortDescription() const override {
        return "Prohibits logic expressions in named port instantiations for Quartus compatibility";
    }
};

REGISTER(NoLogicInPortConnections, NoLogicInPortConnections, TidyKind::Style)

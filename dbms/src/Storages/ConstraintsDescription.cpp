#include <Storages/ConstraintsDescription.h>

#include <Parsers/formatAST.h>
#include <Parsers/ParserCreateQuery.h>
#include <Parsers/parseQuery.h>
#include <Parsers/ASTExpressionList.h>


namespace DB
{

String ConstraintsDescription::toString() const
{
    if (constraints.empty())
        return {};

    ASTExpressionList list;
    for (const auto & constraint : constraints)
        list.children.push_back(constraint);

    return serializeAST(list, true);
}

ConstraintsDescription ConstraintsDescription::parse(const String & str)
{
    if (str.empty())
        return {};

    ConstraintsDescription res;
    ParserConstraintDeclarationList parser;
    ASTPtr list = parseQuery(parser, str, 0);

    for (const auto & constraint : list->children)
        res.constraints.push_back(std::dynamic_pointer_cast<ASTConstraintDeclaration>(constraint));

    return res;
}

ConstraintsExpressions ConstraintsDescription::getExpressions(const DB::Context & context,
                                                              const DB::NamesAndTypesList & source_columns_) const
{
    ConstraintsExpressions res;
    res.reserve(constraints.size());
    for (const auto & constraint : constraints)
    {
        // SyntaxAnalyzer::analyze has query as non-const argument so to avoid accidental query changes we clone it
        ASTPtr expr = constraint->expr->clone();
        auto syntax_result = SyntaxAnalyzer(context).analyze(expr, source_columns_);
        res.push_back(ExpressionAnalyzer(constraint->expr->clone(), syntax_result, context).getActions(false));
    }
    return res;
}

}

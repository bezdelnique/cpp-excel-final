#include "FormulaAST.h"
#include "common.h"
#include "formula.h"
#include "test_runner_p.h"

using Value = FormulaInterface::Value;

namespace {

double ExecuteASTFormula(const std::string &expression) {
  return ParseFormulaAST(expression).Execute();
}

Value ExecuteFormula(const std::string &expression) {
  return ParseFormula(expression)->Evaluate();
}

}  // namespace


int main() {
  ASSERT_EQUAL(ExecuteASTFormula("1"), 1.0);
  ASSERT_EQUAL(ExecuteASTFormula("1+2*3-4/5"), 6.2);
  try {
    ExecuteASTFormula("1/0");
  } catch (const FormulaError &fe) {
    std::cout << fe.what() << std::endl;
  }


//  try {
//    auto empty_formula = ParseFormula("");
//  } catch (const FormulaException &exc) {
//    std::cout << exc.what() << std::endl;
//  }
//
//  try {
//    auto wrong_formula = ParseFormula("A1");
//  } catch (const FormulaException &exc) {
//    std::cout << exc.what() << std::endl;
//  }
//
//  try {
//    auto wrong_formula = ParseFormula("22/(22)+++++");
//  } catch (const FormulaException &exc) {
//    std::cout << exc.what() << std::endl;
//  }
//  ExecuteFormula("1+1");
  //ASSERT_EQUAL(std::get<double>(ExecuteFormula("1")), 1.0);
//  ASSERT_EQUAL(std::get<double>(ExecuteFormula("1+2*3-4/5")), 6.2);
//
//  std::cout << std::get<FormulaError>(ExecuteFormula("1/0")) << std::endl;
//
//  auto complex_formula = ParseFormula("((22/(22+3))-(8*5))");
//  ASSERT_EQUAL(complex_formula->GetExpression(), "22/(22+3)-8*5");
//
//  std::cout << "Tests Passed" << std::endl;
  return 0;
}

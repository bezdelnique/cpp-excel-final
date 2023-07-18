#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <memory>

using namespace std::literals;

std::ostream &operator<<(std::ostream &output, FormulaError fe) {
  return output << fe.ToString();
}

std::ostream &operator<<(std::ostream &output, Position pos) {
  return output << "Position{" << pos.row << "," << pos.col << "}";
}

namespace {
class Formula : public FormulaInterface {
 public:
  // Реализуйте следующие методы:
  explicit Formula(std::string expression)
      : ast_(ParseFormulaAST(expression)) {}

  Value Evaluate(const SheetInterface &sheet) const override {
    try {
      return ast_.Execute(sheet);
    } catch (const FormulaError &e) {
      return e;
    }
  }

  std::string GetExpression() const override {
    std::stringstream ss;
    ast_.PrintFormula(ss);
    return ss.str();
  }

  std::vector<Position> GetReferencedCells() const override {
    std::vector<Position> result;
    //std::vector<std::string> vec;
    //ast_.GetCells(vec);
    // Sorted
    for (auto const &pos : ast_.GetCells()) {
      result.push_back(pos);
    }
    //std::sort(result.begin(), result.end());
    return result;
  };

 private:
  FormulaAST ast_;
};

}  // namespace


std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
  return std::make_unique<Formula>(std::move(expression));
}

#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

// Реализуйте следующие методы
Cell::Cell() {}

Cell::~Cell() {}

void Cell::Set(std::string text) {
  if (text.size() > 1 && text[0] == FORMULA_SIGN) {
    // Попытка разобрать формулу
    try {
      ParseFormula(text.substr(1));
    } catch (...) {
      throw FormulaException("Unable to parse formula");
    }
    cell_type_ = CellType::FORMULA;
  } else {
    cell_type_ = CellType::STRING;
  }

  value_ = text;
}

void Cell::Clear() {}

Cell::Value Cell::GetValue() const {
  if (value_.empty()) {
    return value_;
  }

  if (cell_type_ == FORMULA) {
    auto result = ParseFormula(value_.substr(1))->Evaluate();
    if (std::holds_alternative<FormulaError>(result)) {
      return std::get<FormulaError>(result);
    } else {
      return std::get<double>(result);
    }
  } else {
    if (value_[0] == ESCAPE_SIGN) {
      return value_.substr(1);
    } else {
      return value_;
    }
  }

}

std::string Cell::GetText() const {
  if (cell_type_ == FORMULA) {
    // Очищенная формула
    std::stringstream ss;
    ss << '=' << ParseFormula(value_.substr(1))->GetExpression();
    return ss.str();
  } else {
    return value_;
  }
}


std::ostream &operator<<(std::ostream &output, const CellInterface::Value &value) {
  std::visit(
      [&](const auto &x) {
        output << x;
      },
      value);
  return output;
}

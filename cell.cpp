#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

// Реализуйте следующие методы
Cell::Cell() {}

Cell::~Cell() {}

void Cell::Set(std::string text) {
  cell_type_ = CellType::STRING;
  if (text.empty()) {
    return;
  }

  if (text[0] == FORMULA_SIGN && text.size() > 1) {
    cell_type_ = CellType::FORMULA;
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
      return std::get<FormulaError>(result).what();
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
    return ParseFormula(value_.substr(1))->GetExpression();
  } else {
    return value_;
  }

}

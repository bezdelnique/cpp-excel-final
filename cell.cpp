#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <sstream>

Cell::Cell(SheetInterface &sheet) : sheet_(sheet), value_holder_(std::make_unique<EmptyImpl>()) {};

Cell::~Cell() {}

void Cell::Set(std::string text) {
  if (text.size() > 1 && text[0] == FORMULA_SIGN) {
    //cell_type_ = CellType::FORMULA;
    value_holder_ = std::make_unique<FormulaImpl>(sheet_, text);
  } else {
    //cell_type_ = CellType::STRING;
    value_holder_ = std::make_unique<TextImpl>(text);
  }
  //value_ = text;
}

void Cell::Clear() {

}

Cell::Value Cell::GetValue() const {
  return value_holder_->GetValue();
}

std::string Cell::GetText() const {
  return value_holder_->GetText();
}

std::ostream &operator<<(std::ostream &output, const CellInterface::Value &value) {
  std::visit(
      [&](const auto &x) {
        output << x;
      },
      value);
  return output;
}


#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <sstream>

Cell::Cell(SheetInterface &sheet) : sheet_(sheet), value_holder_(std::make_unique<CellValueEmpty>()) {};

Cell::~Cell() {}

void Cell::Set(std::string text) {
  if (text.size() > 1 && text[0] == FORMULA_SIGN) {
    //cell_type_ = CellType::FORMULA;
    //std::forward_list<Position> forward_list_copy(forward_list_);
    //forward_list_copy.push_front(this.)

    value_holder_ = std::make_unique<CellValueFormula>(sheet_, text);
  } else {
    //cell_type_ = CellType::STRING;
    value_holder_ = std::make_unique<CellValueText>(text);
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

std::vector<Position> Cell::GetReferencedCells() const {
//  std::vector vec(forward_list_.begin(), forward_list_.end());
//  std::sort(vec.begin(), vec.end());
//  return vec;
  if (!IsFormula()) {
    return {};
  }
  return value_holder_->GetReferencedCells();
}

bool Cell::IsFormula() const {
  return value_holder_->GetType() == FORMULA;
}

//std::vector<Position> Cell::GetReferencedCells() const {
//  std::vector vec(forward_list_.begin(), forward_list_.end());
//  return vec;
//}

std::ostream &operator<<(std::ostream &output, const CellInterface::Value &value) {
  std::visit(
      [&](const auto &x) {
        output << x;
      },
      value);
  return output;
}

size_t PositionHasher::operator()(const Position position) const {
  std::hash<int> hasher;
  size_t value = 31;
  if (position.col) {
    value += hasher(position.col) * 31;
  }
  if (position.row) {
    value += hasher(position.row) * 31 * 31;
  }
  return value;
}

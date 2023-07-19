#include "cell.h"

#include <iostream>
#include <string>
#include <optional>

Cell::Cell(SheetInterface &sheet) :
    sheet_(sheet),
    value_holder_(std::make_unique<CellValueEmpty>()) {}

Cell::~Cell() {}

void Cell::Set(std::string text) {
  if (text.size() > 1 && text[0] == FORMULA_SIGN) {
    value_holder_ = std::make_unique<CellValueFormula>(sheet_, text);
  } else {
    value_holder_ = std::make_unique<CellValueText>(text);
  }
}

//void Cell::Clear() {
//}

Cell::Value Cell::GetValue() const {
  return value_holder_->GetValue();
}

std::string Cell::GetText() const {
  return value_holder_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
  if (!IsFormula()) {
    return {};
  }
  return value_holder_->GetReferencedCells();
}

bool Cell::IsFormula() const {
  return value_holder_->GetType() == FORMULA;
}

bool Cell::IsValid() const {
  for (auto const &pos: Cell::GetReferencedCells()) {
    if (pos == Position::NONE) {
      return false;
    }
  }
  return true;
}

void Cell::InvalidateCache() const {
  value_holder_->InvalidateCache();
}

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

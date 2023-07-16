#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <sstream>

Cell::Cell(SheetInterface &sheet) :
    sheet_(sheet),
    value_holder_(std::make_unique<CellValueEmpty>()) {};

Cell::~Cell() {}

void Cell::Set(std::string text) {
  if (text.size() > 1 && text[0] == FORMULA_SIGN) {
    //cell_type_ = CellType::FORMULA;
    //std::forward_list<Position> forward_list_copy(referenced_cells_);
    //forward_list_copy.push_front(this.)
    value_holder_ = std::make_unique<CellValueFormula>(sheet_, text);
  } else {
    //cell_type_ = CellType::STRING;
    value_holder_ = std::make_unique<CellValueText>(text);
  }
  //value_ = text;
}

//void Cell::Clear() {
//  value_holder_ = std::make_unique<CellValueEmpty>();
//  //InvalidateCache();
//}

Cell::Value Cell::GetValue() const {
  return value_holder_->GetValue();
}

std::string Cell::GetText() const {
  return value_holder_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
//  std::vector vec(referenced_cells_.begin(), referenced_cells_.end());
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

void Cell::InvalidateCache() const {
//  for (auto const pos : backward_list_) {
//    // Предполагается что циклических ссылок нет и дополнительные проверки не нужны
//    Cell *cell = reinterpret_cast<Cell *>(sheet_.GetCell(pos));
//    cell->InvalidateCache();
//  }
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

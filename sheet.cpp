#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
  validatePosition(pos);

  {
    if (pos.row + 1 > static_cast<int>(table_.size())) {
      table_.resize(pos.row + 1);
    }

    if (pos.col + 1 > static_cast<int>(table_[pos.row].size())) {
      table_[pos.row].resize(pos.col + 1);
    }

    if (table_[pos.row][pos.col] == nullptr) {
      table_[pos.row][pos.col] = std::make_unique<Cell>(*this);
    }

    table_[pos.row][pos.col]->Set(text);
  }

  afterSet(pos);
}

const CellInterface *Sheet::GetCell(Position pos) const {
  validatePosition(pos);

  if (pos.row < static_cast<int>(table_.size())) {
    if (pos.col < static_cast<int>(table_[pos.row].size())) {
      if (table_[pos.row][pos.col] != nullptr) {
        return table_[pos.row][pos.col].get();
      }
    }
  }
  return nullptr;
}

CellInterface *Sheet::GetCell(Position pos) {
  validatePosition(pos);

  if (pos.row < static_cast<int>(table_.size())) {
    if (pos.col < static_cast<int>(table_[pos.row].size())) {
      if (table_[pos.row][pos.col] != nullptr) {
        return table_[pos.row][pos.col].get();
      }
    }
  }
  return nullptr;
}

void Sheet::ClearCell(Position pos) {
  validatePosition(pos);

  bool found = false;
  CellInterface *cell = GetCell(pos);
  if (cell != nullptr) {
    table_[pos.row][pos.col] = nullptr;
    found = true;
  }

  if (found) {
    afterClear(pos);
  }
}

Size Sheet::GetPrintableSize() const {
  if (table_.empty()) {
    return {0, 0};
  }
  Size size = {0, 0};
  if (!rows.empty()) {
    size.rows = rows.rbegin()->first + 1;
  }
  if (!cols.empty()) {
    size.cols = cols.rbegin()->first + 1;
  }
  return {size.rows, size.cols};
}

void Sheet::PrintValues(std::ostream &output) const {
  auto size = GetPrintableSize();
  for (int row = 0; row < size.rows; ++row) {
    for (int col = 0; col < size.cols; ++col) {
      if (col > 0) {
        output << "\t";
      }
      const auto value = GetCell({row, col});
      if (value != nullptr) {
        output << value->GetValue();
      }
    }

    output << "\n";
  }

}

void Sheet::PrintTexts(std::ostream &output) const {
  auto size = GetPrintableSize();
  for (int row = 0; row < size.rows; ++row) {
    for (int col = 0; col < size.cols; ++col) {
      if (col > 0) {
        output << "\t";
      }
      const auto value = GetCell({row, col});
      if (value != nullptr) {
        output << value->GetText();
      }
    }

    output << "\n";
  }

}

void Sheet::afterClear(Position pos) {
  {
    auto it = rows.find(pos.row);
    if (it != rows.end()) {
      if (it->second > 1) {
        --it->second;
      } else {
        rows.erase(it);
      }
    }
  }

  {
    auto it = cols.find(pos.col);
    if (it != cols.end()) {
      if (it->second > 1) {
        --it->second;
      } else {
        cols.erase(it);
      }
    }
  }

}

void Sheet::afterSet(Position pos) {
  {
    auto it = rows.find(pos.row);
    if (it == rows.end()) {
      rows.emplace(pos.row, 1);
    } else {
      ++it->second;
    }
  }

  {
    auto it = cols.find(pos.col);
    if (it == cols.end()) {
      cols.emplace(pos.col, 1);
    } else {
      ++it->second;
    }
  }

}

void Sheet::validatePosition(Position pos) const {
  if (pos.row < 0 || pos.col < 0) {
    std::stringstream ss;
    ss << pos;
    throw InvalidPositionException(ss.str());
  }

  if (pos.row > Position::MAX_ROWS_ZB || pos.col > Position::MAX_COLS_ZB) {
    std::stringstream ss;
    ss << pos;
    throw InvalidPositionException(ss.str());
  }
}

std::unique_ptr<SheetInterface> CreateSheet() {
  return std::make_unique<Sheet>();
}

//size_t PositionHasher::operator()(const Position position) const {
//  std::hash<int> hasher;
//  size_t value = 31;
//  if (position.col) {
//    value += hasher(position.col) * 31;
//  }
//  if (position.row) {
//    value += hasher(position.row) * 31 * 31;
//  }
//  return value;
//}
//
